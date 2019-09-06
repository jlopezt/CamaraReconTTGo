/***********************************************/
/*                                             */
/*  cacharro es el agregador del secuenciador  */
/*                                             */
/***********************************************/

/***************************** Defines *****************************/
#ifndef _CACHARRO_
#define _CACHARRO_
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
/***************************** Includes *****************************/

class cacharroClass
  {
  private:    
    int nivelActivo; //Indica si el rele se activa con HIGH o LOW. Se activa con HIGH por defecto
    String nombre_dispositivo;//(NOMBRE_FAMILIA);//Nombre del dispositivo, por defecto el de la familia

  public:
    cacharroClass();
    boolean inicializaConfiguracion(boolean debug);

    void setNivelActivo(int nivel);
    int getNivelActivo(void);

    void setNombreDispositivo(String nombre);
    String getNombreDispositivo(void);

    boolean parseaConfiguracionGlobal(String contenido);
    String generaJsonConfiguracionNivelActivo(String configActual, int nivelAct);
    };

extern cacharroClass cacharro;

#endif