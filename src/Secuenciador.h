/*****************************************/
/*                                       */
/*  Secuenciador de fases de entradas    */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
#ifndef _SECUENCIADOR_
#define _SECUENCIADOR_

#define MAX_PLANES 2
#define HORAS_EN_DIA 24

#ifndef NO_CONFIGURADO 
#define NO_CONFIGURADO -1
#endif

#ifndef CONFIGURADO 
#define CONFIGURADO     1
#endif
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
/***************************** Includes *****************************/

//define un array de HORAS_EN_DIA enteros de 16 bits. Para los 12 primeros, es el valor on/off de los 5 minutos de esa hora
typedef struct{
  int8_t configurado;              //indica si el plan esta disponible y bien configurado o no
  int8_t rele;                     //salida a la que se asocia la secuencia
  int    horas[HORAS_EN_DIA];      //el valor es un campo de bit. los primeros 12 son los intervalos de 5 min de cada hora
  }plan; 

class secuenciadorClass
{
private:
plan planes[MAX_PLANES];
boolean secuenciadorActivo=false; //plag para activar o desactivar el secuenciador

public:
void inicializaSecuenciador();
boolean recuperaDatosSecuenciador(boolean debug);
boolean parseaConfiguracionSecuenciador(String contenido);
void actualizaSecuenciador(bool debug);
int8_t getNumPlanes();
int planConfigurado(uint8_t id);
void activarSecuenciador(void);
void desactivarSecuenciador(void);
boolean estadoSecuenciador(void);
String pintaPlanHTML(int8_t plan);
};

extern secuenciadorClass Secuenciador;

#endif