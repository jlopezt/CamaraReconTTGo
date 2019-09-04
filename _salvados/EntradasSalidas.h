/*****************************************/
/*                                       */
/*  Control de entradas y salidas        */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/

/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
#include <ArduinoJson.h>
#include <SistemaFicheros.h>
#include <cacharro.h>
/***************************** Includes *****************************/

//definicion de los tipos de dataos para las entradas y salidas
//Salidas
typedef struct{
  int8_t configurado;     //0 si el rele no esta configurado, 1 si lo esta
  String nombre;          //nombre configurado para el rele
  int8_t estado;          //1 activo, 0 no activo (respecto a nivelActivo)
  int8_t pin;             // Pin al que esta conectado el rele
  int8_t secuenciador;    //1 si esta asociado a un secuenciador que controla la salida, 0 si no esta asociado
  unsigned long finPulso; //fin en millis del pulso para la activacion de ancho definido
  int8_t inicio;          // modo inicial del rele "on"-->1/"off"-->0
  }rele_t; 


//Entradas
typedef struct{
  int8_t configurada;
  String nombre;
  int8_t estado;
  String tipo;        //Puede ser INPUT, INPUT_PULLUP, No valido!!-->INPUT_PULLDOWN
  int8_t pin;
  }entrada_t; 

class EntradasSalidasClass
  {
  private:
  entrada_t entradas[MAX_ENTRADAS];
  rele_t reles[MAX_RELES];

  public:
  void inicializaEntradasSalidas();
  int recuperaDatosEntradasSalidas(boolean debug);
  boolean parseaConfiguracionEntradasSalidas(String contenido);
  void actualizaSalidas(bool debug);
  int8_t estadoRele(int8_t id);
  String nombreRele(int8_t id);
  int8_t inicioRele(int8_t id);
  int8_t pinRele(int8_t id);
  int8_t conmutaRele(int8_t id, boolean estado_final, int debug);
  int8_t pulsoRele(int8_t id);
  int8_t actuaRele(int8_t id, int8_t estado);
  int8_t actuaRele(int8_t id, int8_t estado);
  int releConfigurado(uint8_t id);
  int relesConfigurados(void);
  void asociarSecuenciador(int8_t id, int8_t plan);
  int8_t asociadaASecuenciador(int8_t id);
  void consultaEntradas(bool debug);
  int8_t estadoEntrada(int8_t id);
  String nombreEntrada(int8_t id);
  String tipoEntrada(int8_t id);
  int8_t pinEntrada(int8_t id);
  int8_t entradaConfigurada(int8_t id);
  int entradasConfiguradas(void);
  String generaJsonEstadoSalidas(void);
  String generaJsonEstadoEntradas(void);
  String generaJsonEstado(void);
  };

extern EntradasSalidasClass EntradasSalidas;
