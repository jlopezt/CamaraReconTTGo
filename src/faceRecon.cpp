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
#define INTERVALO_RECONOCIMIENTOS 10000

#define FACE_COLOR_WHITE  0x00FFFFFF
#define FACE_COLOR_BLACK  0x00000000
#define FACE_COLOR_RED    0x000000FF
#define FACE_COLOR_GREEN  0x0000FF00
#define FACE_COLOR_BLUE   0x00FF0000
#define FACE_COLOR_YELLOW (FACE_COLOR_RED | FACE_COLOR_GREEN)
#define FACE_COLOR_CYAN   (FACE_COLOR_BLUE | FACE_COLOR_GREEN)
#define FACE_COLOR_PURPLE (FACE_COLOR_BLUE | FACE_COLOR_RED)

//Streamming
#define PART_BOUNDARY "123456789000000000000987654321"
//Configuracion del puerto para streaming
#define PUERTO_STREAMING  81

//WebSockets
#define PUERTO_WEBSOCKET  88
#define NOT_CONNECTED     -1
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

#include <FS.h>
#include <SD_MMC.h>
/***************************** Includes *****************************/

/********************** Face recon **********************/
extern camera_fb_t *fb;

static void send_face_list(void);
int8_t configuraSensor(const char* variable, int8_t valor);

camera_fb_t *fb = NULL; //Global para todos los modulos que usen la camara. Aqui se llena, el resto solo lee

//websockets
boolean enviarWSTXT(String mensaje);
typedef struct
  {
  int8_t id=NOT_CONNECTED;//No hay cliuente conectado
  IPAddress IP={0,0,0,0};
  }cliente_t;
cliente_t cliente;
WebSocketsServer webSocket = WebSocketsServer(PUERTO_WEBSOCKET);

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

face_id_name_list st_face_list;
static dl_matrix3du_t *aligned_face = NULL;

httpd_handle_t stream_httpd = NULL;

typedef union
  {
  fptp_t *float_p;
  uint8_t *uint8_p;  
  }item_t;

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

unsigned long intervaloReconocimiento=0;
boolean reconocerCaras=true;
/********************************************************************************************************/
//Prototipos de funciones
boolean recuperaDatosCaras(boolean debug);
boolean parseaConfiguracionCaras(String contenido);
/***************************************Inicializa reconocimiento*****************************************************************/
/**********************************************************/
/*                                                        */
/*     inicializa el sistema de reconocimiento facial     */
/*                                                        */
/**********************************************************/
void faceRecon_init(boolean debug)
  {
  Serial.printf("*************Init reconocimiento facial*****************\n");    
  g_state=DISCONNECT;
  intervaloReconocimiento=INTERVALO_RECONOCIMIENTOS;

  if(!recuperaDatosCaras(debugGlobal)) 
    {
    Serial.println("Configuracion de caras leida de la flash");
    //Inicializa la lista de caras
    face_id_name_init(&st_face_list, FACE_ID_SAVE_NUMBER, ENROLL_CONFIRM_TIMES);
    //Carga desde la memoria flash
    read_face_id_from_flash_with_name(&st_face_list);
    }

  Serial.printf("Intervalo: %lu\nconfirm_times: %i\nLeidas %i caras\n",intervaloReconocimiento,st_face_list.confirm_times, st_face_list.count);  
  face_id_node *head = st_face_list.head;
  for (int i = 0; i < st_face_list.count; i++) // loop current faces
    {
    Serial.printf("cara[%i] nombre: %s\n",i,head->id_name);
    head = head->next;
    }

  aligned_face = dl_matrix3du_alloc(1, FACE_WIDTH, FACE_HEIGHT, 3);  
  if(aligned_face==NULL) Serial.printf("Ma' fallao!!!");
  }

boolean recuperaDatosCaras(boolean debug)
  {
  String cad="";

  if (debug) Serial.println("Recupero configuracion de archivo...");
  
  if(!SistemaFicheros.leeFichero(FACE_RECON_CONFIG_FILE, cad)) 
    {
    //Confgiguracion por defecto
    Serial.printf("No existe fichero de configuracion de Caras\n");
    return false; 
    }      
    
  return parseaConfiguracionCaras(cad);
  }

