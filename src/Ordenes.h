/*
 * ordenes.ino
 *
 * Interfaz serie para la recepcion de ordenes y su ejecución 
 *
 * Permite la puesta en hora del reloj a traves de comandos enviados por el puesto
 * serie desde el PC.
 *
 * Para actualizar la hora <comado> <valor>; Ejemplo: "hora 3;"
 * Se pueden anidar: "hora 2;minuto 33;"
 *
 */

/***************************** Defines *****************************/
#ifndef _ORDENES_
#define _ORDENES_

#define LONG_COMANDO 40
#define LONG_PARAMETRO 30
#define LONG_ORDEN 22 //Comando (espacio) Parametros (fin de cadena)
#define MAX_COMANDOS   35
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
/***************************** Includes *****************************/

typedef struct 
  {
  String comando;
  String descripcion;
  void (*p_func_comando) (int, char*, float)=NULL;
  }tipo_comando;

class OrdenesClass
    {
    public:
        OrdenesClass(); 
        void inicializaOrden(void);
        void gestionaOrdenes(int debug);
        int HayOrdenes(int debug);
        int EjecutaOrdenes(int debug);
        void limpiaOrden(void);   
        String getComando(int indice){return comandos[indice].comando;};
        String getDescripcion(int indice){return comandos[indice].descripcion;}; 

    private:
        char ordenRecibida[LONG_ORDEN]="";
        int lonOrden=0;
        tipo_comando comandos[MAX_COMANDOS];
    };

extern OrdenesClass Ordenes;

#endif