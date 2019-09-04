/*****************************************/
/*                                       */
/*  Control de entradas digitales        */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
#ifndef _ENTRADAS_
#define _ENTRADAS_
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
/***************************** Includes *****************************/

class entrada
  {
  private:
  int8_t configurada;
  String nombre;
  int8_t estado;
  String tipo;        //Puede ser INPUT, INPUT_PULLUP, No valido!!-->INPUT_PULLDOWN
  int8_t pin;  
        
  public:
  void inicializaEntrada(void);
  void inicializaEntrada(int8_t conf, String nom, int8_t est, String tip, int8_t p);

  int8_t getConfigurada(void) {return configurada;};
  void setConfigurada(int8_t config) {configurada=config;};

  String getNombre(void) {return nombre;};
  void setNombre(String name) {nombre=name;};

  int8_t getEstado(void) {return estado;};
  void setEstado(int8_t est) {estado=est;};

  String getTipo(void) {return tipo;};
  void setTipo(String tip) {tipo=tip;};

  int8_t getPin(void) {return pin;};
  void setPin(int8_t p) {pin=p;};
  };

class EntradasClass
  {
  private:
  entrada entradas[MAX_ENTRADAS];  

  public:    
  //funciones de configuracion
  void inicializaEntradas(void);
  int recuperaDatosEntradas(boolean debug);
  boolean parseaConfiguracionEntradas(String contenido);
  int entradasConfiguradas(void);

  //funciones por entrada. Se entra en el array por id
  int8_t estadoEntrada(int8_t id);
  String nombreEntrada(int8_t id);
  int8_t entradaConfigurada(int8_t id);
  String tipoEntrada(int8_t id);
  int8_t pinEntrada(int8_t id);

  //funciones globales para todas las entradas
  void consultaEntradas(bool debug);
  String generaJsonEstadoEntradas(void);
  };

  extern EntradasClass Entradas;

#endif  