/*******************************************************/
/*  Lee la configuracion de caras a ficheros en la     */
/*  memoria flash del esp32-CAM.                       */
/*                                                     */
/*  Lee un fichero JSON con el numero de caras, el     */
/*  confirm_times (2 uint8_t) y una lista con  los     */
/*  nombres de las caras. Ademas, lee un fichero       */
/*  binario con las configuraiones de las caras, como  */
/*  nodos de la lista que menja en memoria durtante el */
/*  proceso de reconocimiento.                         */
/*******************************************************/
boolean parseaConfiguracionCaras(String contenido)
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());

  Serial.print("\nJSON a parsear: ");
  json.printTo(Serial);
  if (!json.success()) return false;
        
  Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
  uint8_t confirm_times=ENROLL_CONFIRM_TIMES;
  if (json.containsKey("confirm_times")) confirm_times=json["confirm_times"]; 
  if (json.containsKey("intervaloReconocimiento")) intervaloReconocimiento=json["intervaloReconocimiento"]; 
  
  //Configuracion de la camara
  int8_t valor=0;
  if (json.containsKey("hmirror"))
    {
    valor=json.get<int>("hmirror"); 
    configuraSensor("hmirror",valor);
    }
  else if (json.containsKey("vflip")) 
    {
    valor=json.get<int>("vflip"); 
    configuraSensor("vflip",valor);
    }  
  else if (json.containsKey("special_effect"))
    {
    valor=json.get<int>("special_effect"); 
    configuraSensor("special_effect",valor);
    }
  else if (json.containsKey("wb_mode"))
    {
    valor=json.get<int>("wb_mode"); 
    configuraSensor("wb_mode",valor);
    }
  else if (json.containsKey("framesize"))
    {
    valor=json.get<int>("framesize"); 
    configuraSensor("framesize",valor);
    }
  else if (json.containsKey("quality"))
    {
    valor=json.get<int>("quality"); 
    configuraSensor("quality",valor);
    }
  else if (json.containsKey("contrast"))
    {
    valor=json.get<int>("contrast"); 
    configuraSensor("contrast",valor);
    }
  else if (json.containsKey("brightness"))
    {
    valor=json.get<int>("brightness"); 
    configuraSensor("brightness",valor);
    }
  else if (json.containsKey("saturation"))
    {
    valor=json.get<int>("saturation"); 
    configuraSensor("saturation",valor);
    }

  JsonArray& caras = json["face_id_nodes"];

  //Inicializo la lista de caras con el contador y el numero de confirmaciones
  uint8_t count=caras.size();
  face_id_name_init(&st_face_list, count, confirm_times);

  for(uint8_t i=0;i<count;i++)
    {
    JsonObject& cara = caras[i];  

    face_id_node *new_node = (face_id_node *)malloc(sizeof(face_id_node));
    new_node->next = NULL;//doy valor a next
    strcpy(new_node->id_name, (const char *)cara["nombre"]);//doy valor a id_name
    //relleno id_vec que es un dl_matrix3d_t
    new_node->id_vec = (dl_matrix3d_t *)malloc(sizeof(dl_matrix3d_t));//aloco memoria para id_vec
    if(new_node->id_vec==NULL) return false;
    new_node->id_vec->w=cara["w"];//doy valor a w
    new_node->id_vec->h=cara["h"];//doy valor a h
    new_node->id_vec->c=cara["c"];//doy valor a c
    new_node->id_vec->n=cara["n"];//doy valor a n
    new_node->id_vec->stride=cara["stride"];//doy valor a stride
    new_node->id_vec->item=(fptp_t *)malloc(FACE_ID_SIZE * sizeof(float));
    SistemaFicheros.leeFicheroBin(String("/")+String(new_node->id_name)+String(".bin"),(uint8_t *)new_node->id_vec->item,0,(uint16_t)(FACE_ID_SIZE * sizeof(float)));//doy valor a item

    //Enlazo el nodo a la lista
    if (NULL == st_face_list.head)
      {
        st_face_list.head = new_node;
        st_face_list.tail = new_node;
      }
    else
      {
        st_face_list.tail->next = new_node;
        st_face_list.tail = new_node;
      }

    st_face_list.count++;        
    }

  Serial.printf("Caras:\nnum. caras: %i\nconfirm_times: %i\n",st_face_list.count,st_face_list.confirm_times); 
  face_id_node *cara=st_face_list.head;
  for(int8_t i=0;i<st_face_list.count;i++) 
    {
    Serial.printf("Cara %s\n",cara->id_name);
    cara=cara->next;
    }
