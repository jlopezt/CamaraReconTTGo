/************************************************/
/*                                              */
/* Funciones para la gestion de ficheros en     */
/* tarjeta SD del modulo esp32-CAM              */
/*                                              */
/************************************************/

/***************************** Defines *****************************/
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
#include <SistemaFicherosSD.h>
#include <FS.h>     //this needs to be first, or it all crashes and burns...
#include "SD_MMC.h" // SD Card ESP32-CAM
/***************************** Includes *****************************/

/***************************** Funciones de soporte *******************************/
void listDir(fs::FS &fs, const char * dirname, uint8_t levels)
  {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if(!root)
    {
    Serial.println("Failed to open directory");
    return;
    }

  if(!root.isDirectory())
    {
    Serial.println("Not a directory");
    return;
    }

  File file = root.openNextFile();
  while(file)
    {
    if(file.isDirectory())
      {
      Serial.print("  DIR : ");
      Serial.println(file.name());
          if(levels)listDir(fs, file.name(), levels -1);
      } 
    else 
      {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
      }
      file = root.openNextFile();
    }
  }

void createDir(fs::FS &fs, const char * path)
  {
  Serial.printf("Creating Dir: %s\n", path);
  if(fs.mkdir(path)) Serial.println("Dir created");
  else Serial.println("mkdir failed");
  }

void removeDir(fs::FS &fs, const char * path)
  {
  Serial.printf("Removing Dir: %s\n", path);
  if(fs.rmdir(path)) Serial.println("Dir removed");
  else Serial.println("rmdir failed");
  }

void readFile(fs::FS &fs, const char * path)
  {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if(!file)
    {
    Serial.println("Failed to open file for reading");
    return;
    }

  Serial.print("Read from file: ");
  while(file.available()) Serial.write(file.read());
  }

void writeFile(fs::FS &fs, const char * path, const char * message)
  {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file)
    {
    Serial.println("Failed to open file for writing");
    return;
    }

  if(file.print(message)) Serial.println("File written");
  else Serial.println("Write failed");
  }

void appendFile(fs::FS &fs, const char * path, const char * message)
  {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file)
    {
    Serial.println("Failed to open file for appending");
    return;
    }
  
  if(file.print(message)) Serial.println("Message appended");
  else Serial.println("Append failed");
  }

void renameFile(fs::FS &fs, const char * path1, const char * path2)
  {
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) Serial.println("File renamed");
  else Serial.println("Rename failed");
  }

void deleteFile(fs::FS &fs, const char * path)
  {
  Serial.printf("Deleting file: %s\n", path);
  
  if(fs.remove(path)) Serial.println("File deleted");
  else Serial.println("Delete failed");
  }

void writeBinFile2(fs::FS &fs, const char * path, const uint8_t * buff, uint8_t size)
  {
  Serial.printf("Escribiendo fichero: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file)
    {
    Serial.println("Error al abrir el frichero para escritura");
    return;
    }

  if(file.write(buff, size)) Serial.println("Fichero escrito");
  else Serial.println("Error en la escritura en el fichero");
  }

void testFileIO(fs::FS &fs, const char * path)
  {
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if(file)
    {
    len = file.size();
    size_t flen = len;
    start = millis();
    while(len)
      {
      size_t toRead = len;
      if(toRead > 512) toRead = 512;
      file.read(buf, toRead);
      len -= toRead;
      }

    end = millis() - start;
    Serial.printf("%u bytes read for %u ms\n", flen, end);
    file.close();
    }
  else Serial.println("Failed to open file for reading");

  file = fs.open(path, FILE_WRITE);
  if(!file)
    {
    Serial.println("Failed to open file for writing");
    return;
    }

  size_t i;
  start = millis();
  for(i=0; i<2048; i++) file.write(buf, 512);

  end = millis() - start;
  Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
  file.close();
  }
/**************************** Fin funciones de soporte **************************/

/************************************************/
/* Inicializa el sistema de ficheros del modulo */
/************************************************/
boolean SistemaFicherosSDClass::inicializaFicheros(int debug)
  {
  //inicializo el sistema de ficheros de la tarjeta SD
  if (!SD_MMC.begin(MONTAJE_SD))
    {
    Serial.println("Tarjeta SD no disponible");
    return (false);
    }

  uint8_t cardType = SD_MMC.cardType();

  if(cardType == CARD_NONE)
    {
    Serial.println("Tarjeta SD_MMC no detectada");
    return false;
    }

  Serial.print("Tipo de tarjeta SD_MMC: ");
  if(cardType == CARD_MMC) Serial.println("MMC");
  else if(cardType == CARD_SD) Serial.println("SDSC");
  else if(cardType == CARD_SDHC) Serial.println("SDHC");
  else Serial.println("DESCONOCIDO");

  uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
  Serial.printf("Tamaño de la tarjeta SD_MMC: %lluMB\n", cardSize);

  listDir(SD_MMC, "/", 0);
  createDir(SD_MMC, "/mydir");
  listDir(SD_MMC, "/", 0);
  removeDir(SD_MMC, "/mydir");
  listDir(SD_MMC, "/", 2);
  writeFile(SD_MMC, "/hello.txt", "Hello ");
  appendFile(SD_MMC, "/hello.txt", "World!\n");
  readFile(SD_MMC, "/hello.txt");
  deleteFile(SD_MMC, "/foo.txt");
  renameFile(SD_MMC, "/hello.txt", "/foo.txt");
  readFile(SD_MMC, "/foo.txt");
  testFileIO(SD_MMC, "/test.txt");
  Serial.printf("Total space: %lluMB\n", SD_MMC.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024));    

  return (true);
}

