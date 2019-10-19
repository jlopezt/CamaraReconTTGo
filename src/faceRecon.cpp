/**********************************************/
/*                                            */
/*      Sistema de reconocimiento facial      */
/*                                            */
/*    Basado en ESP-WHO                       */
/*                                            */
/**********************************************/

/***************************** Defines *****************************/
#define ENROLL_CONFIRM_TIMES 5
#define FACE_ID_SAVE_NUMBER 7
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
#include "esp_camera.h"
#include "faceRecon.h"
#include "fd_forward.h"
#include "fr_forward.h"
#include "fr_flash.h"
#include "camara.h"
/***************************** Includes *****************************/

typedef struct
{
  uint8_t *image;
  box_array_t *net_boxes;
  dl_matrix3d_t *face_id;
} http_img_process_result;

camera_fb_t *fb = NULL; //Global para todos los modulos que usen la camara. Aqui se llena, el resto solo lee

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

/**********************************************************/
/*                                                        */
/*     inicializa el sistema de reconocimiento facial     */
/*                                                        */
/**********************************************************/
void faceRecon_init(boolean debug)
  {
  if(debug) Serial.printf("*************Init reconocimiento facial*****************\n");    
  
  face_id_name_init(&st_face_list, FACE_ID_SAVE_NUMBER, ENROLL_CONFIRM_TIMES);
  aligned_face = dl_matrix3du_alloc(1, FACE_WIDTH, FACE_HEIGHT, 3);
  read_face_id_from_flash_with_name(&st_face_list);
  if(debug) Serial.printf("Leidas %i caras\n", st_face_list.count);    
  }

/**********************************************************/
/*                                                        */
/*     Incluye una nueva cara al la lista de conocidas    */
/*                                                        */
/**********************************************************/
static inline int do_enrollment(face_id_name_list *face_list, dl_matrix3d_t *new_id, char enroll_name[ENROLL_NAME_LEN])
  {
  ESP_LOGD(TAG, "START ENROLLING");
  int left_sample_face = enroll_face_id_to_flash_with_name(face_list, new_id, enroll_name);
  ESP_LOGD(TAG, "Face ID %s Enrollment: Sample %d",
           st_name.enroll_name,
           ENROLL_CONFIRM_TIMES - left_sample_face);
  return left_sample_face;
  }

/******************************************************/
/*                                                    */
/* Envia el mensake de cara reconocida mediante MQTT  */ 
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
//camera_fb_t *fb = NULL;
  dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, 320, 240, 3);
  http_img_process_result out_res = {0};
  out_res.image = image_matrix->item;

  Serial.print("Face recon executed on core ");
  Serial.println(xPortGetCoreID());

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

      if (st_face_list.count > 0)
        {
        face_id_node *f = recognize_face_with_name(&st_face_list, out_res.face_id);
        if (f)
          {
          //cara reconocida  
          if(debug || true) Serial.printf("Reconocido %s\n", f->id_name);
          caraReconocida(f->id_name);
          }
        else
          {
          //cara no reconocida 
          if(debug) Serial.printf("Cara no reconocida\n"); 
          }
        }
      dl_matrix3d_free(out_res.face_id);
      }
    }
  else
    {
    //No se ha detectado cara  
    }

  if(debug) Serial.printf("Liberamos y salimos\n");
  dl_matrix3du_free(image_matrix); //void dl_matrix3du_free(dl_matrix3du_t *m); en dl_lib_matrix3d.h
  //esp_camera_fb_return(fb);
  //fb = NULL; 
  } 



/********************************************************************************************************************************************/


/**********************************************/
/*                                            */
/*  Servicio de streaming sobre la camara     */
/*                                            */
/**********************************************/

/***************************** Defines *****************************/
#define PART_BOUNDARY "123456789000000000000987654321"
//Configuracion del puerto para streaming
#define PUERTO_STREAMING  81
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h> //Lo tienen todos los modulos
//#include <streaming.h>
#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_camera.h"
#include "img_converters.h"
#include "Arduino.h"

#include "fb_gfx.h"
#include "fd_forward.h"
/***************************** Includes *****************************/

//Variables del modulo
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t stream_httpd = NULL;

camera_fb_t *xfb;

static esp_err_t stream_handler(httpd_req_t *req)
  {
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];

  static int64_t last_frame = 0;
  if(!last_frame) last_frame = esp_timer_get_time();

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if(res != ESP_OK) return res;

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

  Serial.print("stream_handler executed on core ");
  Serial.println(xPortGetCoreID());

  while(true)
    {
    xfb = esp_camera_fb_get();
    _jpg_buf = xfb->buf;
    _jpg_buf_len = xfb->len;

    if(res == ESP_OK)
      {
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
      }
        
    if(res == ESP_OK)
      {
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
      }
        
    if(res == ESP_OK)
      {
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
      }

    esp_camera_fb_return(xfb);
    xfb=NULL;
    delay(100);
        
    if(res != ESP_OK) break;
      
    int64_t fr_end = esp_timer_get_time();     
    int64_t frame_time = fr_end - last_frame;
    last_frame = fr_end;
    frame_time /= 1000;
    if (debugGlobal) Serial.printf("MJPG: Tamaño imagen: %uB | Tiempo de procesamiento: %ums (%.1ffps)\n",(uint32_t)(_jpg_buf_len),(uint32_t)frame_time, 1000.0 / (uint32_t)frame_time);
    }

  last_frame = 0;
  return res;
  }

