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
//#include <Global.h>
/***************************** Includes *****************************/
//prototipo de funciones
void streaming_init(boolean debug);

void WebSocket_init(boolean debug);
void atiendeWebsocket(void);

void faceRecon_init(boolean debug);
void reconocimientoFacial(boolean debug);
void delete_all_faces(void);
void activaRecon(boolean activar);
boolean getRecon(void);
void flipRecon(void);

#endif