/************************************************/
/* Responde true si el sistema de ficheros de   */ 
/* la tarjeta SD esta disponible                */
/************************************************/
boolean SistemaFicherosSDClass::SDDisponible(void)
  {
  return SD_MMC.begin();
  }

/************************************************/
/* Recupera los datos de                        */
/* de un archivo cualquiera                     */
/************************************************/
boolean SistemaFicherosSDClass::leeFichero(String nombre, String &contenido)
  {
  boolean leido=false;
  
  Serial.println("Inicio de lectura de fichero " + nombre);

  if (SD_MMC.exists(nombre)) 
    {
    //file exists, reading and loading
    Serial.printf("Encontrado fichero de configuracion %s.\n",nombre.c_str());
    File configFile = SD_MMC.open(nombre, "r");
    if (configFile) 
      {
      if(debugGlobal) Serial.printf("Abierto fichero de configuracion %s.\n",configFile.name());
      size_t size = configFile.size();

      // Allocate a buffer to store contents of the file.
      char *buff=NULL;
      buff=(char *)malloc(size+1);

      configFile.readBytes(buff, size);
      buff[size]=0;//pongo el fin de cadena
        
      contenido=String(buff);
      Serial.printf("Contenido del fichero: #%s#\n",contenido.c_str());
      free(buff);
      leido=true;
        
      configFile.close();//cierro el fichero
      if(debugGlobal) Serial.println("Cierro el fichero");
      }//la de abrir el fichero de configuracion del WiFi
      else Serial.println("Fichero no se puede abrir");
    }//la de existe fichero
    else Serial.println("Fichero no existe");
  if(debugGlobal) Serial.printf("Longitud devuelta: %i",contenido.length());
  return leido;
  }
/**********************************************************************/
/* Salva la cadena pasada al fichero especificado                     */
/* Si ya existe lo sobreescribe                                       */
/**********************************************************************/  
boolean SistemaFicherosSDClass::salvaFichero(String nombreFichero, String nombreFicheroBak, String contenidoFichero)
  {
  boolean salvado=false;

  //file exists, reading and loading
  if(SD_MMC.exists(nombreFichero.c_str())) 
    {
    if(nombreFicheroBak!="")
      {
      Serial.printf("El fichero %s ya existe, se copiara con el nombre %s.\n",nombreFichero.c_str(),nombreFicheroBak.c_str());
        
      if(SD_MMC.exists(nombreFicheroBak.c_str())) SD_MMC.remove(nombreFicheroBak.c_str());  
      SD_MMC.rename(nombreFichero.c_str(),nombreFicheroBak.c_str());  
      }
    else Serial.printf("El fichero %s ya existe, sera sobreescrito.\n",nombreFichero.c_str());
    }

  Serial.print("Nombre fichero: ");
  Serial.println(nombreFichero.c_str());
  Serial.print("Contenido fichero: ");
  Serial.println(contenidoFichero.c_str());
   
  File newFile = SD_MMC.open(nombreFichero.c_str(), FILE_WRITE);//abro el fichero, si existe lo borra
  if (newFile) 
    {
    Serial.printf("Abierto fichero %s.\nGuardo contenido:\n#%s#\n",newFile.name(),contenidoFichero.c_str());
  
    newFile.print(contenidoFichero);
    newFile.close();//cierro el fichero
    Serial.println("Cierro el fichero");
    salvado=true;
    }
  else Serial.println("El fichero no se pudo abrir para escritura.\n");
      
  return salvado;
  }


/****************************************************/
/* Borra el fichero especificado                    */
/****************************************************/  
boolean SistemaFicherosSDClass::borraFichero(String nombreFichero)
  {
  boolean borrado=false;

  //file exists, reading and loading
  if(!SD_MMC.exists(nombreFichero)) Serial.println("El fichero " + nombreFichero + " no existe.");
  else
    {
    if (SD_MMC.remove(nombreFichero)) 
      {
      borrado=true;
      Serial.println("El fichero " + nombreFichero + " ha sido borrado.");
      }
    else Serial.println("No se pudo borrar el fichero " + nombreFichero + ".");
    }  

  return borrado;
  }  

