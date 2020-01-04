/**********************************************/
/*                                            */
/*      Sistema de reconocimiento facial      */
/*                                            */
/*    Basado en ESP-WHO                       */
/*                                            */
/**********************************************/

/***************************** Defines *****************************/
//Face reocn
#define ENROLL_CONFIRM_TIMES 5
#define FACE_ID_SAVE_NUMBER 7

//Streamming
#define PART_BOUNDARY "123456789000000000000987654321"
//Configuracion del puerto para streaming
#define PUERTO_STREAMING  81

//WebSockets
#define PUERTO_WEBSOCKET  88
#define NOT_CONNECTED     -1
#define INDEX_HTML "<!doctype html><html><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"><title>ESP32 Captura de caras con nombre<\title><\head><body>VACIO<\body><\html>"
#define INDEX_HTML_FILE                 "/web/index.html"
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
#include "esp_camera.h"
#include "faceRecon.h"
#include "fd_forward.h"
#include "fr_forward.h"
#include "fr_flash.h"
#include "camara.h"

//Adicionales por Streamming
#include "esp_http_server.h"
#include "esp_timer.h"
#include "img_converters.h"

#include "fb_gfx.h"

//Adicionales por WebSockets
//#include <ArduinoWebsockets.h>
#include <WebSocketsServer.h>
/***************************** Includes *****************************/

/********************** Face recon **********************/
extern camera_fb_t *fb;

static void send_face_list(void);
boolean enviarWSTXT(String mensaje);

typedef struct
  {
  uint8_t *image;
  box_array_t *net_boxes;
  dl_matrix3d_t *face_id;
  } http_img_process_result;

camera_fb_t *fb = NULL; //Global para todos los modulos que usen la camara. Aqui se llena, el resto solo lee
//websockets
WebSocketsServer webSocket = WebSocketsServer(PUERTO_WEBSOCKET);
typedef struct
  {
  uint8_t id=NOT_CONNECTED;//No hay cliuente conectado
  IPAddress IP={0,0,0,0};
  }cliente_t;
cliente_t cliente;

/**********************************************************/
/*                                                        */
/*    configuracion el sistema de reconocimiento facial   */
/*                                                        */
/**********************************************************/
static inline mtmn_config_t app_mtmn_config()
{
  mtmn_config_t mtmn_config = {0};
  mtmn_config.type = FAST;
  mtmn_config.min_face = 80;
  mtmn_config.pyramid = 0.707;
  mtmn_config.pyramid_times = 4;
  mtmn_config.p_threshold.score = 0.6;
  mtmn_config.p_threshold.nms = 0.7;
  mtmn_config.p_threshold.candidate_number = 20;
  mtmn_config.r_threshold.score = 0.7;
  mtmn_config.r_threshold.nms = 0.7;
  mtmn_config.r_threshold.candidate_number = 10;
  mtmn_config.o_threshold.score = 0.7;
  mtmn_config.o_threshold.nms = 0.7;
  mtmn_config.o_threshold.candidate_number = 1;
  return mtmn_config;
}
mtmn_config_t mtmn_config = app_mtmn_config();

/*
typedef struct
{
    face_id_node *head;    //!< head pointer of the id list 
    face_id_node *tail;    //!< tail pointer of the id list 
    uint8_t count;         //!< number of enrolled ids 
    uint8_t confirm_times; //!< images needed for one enrolling 
} face_id_name_list;
*/
face_id_name_list st_face_list;
static dl_matrix3du_t *aligned_face = NULL;

httpd_handle_t stream_httpd = NULL;

//*******Recupero la pagina inicial***********   
extern const unsigned char index_html_zip_start[] asm("_binary_src_www_index_html_zip_start");
extern const unsigned char index_html_zip_end[]   asm("_binary_src_www_index_html_zip_end");
size_t index_html_zip_len;

camera_fb_t *xfb;

typedef enum
{
  DISCONNECT,
  START_STREAM,
  START_DETECT,
  SHOW_FACES,
  START_RECOGNITION,
  START_ENROLL,
  ENROLL_COMPLETE,
  DELETE_ALL,
} en_fsm_state;
en_fsm_state g_state=DISCONNECT;

