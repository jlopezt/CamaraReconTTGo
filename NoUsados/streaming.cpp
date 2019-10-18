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

static esp_err_t stream_handler(httpd_req_t *req)
  {
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];
  dl_matrix3du_t *image_matrix = NULL;

  static int64_t last_frame = 0;
  if(!last_frame) last_frame = esp_timer_get_time();

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if(res != ESP_OK) return res;

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

  while(true)
    {
    fb = esp_camera_fb_get();
    if (!fb) 
      {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
      } 
    else 
      {
      if(fb->width > 400)
        {
        if(fb->format != PIXFORMAT_JPEG)
          {
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if(!jpeg_converted)
            {
            Serial.println("JPEG compression failed");
            res = ESP_FAIL;
            }
          }
        else 
          {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
          }
        }
      else 
        {
        image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);

        if (!image_matrix) 
          {
          Serial.println("dl_matrix3du_alloc failed");
          res = ESP_FAIL;
          }
        else 
          {
          if(!fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item))
            {
            Serial.println("fmt2rgb888 failed");
            res = ESP_FAIL;
            } 
          else 
            {
            if (fb->format != PIXFORMAT_JPEG)
              {
              if(!fmt2jpg(image_matrix->item, fb->width*fb->height*3, fb->width, fb->height, PIXFORMAT_RGB888, 90, &_jpg_buf, &_jpg_buf_len))
                {
                Serial.println("fmt2jpg failed");
                res = ESP_FAIL;
                }
              esp_camera_fb_return(fb);
              fb = NULL;
              }
            else 
              {
              _jpg_buf = fb->buf;
              _jpg_buf_len = fb->len;
              }
            }
            dl_matrix3du_free(image_matrix);
          }
        }
      }
        
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
        
    if(fb)
      {
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
      }
    else if(_jpg_buf)
      {
      free(_jpg_buf);
      _jpg_buf = NULL;
      }
        
    if(res != ESP_OK) break;
      
    int64_t fr_end = esp_timer_get_time();     
    int64_t frame_time = fr_end - last_frame;
    last_frame = fr_end;
    frame_time /= 1000;
    Serial.printf("MJPG: %uB %ums (%.1ffps)\n",(uint32_t)(_jpg_buf_len),(uint32_t)frame_time, 1000.0 / (uint32_t)frame_time);
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