/************************************************/
/* Recupera los ficheros almacenados en el      */
/* dispositivo. Devuelve una cadena separada    */
/* por SEPARADOR                                */
/************************************************/
boolean SistemaFicherosSDClass::listaFicheros(String &contenido)
  {   
  contenido="";

  File root = SD_MMC.open("/");
  File file = root.openNextFile();
 
  while(file)
    {
    Serial.print("FILE: ");
    Serial.println(file.name());

    contenido += String(file.name());
    contenido += SEPARADOR;
      
    file = root.openNextFile();
    }
    
  return (true);
  }  

/************************************************/
/* Devuelve si existe o no un fichero en el     */
/* dispositivo                                  */
/************************************************/
boolean SistemaFicherosSDClass::existeFichero(String nombre)
  {  
  return (SD_MMC.exists(nombre));
  }

/************************************************/
/* Recupera los datos de un archivo binario     */
/************************************************/
boolean SistemaFicherosSDClass::leeFicheroBin(String nombre, uint8_t *contenido,uint8_t inicio, uint16_t len)
  {
  boolean leido=false;
  size_t size=0;

  if(debugGlobal) Serial.println("Inicio de lectura de fichero binario" + nombre);

  if (SD_MMC.exists(nombre)) 
    {
    //file exists, reading and loading
    if(debugGlobal) Serial.printf("Encontrado fichero binario %s.\n",nombre.c_str());
    File binaryfile = SD_MMC.open(nombre, "r");
    if (binaryfile) 
      {
      Serial.printf("Abierto fichero binario %s.\n",binaryfile.name());
      size = binaryfile.size();
      if(size-inicio<len) return false; //no hay fichero suficiente para devolver len readBytes

      // Allocate a buffer to store contents of the file.
      uint8_t *buff=NULL;
      buff=(uint8_t *)malloc(size);
      binaryfile.read(buff, size);       
      memcpy(contenido,buff+inicio,len);
      free(buff);

      leido=true;        
      binaryfile.close();//cierro el fichero
      if(debugGlobal) Serial.println("Cierro el fichero");
      }//la de abrir el fichero de configuracion del WiFi
      else Serial.println("Fichero no se puede abrir");
    }//la de existe fichero
  else Serial.println("Fichero no existe");

  Serial.printf("Longitud devuelta: %i\n",size);
  return leido;
  }

/**********************************************************************/
/* Salva la cadena pasada al fichero especificado                     */
/* Si ya existe lo sobreescribe                                       */
/**********************************************************************/  
boolean SistemaFicherosSDClass:: salvaFicheroBin(String nombreFichero, String nombreFicheroBak, uint8_t *contenidoFichero, uint16_t len)
  {
  boolean salvado=false;

  //file exists, reading and loading
  if(SD_MMC.exists(nombreFichero.c_str()))  
    {
    if(nombreFicheroBak!="")
      {
      Serial.printf("El fichero %s ya existe, se copiara con el nombre %s.\n",nombreFichero.c_str(),nombreFicheroBak.c_str());
        
      if(SD_MMC.exists(nombreFicheroBak.c_str())) SD_MMC.remove(nombreFicheroBak.c_str());  
      SD_MMC.rename(nombreFichero.c_str(),nombreFicheroBak.c_str());  
      }
    else Serial.printf("El fichero %s ya existe, sera sobreescrito.\n",nombreFichero.c_str());
    }

  Serial.printf("Nombre fichero: %s\n",nombreFichero.c_str());
  Serial.printf("Longitud del contenido: %i\n",len);
   
  File newFile = SD_MMC.open(nombreFichero.c_str(), FILE_WRITE);//abro el fichero, si existe lo borra
  if (newFile) 
    {
    Serial.printf("Abierto fichero %s.\nGuardo %i bytes\n",newFile.name(),len);
    newFile.write(contenidoFichero,len);
    newFile.close();//cierro el fichero    
    Serial.println("Cierro el fichero");
    salvado=true;
    }
  else Serial.println("El fichero no se pudo abrir para escritura.\n");
      
  return salvado;
  }

/**********************************************************************/
/* Devuelve el tamaño del fichero indicado                           */
/**********************************************************************/  
uint16_t SistemaFicherosSDClass::tamanoFichero(String nombreFichero)
  {
  if(SD_MMC.exists(nombreFichero.c_str()))
    {
    File newFile = SD_MMC.open(nombreFichero.c_str(), "r");
    if(newFile) return newFile.size();
    }

  return -1;
  }

//Declaro la instancia unica
SistemaFicherosSDClass SistemaFicherosSD; 
