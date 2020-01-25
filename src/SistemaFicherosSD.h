/************************************************/
/*                                              */
/* Funciones para la gestion de ficheros en     */
/* tarjeta SD del modulo esp32-CAM              */
/*                                              */
/************************************************/

/***************************** Defines *****************************/
#ifndef _SISTEMA_FICHEROS_SD_
#define _SISTEMA_FICHEROS_SD_

#define MONTAJE_SD "/SD"
/***************************** Defines *****************************/

/***************************** Includes *****************************/

/***************************** Includes *****************************/

class SistemaFicherosSDClass
  {
  public:
    SistemaFicherosSDClass(){};
    boolean inicializaFicheros(int debug);

    boolean SDDisponible(void);

    boolean leeFicheroBin(String nombre, uint8_t *contenido,uint8_t inicio, uint16_t len); //devuelve len bytes desde inicio del fichero indicado donde apunte el puntero indicado. El puntero debe tener memoria asignada suficiente
    boolean salvaFicheroBin(String nombreFichero, String nombreFicheroBak, uint8_t *contenidoFichero, uint16_t len);
    boolean leeFichero(String nombre, String &contenido);
    boolean salvaFichero(String nombreFichero, String nombreFicheroBak, String contenidoFichero);

    boolean borraFichero(String nombreFichero);
    uint16_t tamanoFichero(String nombreFichero);
    boolean existeFichero(String nombre);

    boolean listaFicheros(String &contenido);
  };

extern SistemaFicherosSDClass SistemaFicherosSD;

#endif