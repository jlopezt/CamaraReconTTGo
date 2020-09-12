/***********************************************/
/*                                             */
/*  cacharro es el agregador del secuenciador  */
/*                                             */
/***********************************************/

/***************************** Defines *****************************/
#ifndef _CACHARRO_
#define _CACHARRO_

#define MAX_VUELTAS      UINT16_MAX // 65535 
/***************************** Defines *****************************/

/***************************** Includes *****************************/

/***************************** Includes *****************************/

class cacharroClass
  {
  private:    
    int nivelActivo; //Indica si el rele se activa con HIGH o LOW. Se activa con HIGH por defecto
    String nombre_dispositivo;//(NOMBRE_FAMILIA);//Nombre del dispositivo, por defecto el de la familia
    uint16_t vuelta = MAX_VUELTAS-100;//0; //vueltas de loop

  public:
    cacharroClass();

    boolean inicializaConfiguracion(boolean debug);

    int getNivelActivo(void) {return nivelActivo;};
    void setNivelActivo(int nivel) {nivelActivo=nivel;};

    String getNombreDispositivo(void) {return nombre_dispositivo;};
    void setNombreDispositivo(String nombre) {nombre_dispositivo=nombre;};

    boolean parseaConfiguracionGlobal(String contenido);
    String generaJsonConfiguracionNivelActivo(String configActual, int nivelAct);

    void configuraLed(void);
    void enciendeLed(void);
    void apagaLed(void);
    void parpadeaLed(uint8_t veces, uint16_t delayed=100);
    void parpadeaLed(uint8_t veces);

    uint16_t getVuelta(void) {return vuelta;}
    uint16_t incrementaVuelta(uint16_t incremento=1) {vuelta+=incremento;return vuelta;}
    };

extern cacharroClass cacharro;

#endif