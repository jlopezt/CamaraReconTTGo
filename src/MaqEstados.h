/*******************************************/
/*                                         */
/*  Definicion de la maquina de estados    */
/*  configurable.                          */
/*                                         */
/*  Hay un mapeo de E/S del dispositivo y  */
/*  de la maquina de estados. La entrada 1 */
/*  de la maquina puede ser la 4 del dis-  */
/*  positivo. Igual con las salidas.       */
/*                                         */
/*  el estado 0 es el de error             */
/*  el estado 1 es el de inicio            */
/*  el resto configurables                 */
/*                                         */
/*******************************************/

/***************************** Defines *****************************/
#ifndef _MAQESTADOS_
#define _MAQESTADOS_

#define MAX_ESTADOS 10
#define MAX_TRANSICIONES 3*MAX_ESTADOS
#define ESTADO_ERROR   0 //Estado de error de la logica de la maquina. Imposible evolucionar del ese estado con las estradas actuales
#define ESTADO_INICIAL 1 //Estado inicio

/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
/***************************** Includes *****************************/
//Estados de la maquina. Tienen un nombre, un id y una lista de salidas asociadas, son el valor de las salidas en ese estado
typedef struct {
    uint8_t id;
    String nombre;
    int8_t valorSalidas[MAX_SALIDAS];
    }estados_t;

//Los lazos del grafo. Estado inicial, estado final y los valores de las entradas que gobiernan la transicion
typedef struct {
    int8_t estadoInicial;
    int8_t estadoFinal;
    int8_t valorEntradas[MAX_ENTRADAS];
    }transicionEstados_t;

class maqEstadosClass
    {
    private:
    uint8_t numeroEstados=0;
    uint8_t numeroTransiciones=0; //numero de lazos de la maquina de estados. A cada transicion se asocia un estado inicial, unos valores de las entradas y un estado final
    uint8_t numeroEntradas=0;
    uint8_t numeroSalidas=0;

    estados_t estados[MAX_ESTADOS];
    transicionEstados_t transiciones[MAX_TRANSICIONES];
    uint8_t mapeoEntradas[MAX_ENTRADAS]; //posicion es la entrada de la maquina de estados, el valor es el id de la entrada del dispositivo
    uint8_t mapeoSalidas[MAX_SALIDAS]; //posicion es la salida de la maquina de estados, el valor es el id de la salida del dispositivo

    uint8_t estadoActual;
    int8_t entradasActual[MAX_ENTRADAS]; //Valor leido de las entradas
    int8_t salidasActual[MAX_SALIDAS];
    boolean debugMaquinaEstados;

    public:
    void inicializaMaquinaEstados(void);
    boolean recuperaDatosMaquinaEstados(int debug);
    boolean parseaConfiguracionMaqEstados(String contenido);
    void actualizaMaquinaEstados(int debug=false);
    uint8_t mueveMaquina(uint8_t estado, int8_t entradasActual[], boolean debug=false);
    int8_t actualizaSalidasMaquinaEstados(uint8_t estado);

    /****************************************************/
    /* Funciones de consulta de dataos (encapsulan)     */
    /****************************************************/
    uint8_t getNumEstados(void){return numeroEstados;}
    uint8_t getNumTransiciones(void){return numeroTransiciones;}
    uint8_t getNumEntradasME(void){return numeroEntradas;}
    uint8_t getNumSalidasME(void){return numeroSalidas;}

    uint8_t getNumEntradaME(uint8_t entrada)
    {
    if(entrada>numeroEntradas) return -1;
    return mapeoEntradas[entrada];
    }
    
    uint8_t getNumSalidaME(uint8_t salida)
    {
    if(salida>numeroSalidas) return -1;
    return mapeoSalidas[salida];
    }

    int8_t getEstadoInicialTransicion(int8_t transicion) {return transiciones[transicion].estadoInicial;}
    int8_t getEstadoFinalTransicion(int8_t transicion) {return transiciones[transicion].estadoFinal;}
    int8_t getValorEntradaTransicion(int8_t transicion, int8_t entrada) {return transiciones[transicion].valorEntradas[entrada];}

    String getNombreEstado(uint8_t estado){return estados[estado].nombre;}
    String getNombreEstadoActual(void){return getNombreEstado(estadoActual);}

    };
#endif