//************************************************************************************************

  return true;    
}

/**********************************************/
/*                                            */
/*       Serializa la lista de caras          */
/*                                            
typedef float fptp_t; //apunta a un area de memoria de FACE_ID_SIZE floats
typedef struct
{
    int w;          //!< Width 
    int h;          //!< Height 
    int c;          //!< Channel 
    int n;          //!< Number of filter, input and output must be 1 
    int stride;     //!< Step between lines 
    fptp_t *item;   //!< Data 
} dl_matrix3d_t;

typedef struct tag_face_id_node
  {
  struct tag_face_id_node *next;
  char id_name[ENROLL_NAME_LEN];
  dl_matrix3d_t *id_vec;
  } face_id_node;

typedef struct
  {
  face_id_node *head;    //!< head pointer of the id list 
  face_id_node *tail;    //!< tail pointer of the id list 
  uint8_t count;         //!< number of enrolled ids 
  uint8_t confirm_times; //!< images needed for one enrolling 
  } face_id_name_list;

JSON para serializar la lista de caras
{
  "count": 3,
  "confirm_times": 5,
  "face_id_nodes": [
    {"id_name": "Jose",     "dl_matrix3d_t":{"w":1,"h":1,"c":1,"n":1,"stride":1,"item":1}},
    {"id_name": "Almudena", "dl_matrix3d_t":{"w":1,"h":1,"c":1,"n":1,"stride":1,"item":1}},
    {"id_name": "Jorge",    "dl_matrix3d_t":{"w":1,"h":1,"c":1,"n":1,"stride":1,"item":1}}
  ]
}  
                                              */
/**********************************************/
/*******************************************************/
/*  Salva la configuracion de caras a ficheros en la   */
/*  memoria flash del esp32-CAM.                       */
/*                                                     */
/*  Salva un fichero JSON con el numero de caras, el   */
/*  confirm_times (2 uint8_t) y una lista con  los     */
/*  nombres de las caras. Ademas, salva un fichero     */
/*  binario con las configuraiones de las caras, como  */
/*  nodos de la lista que menja en memoria durtante el */
/*  proceso de reconocimiento.                         */
/*******************************************************/
boolean salvar_lista_face_id_a_fichero(face_id_name_list *lista, String ficheroConfig,String ficheroConfigBak)
{
  //genero el jsonBuffer
  const size_t tamanoBuffer=JSON_ARRAY_SIZE(lista->count)+JSON_OBJECT_SIZE(3)+lista->count*JSON_OBJECT_SIZE(6);
  DynamicJsonBuffer jb(tamanoBuffer);

  //Creo el JsonObject 
  JsonObject& root = jb.createObject();
  //root.set("count",lista->count);
  root.set("intervaloReconocimiento",intervaloReconocimiento);
  root.set("confirm_times",lista->confirm_times);

  face_id_node *cara = lista->head;
  JsonArray& nodos = root.createNestedArray("face_id_nodes");
  for(uint8_t caras=0;caras<lista->count;caras++)
    {
    if(cara==NULL) return false;//hay menos nodos de los que dice count

    dl_matrix3d_t *matrix=cara->id_vec;
    JsonObject& nodo = nodos.createNestedObject();
    nodo["nombre"] = cara->id_name;
    nodo["w"] = matrix->w;
    nodo["h"] = matrix->h;
    nodo["c"] = matrix->c;
    nodo["n"] = matrix->n;
    nodo["stride"] = matrix->stride;

    String nombreFicheroBin=String("/")+String(cara->id_name)+String(".bin");
    SistemaFicheros.salvaFicheroBin(nombreFicheroBin,nombreFicheroBin+String(".bak"),(uint8_t *)matrix->item,FACE_ID_SIZE * (uint16_t)sizeof(float));

    cara = cara->next;//paso a la siguiente cara de la lista
    }
  
  root.prettyPrintTo(Serial);

  //Salvo el fichero de configuracion
  String cad;
  root.printTo(cad);
  if(!SistemaFicheros.salvaFichero(ficheroConfig,ficheroConfigBak,cad)) return false;

  return true;
}  