typedef struct
{
  char enroll_name[ENROLL_NAME_LEN];
} httpd_resp_value;

httpd_resp_value st_name;
/********************************************************************************************************/

/***************************************Inicio reconocimiento*****************************************************************/
/**********************************************************/
/*                                                        */
/*     inicializa el sistema de reconocimiento facial     */
/*                                                        */
/**********************************************************/
void faceRecon_init(boolean debug)
  {
  if(debug) Serial.printf("*************Init reconocimiento facial*****************\n");    
  g_state=DISCONNECT;

  //carga la lista de caras
  face_id_name_init(&st_face_list, FACE_ID_SAVE_NUMBER, ENROLL_CONFIRM_TIMES);
  aligned_face = dl_matrix3du_alloc(1, FACE_WIDTH, FACE_HEIGHT, 3);
  read_face_id_from_flash_with_name(&st_face_list);

  if(debug) Serial.printf("Leidas %i caras\n", st_face_list.count);  
  face_id_node *head = st_face_list.head;
  for (int i = 0; i < st_face_list.count; i++) // loop current faces
    {
    Serial.printf("cara[%i] nombre: %s\n",i,head->id_name);
    head = head->next;
    }
  }

/******************************************************/
/*                                                    */
/* Envia el mensaje de cara reconocida mediante MQTT  */ 
/* boolean enviarMQTT(String topic, String payload);  */
/******************************************************/
int caraReconocida(String nombre)
  {
  String topic;
  String payload;

  topic="CaraReconocida";
  payload=String("{\"nombre\": \"")+ nombre + String("\"}");
    
  Serial.printf("Se envia:\ntopic: %s | payload: %s\n",topic.c_str(),payload.c_str());
  if (miMQTT.enviarMQTT(topic,payload)) return OK;
  
  return KO;
  } 
  
/**********************************************************/
/*                                                        */
/* funcion de reconiocimiento facial sobre la imagen      */
/* actual de la camara comparando con la lista de caras   */
/* conocidas                                              */
/*                                                        */
/**********************************************************/
void reconocimientoFacial(boolean debug) 
  {    
  if(debug) Serial.printf("*************Reconocimiento facial*****************\nEmpezamos...\n");  
  dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, 320, 240, 3);
  http_img_process_result out_res = {0};
  out_res.image = image_matrix->item;

  if(debug) Serial.printf("reconocimientoFacial() ejecutado en el core %i\n",xPortGetCoreID());

  if(debug) Serial.printf("Leo la imagen de la camara\n");
  if(fb!=NULL) 
    {
    esp_camera_fb_return(fb);
    fb=NULL;
    }

  fb = esp_camera_fb_get();

  out_res.net_boxes = NULL;
  out_res.face_id = NULL;

  if(debug) Serial.printf("Convierte la imagen\n");
  fmt2rgb888(fb->buf, fb->len, fb->format, out_res.image);

  if(debug) Serial.printf("Evalua si hay una cara\n");
  out_res.net_boxes = face_detect(image_matrix, &mtmn_config);

  if (out_res.net_boxes)
    {
    if (align_face(out_res.net_boxes, image_matrix, aligned_face) == ESP_OK)
      {
      //Se ha detectado una cara
      if(debug) Serial.printf("Hay una cara\n");
      out_res.face_id = get_face_id(aligned_face);////////DA CORE
/***************************************************************/
      if (g_state == START_ENROLL)
        {
        int left_sample_face = enroll_face_id_to_flash_with_name(&st_face_list, out_res.face_id, st_name.enroll_name);
        char enrolling_message[64];
        sprintf(enrolling_message, "SAMPLE NUMBER %d FOR %s", ENROLL_CONFIRM_TIMES - left_sample_face, st_name.enroll_name);
        enviarWSTXT(enrolling_message);

        if (left_sample_face == 0)
          {
          ESP_LOGI(TAG, "Enrolled Face ID: %s", st_face_list.tail->id_name);
          g_state = START_STREAM;
          char captured_message[64];
          sprintf(captured_message, "FACE CAPTURED FOR %s", st_face_list.tail->id_name);
          enviarWSTXT(captured_message);
          send_face_list();
          }
        }
/***************************************************************/
      else
        {     
        if (st_face_list.count > 0)
          {
          face_id_node *f = recognize_face_with_name(&st_face_list, out_res.face_id);
          if (f)
            {
            //cara reconocida  
            if(debug || true) Serial.printf("Reconocido %s\n", f->id_name);
            enviarWSTXT("RECOGNISING");
            caraReconocida(f->id_name);
            }
          else
            {
            //cara no reconocida 
            if(debug) Serial.printf("Cara no reconocida\n"); 
            }
          }
        }
/***************************************************************/
      dl_matrix3d_free(out_res.face_id);
      }
    }
  else
    {
    //No se ha detectado cara  
    }
  
  //Si esta en modo streamming
  if (g_state == START_STREAM && cliente.id!=NOT_CONNECTED) 
    {
    //Serial.println("Enviando imagen cliente: %i tamaño:%i",cliente.id,fb->len);
    if(cliente.id!=NOT_CONNECTED) webSocket.sendBIN(cliente.id, (const uint8_t *)fb->buf, fb->len);
    }

  if(debug) Serial.printf("Liberamos y salimos\n");
  dl_matrix3du_free(image_matrix); //void dl_matrix3du_free(dl_matrix3du_t *m); en dl_lib_matrix3d.h
  }
 
