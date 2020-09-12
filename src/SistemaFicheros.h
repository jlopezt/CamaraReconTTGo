/************************************************/
/*                                              */
/* Funciones para la gestion de ficheros en     */
/* memoria del modulo esp6288                   */
/*                                              */
/************************************************/

/***************************** Defines *****************************/
#ifndef _SISTEMA_FICHEROS_
#define _SISTEMA_FICHEROS_
/***************************** Defines *****************************/

/***************************** Includes *****************************/

/***************************** Includes *****************************/

class SistemaFicherosClass
  {
  private:
    boolean candado; //Candado de configuracion. true implica que la ultima configuracion fue mal

  public:
    SistemaFicherosClass() {};
    boolean inicializaFicheros(int debug);

    boolean leeFicheroBin(String nombre, uint8_t *contenido,uint8_t inicio, uint16_t len); //devuelve len bytes desde inicio del fichero indicado donde apunte el puntero indicado. El puntero debe tener memoria asignada suficiente
    boolean salvaFicheroBin(String nombreFichero, String nombreFicheroBak, uint8_t *contenidoFichero, uint16_t len);
    boolean leeFichero(String nombre, String &contenido);
    boolean salvaFichero(String nombreFichero, String nombreFicheroBak, String contenidoFichero);
    
    boolean borraFichero(String nombreFichero);
    boolean existeFichero(String nombre);
    uint16_t tamanoFichero(String nombreFichero);
    
    boolean listaFicheros(String &contenido);

    boolean formatearFS(void);
  };

extern SistemaFicherosClass SistemaFicheros;

#endif