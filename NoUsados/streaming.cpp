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
#include <streaming.h>
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