void streaming_init(boolean debug)
  {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  httpd_uri_t stream_uri = 
    {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
    };
    
    config.server_port = PUERTO_STREAMING;
    config.ctrl_port += 1;
    Serial.printf("Servicio de streaming iniciado en el puerto: '%d'\n", config.server_port);
    if (httpd_start(&stream_httpd, &config) == ESP_OK) 
      {
      httpd_register_uri_handler(stream_httpd, &stream_uri);
      }
   }  

/*********************************** Inicio WebSocket *****************************************************************/
/**********************************************/
/*                                            */
/*  Servicio de streaming sobre la camara     */
/*                                            */
/**********************************************/

/***************************** Defines *****************************/
#define PUERTO_WEBSOCKET  82
#define INDEX_HTML ""
/***************************** Defines *****************************/

/***************************** Includes *****************************/ 
#include <ArduinoWebsockets.h>
/***************************** Includes *****************************/ 
/*
String paginaWS="";

using namespace websockets;
WebsocketsServer socket_server;
WebsocketsClient &client=NULL;

typedef enum
{
  START_STREAM,
  START_DETECT,
  SHOW_FACES,
  START_RECOGNITION,
  START_ENROLL,
  ENROLL_COMPLETE,
  DELETE_ALL,
} en_fsm_state;
en_fsm_state g_state;

void app_facenet_main()
{
  face_id_name_init(&st_face_list, FACE_ID_SAVE_NUMBER, ENROLL_CONFIRM_TIMES);
  aligned_face = dl_matrix3du_alloc(1, FACE_WIDTH, FACE_HEIGHT, 3);
  read_face_id_from_flash_with_name(&st_face_list);
}

static inline int do_enrollment(face_id_name_list *face_list, dl_matrix3d_t *new_id)
{
  ESP_LOGD(TAG, "START ENROLLING");
  int left_sample_face = enroll_face_id_to_flash_with_name(face_list, new_id, st_name.enroll_name);
  ESP_LOGD(TAG, "Face ID %s Enrollment: Sample %d",
           st_name.enroll_name,
           ENROLL_CONFIRM_TIMES - left_sample_face);
  return left_sample_face;
}

static esp_err_t send_face_list(WebsocketsClient &client)
{
  client.send("delete_faces"); // tell browser to delete all faces
  face_id_node *head = st_face_list.head;
  char add_face[64];
  for (int i = 0; i < st_face_list.count; i++) // loop current faces
  {
    sprintf(add_face, "listface:%s", head->id_name);
    client.send(add_face); //send face to browser
    head = head->next;
  }
}

static esp_err_t delete_all_faces(WebsocketsClient &client)
{
  delete_face_all_in_flash_with_name(&st_face_list);
  client.send("delete_faces");
}

void handle_message(WebsocketsClient &client, WebsocketsMessage msg)
{
  if (msg.data() == "stream") {
    g_state = START_STREAM;
    client.send("STREAMING");
  }
  if (msg.data() == "detect") {
    g_state = START_DETECT;
    client.send("DETECTING");
  }
  if (msg.data().substring(0, 8) == "capture:") {
    g_state = START_ENROLL;
    char person[FACE_ID_SAVE_NUMBER * ENROLL_NAME_LEN] = {0,};
    msg.data().substring(8).toCharArray(person, sizeof(person));
    memcpy(st_name.enroll_name, person, strlen(person) + 1);
    client.send("CAPTURING");
  }
  if (msg.data() == "recognise") {
    g_state = START_RECOGNITION;
    client.send("RECOGNISING");
  }
  if (msg.data().substring(0, 7) == "remove:") {
    char person[ENROLL_NAME_LEN * FACE_ID_SAVE_NUMBER];
    msg.data().substring(7).toCharArray(person, sizeof(person));
    delete_face_id_in_flash_with_name(&st_face_list, person);
    send_face_list(client); // reset faces in the browser
  }
  if (msg.data() == "delete_all") {
    delete_all_faces(client);
  }
}

void inicializaWebServer(void)
  {
  //*******Recupero la pagina inicial***********   
  if(!SistemaFicheros.leeFicheroConfig(INDEX_HTML, paginaWS)) paginaWS=INDEX_HTML;

  //*******Configuracion del WS server***********
  socket_server.listen(PUERTO_WEBSOCKET);
  client = socket_server.accept();
  client.onMessage(handle_message);
  }
*********************************** Fin WebSocket ********************************************************************/   