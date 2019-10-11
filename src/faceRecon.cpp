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
  if(debug) Serial.printf("*************Init reconocimiento facial*****************\nEmpezamos...\n");    
  
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

  if(debug) Serial.printf("Leo la imagen de la camara\n");
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
          if(debug) Serial.printf("Reconocido %s\n", f->id_name);
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
  esp_camera_fb_return(fb);
  fb = NULL; 
  }
