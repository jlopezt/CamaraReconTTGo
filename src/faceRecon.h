/**********************************************/
/*                                            */
/*      Sistema de reconocimiento facial      */
/*                                            */
/**********************************************/

/***************************** Defines *****************************/
#ifndef _FACERECON_
#define _FACERECON_
/***************************** Defines *****************************/

/***************************** Includes *****************************/
/***************************** Includes *****************************/
extern camera_fb_t *fb;
//prototipo de funciones
void streaming_init(boolean debug);

void WebSocket_init(boolean debug);

void faceRecon_init(boolean debug);
void reconocimientoFacial(boolean debug);
#endif