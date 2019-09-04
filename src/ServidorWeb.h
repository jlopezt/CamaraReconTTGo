/*****************************************************************/
/*                                                               */
/* Serevicios de gestion del servidor Web incorporado al ESP32   */
/*                                                               */
/*****************************************************************/

/***************************** Defines *****************************/
#ifndef _SERVIDOR_WEB_
#define _SERVIDOR_WEB_

//Configuracion de los servicios web
#define PUERTO_WEBSERVER  80
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
/***************************** Includes *****************************/

void webServer(int debug);
void inicializaWebServer(void);

#endif