/*********************************** Fin reconocimiento****************************************************************/

/*********************************** Inicio WebSocket *****************************************************************/
/**********************************************/
/*                                            */
/*Funciones auxiliares y gestion de Websockets*/
/*                                            */
/**********************************************/

/**********************************************/
/*                                            */
/*  Envia la lista de caras por websockets    */
/*                                            */
/**********************************************/
static void send_face_list(void)
  {
  enviarWSTXT("delete_faces"); // tell browser to delete all faces
  Serial.println("Enviado delete_faces");
  face_id_node *head = st_face_list.head;
  char add_face[64];
  for (int i = 0; i < st_face_list.count; i++) // loop current faces
    {
    Serial.printf("Enviado listface:%s\n",head->id_name);
    sprintf(add_face, "listface:%s", head->id_name);
    enviarWSTXT(add_face); //send face to browser
    head = head->next;
    }
  }

/**********************************************/
/*                                            */
/*       Borra la lista de caras              */
/*                                            */
/**********************************************/
void delete_all_faces(void)
  {
  delete_face_all_in_flash_with_name(&st_face_list);
  enviarWSTXT("delete_faces");
  Serial.println("Enviado delete_faces");
  }

/**********************************************/
/*                                            */
/*   Gestiona los mensajes recibidos por WS   */
/*                                            */
/**********************************************/
void gestionaMensajes(uint8_t cliente, String mensaje) //Tiene que implementar la maquina equivalente a la del loop de ESP-WHO. En funcion de g_state
  {
  Serial.printf("Procesando mensaje %s de %i\n",mensaje.c_str(),cliente);
	if(mensaje== "stream") 
		{
    g_state = START_STREAM;
    webSocket.sendTXT(cliente, "STREAMING");
    Serial.println("Enviado STREAMING");
 	  }

	if(mensaje== "detect") 
		{
    g_state = START_DETECT;
    webSocket.sendTXT(cliente, "DETECTING");
    Serial.println("Enviado DETECTING");
 	  }

	if(mensaje.substring(0, 8) == "capture:") 
		{
    g_state = START_ENROLL;
    char person[FACE_ID_SAVE_NUMBER * ENROLL_NAME_LEN] = {0,};
    mensaje.substring(8).toCharArray(person, sizeof(person));
    memcpy(st_name.enroll_name, person, strlen(person) + 1);    
    webSocket.sendTXT(cliente, "CAPTURING");
    Serial.println("Enviado CAPTURING");
 	  }

	if(mensaje== "recognise") 
		{
    g_state = START_RECOGNITION;
    webSocket.sendTXT(cliente, "RECOGNISING");
    Serial.println("Enviado RECOGNISING");
 	  }


	if(mensaje.substring(0, 7) == "remove:") 
		{
    char person[ENROLL_NAME_LEN * FACE_ID_SAVE_NUMBER];
    mensaje.substring(7).toCharArray(person, sizeof(person));
    delete_face_id_in_flash_with_name(&st_face_list, person);
    Serial.println("Enviado nueva lista de caras");
    send_face_list(); // reset faces in the browser
 	  }

  if (mensaje == "delete_all") 
  	{
    Serial.println("Borrando caras");
    delete_all_faces();
	  }
  }