int8_t configuraSensor(const char* variable, int8_t valor)
  {
  //int valor = atoi(value);
  sensor_t * s = esp_camera_sensor_get();
  int8_t res = 0;

  if(!strcmp(variable, "framesize")) {if(s->pixformat == PIXFORMAT_JPEG) res = s->set_framesize(s, (framesize_t)valor);}
  /*
  value="10" -->UXGA(1600x1200) 
  value="9"  -->SXGA(1280x1024) 
  value="8"  -->XGA(1024x768) 
  value="7"  -->SVGA(800x600) 
  value="6"  -->VGA(640x480) 
  value="5"  -->CIF(400x296) 
  value="4"  -->QVGA(320x240) 
  value="3"  -->HQVGA(240x176) 
  value="0"  -->QQVGA(160x120)   
  */
  else if(!strcmp(variable, "quality")) res = s->set_quality(s, valor); //10 - 63
  else if(!strcmp(variable, "contrast")) res = s->set_contrast(s, valor); //-2 - +2
  else if(!strcmp(variable, "brightness")) res = s->set_brightness(s, valor); //-2 - +2
  else if(!strcmp(variable, "saturation")) res = s->set_saturation(s, valor); //-2 - +2
  else if(!strcmp(variable, "gainceiling")) res = s->set_gainceiling(s, (gainceiling_t)valor); //+2 - +128
  else if(!strcmp(variable, "colorbar")) res = s->set_colorbar(s, valor); //check-box
  else if(!strcmp(variable, "awb")) res = s->set_whitebal(s, valor); //check-box 
  else if(!strcmp(variable, "agc")) res = s->set_gain_ctrl(s, valor); //check-box
  else if(!strcmp(variable, "aec")) res = s->set_exposure_ctrl(s, valor); //check-box
  else if(!strcmp(variable, "hmirror")) res = s->set_hmirror(s, valor); //check-box
  else if(!strcmp(variable, "vflip")) res = s->set_vflip(s, valor); //check-box
  else if(!strcmp(variable, "awb_gain")) res = s->set_awb_gain(s, valor); //check-box
  else if(!strcmp(variable, "agc_gain")) res = s->set_agc_gain(s, valor); //0 - +30
  else if(!strcmp(variable, "aec_value")) res = s->set_aec_value(s, valor); //0 - +1200
  else if(!strcmp(variable, "aec2")) res = s->set_aec2(s, valor); //check-box
  else if(!strcmp(variable, "dcw")) res = s->set_dcw(s, valor); //check-box
  else if(!strcmp(variable, "bpc")) res = s->set_bpc(s, valor); //check-box
  else if(!strcmp(variable, "wpc")) res = s->set_wpc(s, valor); //check-box
  else if(!strcmp(variable, "raw_gma")) res = s->set_raw_gma(s, valor); //check-box
  else if(!strcmp(variable, "lenc")) res = s->set_lenc(s, valor); //check-box
  else if(!strcmp(variable, "special_effect")) res = s->set_special_effect(s, valor);
  /*
  value="0"-->No Effect
  value="1"-->Negative            
  value="2"-->Grayscale           
  value="3"-->Red Tint            
  value="4"-->Green Tint          
  value="5"-->Blue Tint           
  value="6"-->Sepia               
  */
  else if(!strcmp(variable, "wb_mode")) res = s->set_wb_mode(s, valor);
  /*
  value="0"-->Auto
  value="1"-->Sunny          
  value="2"-->Cloudy         
  value="3"-->Office         
  value="4"-->Home             
  */
  else if(!strcmp(variable, "ae_level")) res = s->set_ae_level(s, valor); //-2 - +2
  else {res = -1;}    

  return res;
  }
/**********************************************************Fin configuracion******************************************************************/  

/******************************************************/
/*                                                    */
/* Activa o desactiva el reconocimiento facial        */ 
/*                                                    */
/******************************************************/
void activaRecon(boolean activar)
  {
  Serial.printf("El reconocimiento facial esta %s\n",(activar?"on":"off"));  
  reconocerCaras=activar;
  }

