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
    boolean leeFicheroConfig(String nombre, String &contenido);
    boolean salvaFicheroConfig(String nombreFichero, String nombreFicheroBak, String contenidoFichero);
    boolean leeFichero(String nombre, String &contenido);
    boolean salvaFichero(String nombreFichero, String nombreFicheroBak, String contenidoFichero);
    boolean borraFichero(String nombreFichero);
    boolean listaFicheros(String &contenido);
    boolean existeFichero(String nombre);
    boolean formatearFS(void);

    boolean setCandado(void);
    boolean getCandado(void);
  };

extern SistemaFicherosClass SistemaFicheros;

#endif