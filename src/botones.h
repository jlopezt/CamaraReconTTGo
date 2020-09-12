/**********************************************/
/*                                            */
/*  Gestion de los botones de TTGO T-camera   */
/*                                            */
/**********************************************/
/***************************** Defines *****************************/
#ifndef _BOTONES_
#define _BOTONES_

//#define BUTTON_1            34
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <OneButton.h>
/***************************** Includes *****************************/

void botones_init(boolean debug);
void atiendeBotones(void);

extern OneButton button;

#endif