/******************************************************/
/*                                                    */
/*    Devuelve el estado del reconocimiento facial    */ 
/*                                                    */
/******************************************************/
boolean getRecon(void)
  {
  return reconocerCaras;
  }

/******************************************************/
/*                                                    */
/*      Cambia el modo del reconocimeinto facial      */ 
/*                                                    */
/******************************************************/
void flipRecon(void)
  {
  Serial.printf("El reconocimiento facial estaba %s\n",(reconocerCaras?"on":"off"));  
  reconocerCaras=!reconocerCaras;
  Serial.printf("ahora esta %s\n",(reconocerCaras?"on":"off"));  
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
  unsigned long ahora=millis();
  static unsigned long ultimoReconocimiento=0;

  if(ahora-ultimoReconocimiento>intervaloReconocimiento) 
    {
    ultimoReconocimiento=ahora;

    //Activa el rele para abrir la puerta
    Salidas.pulsoRele(0); //Fuerzo a que el primer rele es el que abre la puerta!!!!!
  
    //Informa poir MQTT que se ha habierto la puerta y a quien y cuando
    topic="/camara/caraReconocida";
    payload=String("{\"nombre\": \"")+ nombre + String("\", \"Hora\": \"") + reloj.getHora() + String("\"}");
      
    if(debugGlobal) Serial.printf("Se envia:\ntopic: %s | payload: %s\n",topic.c_str(),payload.c_str());
    if (miMQTT.enviarMQTT(topic,payload))
      {
      topic="puerta/cerradura";
      payload="{\"id\":0,\"estado\":\"pulso\"}";
      if (miMQTT.enviarMQTT(topic,payload)) return OK;   
      }
    return KO;
    } 
  
  //Serial.printf("Dentro de intervalo de guarda\n Llevamos %0.2f s",((float)ahora-(float)ultimoReconocimiento)/1000);
  return OK;
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
  if(debug) Serial.printf("*************Reconocimiento facial*****************\n");  
  if(debug) Serial.printf("Ejecutadose en el core %i\n",xPortGetCoreID());

  fb = esp_camera_fb_get();
  if (fb!=NULL)
    {
    if(debug) Serial.printf("Imagen leida de la camara\n");
    dl_matrix3du_t *image_matrix =  dl_matrix3du_alloc(1, fb->width, fb->height, 3);//dl_matrix3du_alloc(1, 320, 240, 3); //NO GESTIONO QUE SEA NULL
    if(!image_matrix) 
      {
      Serial.printf("AAAArrrrrrgggggghhhh!!!!!!!!!!!!!!!!!!");
      return;
      }

    /**************************INICIO RECONOCER CARA*********************/
    if(!Entradas.estadoEntrada(0))//if(reconocerCaras && image_matrix)
      {
      pintaCadenaLimpia("Esperando...");
      }
    else
      {
      pintaCadenaLimpia("Buscando...");
      if(debug) Serial.printf("Convierte la imagen\n");

      fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item); //NO GESTIONO EL KO!!! 

      if(debug) Serial.printf("Evalua si hay una cara\n");
      box_array_t *net_boxes_x = face_detect(image_matrix, &mtmn_config);
      if (net_boxes_x)
        {
        if (align_face(net_boxes_x, image_matrix, aligned_face) == ESP_OK)
          {
          //Se ha detectado una cara
          if(debug) Serial.printf("Hay una cara\n");

          dl_matrix3d_t *face_id_x = get_face_id(aligned_face);

          if (g_state == START_ENROLL)
            {
            int left_sample_face = enroll_face_id_to_flash_with_name(&st_face_list, face_id_x, st_name.enroll_name);
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
          else
            {     
            if (st_face_list.count > 0)
              {
              face_id_node *f = recognize_face_with_name(&st_face_list, face_id_x);
              if (f)
                {
                //cara reconocida
                String cad="Reconocido: \n"+ String(f->id_name);
                pintaCadenaLimpia(cad);
                if(debug) Serial.printf("%s\n", cad.c_str());
                enviarWSTXT(cad);
                caraReconocida(f->id_name);
                }
              else
                {
                //cara no reconocida 
                if(debug) Serial.printf("Cara no reconocida\n"); 
                pintaCadenaLimpia("Cara no\nreconocida");
                enviarWSTXT("Cara no reconocida");
                }
              }
            }
          dl_matrix3d_free(face_id_x); //Evito memory leaks
          }
        else
          {
          pintaCadenaLimpia("No hay cara");
          }
        
          //Evito memory leaks
          free(net_boxes_x->score);
          free(net_boxes_x->box);
          free(net_boxes_x->landmark);
          free(net_boxes_x);
        }
      else
        {
        //No se ha detectado cara  
        enviarWSTXT("No se detecta cara");
        }
      }
    /**************************FIN RECONOCER CARA*********************/
    //Si esta en modo streamming
    if (g_state == START_STREAM && cliente.id!=NOT_CONNECTED) 
      {
      //Serial.println("Enviando imagen cliente: %i tamaño:%i",cliente.id,fb->len);
      if(cliente.id!=NOT_CONNECTED) webSocket.sendBIN(cliente.id, (const uint8_t *)fb->buf, fb->len);
      }

    if(debug) Serial.printf("Liberamos y salimos\n");
    dl_matrix3du_free(image_matrix);
    }

  if(fb!=NULL) 
    {
    esp_camera_fb_return(fb);
    fb=NULL;
    }     
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
  Serial.printf("Procesando mensaje %s, cliente: %i\n",mensaje.c_str(),cliente);
	if(mensaje== "stream") 
		{
    g_state = START_STREAM;
    webSocket.sendTXT(cliente, "STREAMING");
    Serial.println("Enviado STREAMING");
 	  }

	if(mensaje== "detect") //NO LO ENVIA NADIE!!!!
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

	if(mensaje== "recognise") //NO LO ENVIA NADIE
		{
    g_state = START_RECOGNITION;
    webSocket.sendTXT(cliente, "RECOGNISING");
    Serial.println("Enviado RECOGNISING");
 	  }


	if(mensaje.substring(0, 7) == "remove:") 
		{
    char person[ENROLL_NAME_LEN * FACE_ID_SAVE_NUMBER];
    mensaje.substring(7).toCharArray(person, sizeof(person));
    //delete_face_id_in_flash_with_name(&st_face_list, person);
    delete_face_with_name(&st_face_list, person);
    Serial.println("Enviado nueva lista de caras");
    send_face_list(); // reset faces in the browser
 	  }

  if (mensaje == "delete_all") 
  	{
    Serial.println("Borrando caras");
    delete_all_faces();
	  }

  if (mensaje == "serialize") 
    {
    String cad="";  
    if(salvar_lista_face_id_a_fichero(&st_face_list, FACE_RECON_CONFIG_FILE,FACE_RECON_CONFIG_BAK_FILE)) cad="config salvada";
    else cad="Error al salvar la configuracion";

    webSocket.sendTXT(cliente, cad);
    }

  if (mensaje == "foto")  
    {
    Serial.println("Enviando foto al cliente...");
    if(fb!=NULL)
      {
      webSocket.sendTXT(cliente, "foto");
      webSocket.sendBIN(cliente,fb->buf, fb->len);//uint8_t num, const uint8_t * payload, size_t length);
      }
    }

  if (mensaje == "reconoce")  
    {
    Serial.println("Activa reconocimiento");
    //reconocerCaras=true;
    activaRecon(true);
    }

  if (mensaje == "no_reconoce")  
    {
    Serial.println("Activa reconocimiento");
    //reconocerCaras=false;
    activaRecon(false);
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
        }
        break;
    case WStype_TEXT:
        {
        String datos="";
        for(uint8_t i=0;i<length;i++) datos+=(char)payload[i];

        Serial.printf("[%u] recibido el texto: %s | datos: %s\n", clienteId, payload,datos.c_str());
        gestionaMensajes(clienteId,datos);
        }
        break;
    case WStype_BIN:
        Serial.printf("[%u] recibidos datos binarios length: %u\n", clienteId, length);
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

  if (debugGlobal)
    {
    if (salida ) Serial.println(mensaje);
    else Serial.printf("Error en el envio de [%s] | id de cliente: %i | IP de cliente: %s\n",mensaje.c_str(),cliente.id,cliente.IP.toString().c_str());
    }

  return salida; 
  }

void atiendeWebsocket(void)
  {
  webSocket.loop();  
  }
/*********************************** Fin WebSocket ********************************************************************/   