/**********************************************/
/*                                            */
/*   Trasnforma un mensaje hexadecimal en     */
/*   texto imprimible en pantalla             */
/*                                            */
/**********************************************/
void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) 
  {
	const uint8_t* src = (const uint8_t*) mem;
	Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
	for(uint32_t i = 0; i < len; i++) 
    {
		if(i % cols == 0) 
      {
			Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
		  }
		Serial.printf("%02X ", *src);
		src++;
	  }
	Serial.printf("\n");
  }

void webSocketEvent(uint8_t clienteId, WStype_t type, uint8_t * payload, size_t length) 
  {
  switch(type) 
    {
    case WStype_DISCONNECTED:
        Serial.printf("[%u] Desconectado\n", clienteId);
        cliente.id=NOT_CONNECTED;//Se ha Desconectado
        cliente.IP={0,0,0,0};
        break;
    case WStype_CONNECTED:
        {
        IPAddress ip = webSocket.remoteIP(clienteId);
        Serial.printf("[%u] Connectado desde IP %d.%d.%d.%d url: %s\n", clienteId, ip[0], ip[1], ip[2], ip[3], payload);
        //Nuevo cliente conectado
        cliente.id=clienteId;
        cliente.IP=ip;

        //Lo paso al estado inicial
        send_face_list();
        //gestionaMensajes(clienteId,"stream");
        }
        break;
    case WStype_TEXT:
        {
        String datos="";
        for(uint8_t i=0;i<length;i++) datos+=(char)payload[i];

        Serial.printf("[%u] envia el texto: %s | datos: %s\n", clienteId, payload,datos.c_str());
        gestionaMensajes(clienteId,datos);
        }
        break;
    case WStype_BIN:
        Serial.printf("[%u] enviadatos bynarios length: %u\n", clienteId, length);
        hexdump(payload, length);

        // send message to client
        // webSocket.sendBIN(num, payload, length);
        break;
		case WStype_ERROR:			
		case WStype_FRAGMENT_TEXT_START:
		case WStype_FRAGMENT_BIN_START:
		case WStype_FRAGMENT:
		case WStype_FRAGMENT_FIN:
    case WStype_PING:
    case WStype_PONG:
			break;
    }
  }

void WebSocket_init(boolean debug)
  {
  //Inicializo la estructura de cliente
  cliente.id=NOT_CONNECTED;
  cliente.IP={0,0,0,0};

  //*******Configuracion del WS server***********
  if (debug) Serial.printf("Iniciamos el servidor de websockets\n");
  webSocket.begin();
  if (debug) Serial.printf("Asignado gestor de mensajes\n");
  webSocket.onEvent(webSocketEvent);
  if (debug) Serial.printf("Finalizado\n");  
  }

boolean enviarWSTXT(String mensaje)
  {
  boolean salida=false;  
  if(cliente.id!=NOT_CONNECTED) salida=webSocket.sendTXT(cliente.id, (const uint8_t *)mensaje.c_str());
  if (salida) Serial.println(mensaje);
  else Serial.printf("Error en el envio de %s\n",mensaje.c_str());
  return salida;
  }

void atiendeWebsocket(void)
  {
  webSocket.loop();  
  }
/*********************************** Fin WebSocket ********************************************************************/   