/**********************************************/
/*                                            */
/*      Sistema de reconocimiento facial      */
/*                                            */
/*    Basado en ESP-WHO                       */
/*                                            */
/**********************************************/

/***************************** Defines *****************************/
#define BUTTON_1            34
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include "botones.h"
#include "Global.h"
#include "faceRecon.h"
/***************************** Includes *****************************/

/********************** botones **********************/

/***************************************Inicializa reconocimiento*****************************************************************/
/**********************************************************/
/*                                                        */
/*     inicializa los botones del sistema                 */
/*                                                        */
/**********************************************************/
void botones_init(boolean debug)
  {   
  button.attachClick(flipRecon); //volteaCamara);
  //button.attachDoubleClick();
  }

void atiendeBotones(void) 
  {
  button.tick();
  }

OneButton button(BUTTON_1, true);