/*****************************************/
/*                                       */
/*        Control de salidas             */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
#ifndef _SALIDAS_
#define _SALIDAS_
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
/***************************** Includes *****************************/

class salida
  {
  private:
  int8_t configurado;     //0 si el rele no esta configurado, 1 si lo esta
  String nombre;          //nombre configurado para el rele
  int8_t estado;          //1 activo, 0 no activo (respecto a nivelActivo)
  int8_t pin;             // Pin al que esta conectado el rele
  int8_t pinLed;          // Pin al que esta conectado el led
  int8_t secuenciador;    //1 si esta asociado a un secuenciador que controla la salida, 0 si no esta asociado
  unsigned long finPulso; //fin en millis del pulso para la activacion de ancho definido
  int8_t inicio;          // modo inicial del rele "on"-->1/"off"-->0

  public:
  void inicializaSalida(void);
  void inicializaSalida(int8_t config, String nom, int8_t est, int8_t p, int8_t pLed, int8_t sec, unsigned long finP,int8_t ini);

  int8_t getConfigurado(void) {return configurado;};
  void setConfigurado(int8_t config) {configurado=config;};

  String getNombre(void) {return nombre;};
  void setNombre(String name) {nombre=name;};

  int8_t getEstado(void) {return estado;};
  void setEstado(int8_t est) {estado=est;};

  int8_t getPin(void) {return pin;};
  void setPin(int8_t p) {pin=p;};

  int8_t getPinLed(void) {return pinLed;};
  void setPinLed(int8_t p) {pinLed=p;};

  int8_t getSecuenciador(void) {return secuenciador;};
  void setSecuenciador(int8_t sec) {secuenciador=sec;};

  unsigned long getFinPulso(void) {return finPulso;};
  void setFinPulso(unsigned long finP) {finPulso=finP;};

  int8_t getInicio(void) {return inicio;};
  void setInicio(int8_t ini) {inicio=ini;};

  };  

class SalidasClass
  {
  private:
  salida reles[MAX_RELES];

  public:
  //funciones de configuracion
  void inicializaSalidas();
  boolean recuperaDatosSalidas(boolean debug);
  boolean parseaConfiguracionSalidas(String contenido);
  int relesConfigurados(void);

  //funciones por salida
  //consulta datos
  int8_t getInicioRele(int8_t id);
  int8_t getEstadoRele(int8_t id);
  String getNombreRele(int8_t id);
  int8_t getPinRele(int8_t id);
  int8_t getPinReleLed(int8_t id);
  int8_t getReleConfigurado(uint8_t id);
  int8_t getAsociadaASecuenciador(int8_t id); 
  //actua sobre el rele
  boolean conmutaRele(int8_t id, boolean estado_final, int debug);
  boolean pulsoRele(int8_t id);
  boolean actuaRele(int8_t id, int8_t estado);
  boolean asociarSecuenciador(int8_t id, int8_t plan);

  //funcviones globales para todas las salidas
  void actualizaSalidas(bool debug);
  String generaJsonEstadoSalidas(void); 
  };

  extern SalidasClass Salidas;

#endif


