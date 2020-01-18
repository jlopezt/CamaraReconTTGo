/*****************************************************************/
/*                                                               */
/* Serevicios de gestion del servidor Web incorporado al ESP32   */
/*                                                               */
/*****************************************************************/
/************************************************************************************************
Servicio                       URL                     Formato entrada Formato salida                                            Comentario                                            Ejemplo peticion              Ejemplo respuesta
Estado de los reles            http://IP/estado        N/A             <id_0>#<nombre_0>#<estado_0>|<id_1>#<nombre_1>#<estado_1> Devuelve el id de cada rele y su estado               http://IP/estado              1#1|2#0
Activa rele                    http://IP/activaRele    id=<id>         <id>#<estado>                                             Activa el rele indicado y devuelve el estado leido    http://IP/activaRele?id=1     1|1
Desactivarele                  http://IP/desactivaRele id=<id>         <id>#<estado>                                             Desactiva el rele indicado y devuelve el estado leido http://IP/desactivaRele?id=0  0|0
Test                           http://IP/test          N/A             HTML                                                      Verifica el estado del Actuadot   
Reinicia el actuador           http://IP/restart
Informacion del Hw del sistema http://IP/info
************************************************************************************************/

/***************************** Defines *****************************/
//Configuracion de los servicios web
#define PUERTO_WEBSERVER  80
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
#include <ServidorWeb.h>
#include <WebServer.h> //#include <ESP8266WebServer.h>
/***************************** Includes *****************************/

//Prototipo de funciones
bool handleFileRead(String path);

//Variables globales
WebServer server(PUERTO_WEBSERVER); //ESP8266WebServer server[MAX_WEB_SERVERS];

//Cadenas HTML precargadas
String cabeceraHTML="<HTML><HEAD><TITLE>" + cacharro.getNombreDispositivo() + "</TITLE></HEAD><BODY>";
String pieHTML="</BODY></HTML>";
String enlaces="<TABLE>\n<CAPTION>Enlaces</CAPTION>\n<TR><TD><a href=\"info\" target=\"_blank\">Info</a></TD></TR>\n<TR><TD><a href=\"test\" target=\"_blank\">Test</a></TD></TR>\n<TR><TD><a href=\"restart\" target=\"_blank\">Restart</a></TD></TR>\n<TR><TD><a href=\"estado\" target=\"_blank\">Estado</a></TD></TR>\n<TR><TD><a href=\"listaFicheros\" target=\"_blank\">Lista ficheros</a></TD></TR>\n<TR><TD><a href=\"estadoSalidas\" target=\"_blank\">Estado salidas</a></TD></TR>\n<TR><TD><a href=\"estadoEntradas\" target=\"_blank\">Estado entradas</a></TD></TR>\n<TR><TD><a href=\"planes\" target=\"_blank\">Planes del secuenciador</a></TD></TR></TABLE>\n"; 

/*******************************************************/
/*                                                     */ 
/* Invocado desde Loop, gestiona las peticiones web    */
/*                                                     */
/*******************************************************/
void webServer(int debug)
  {
  server.handleClient();  
  }

/************************* Gestores de las diferentes URL coniguradas ******************************/
/*************************************************/
/*                                               */
/*  Pagina principal. informacion de E/S         */
/*  Enlaces a las principales funciones          */
/*                                               */
/*************************************************/  
void handleRoot() 
  {
  String cad=cabeceraHTML;
  String orden="";

  //genero la respuesta por defecto  
  cad += "<h1>" + cacharro.getNombreDispositivo() + "</h1>";
  cad += "<BR>\n";
 
  //Enlaces
  cad += "<BR><BR>\n";
  cad += enlaces;
  cad += "<BR><BR>" + cacharro.getNombreDispositivo() + " . Version " + String(VERSION) + ".";

  server.send(200, "text/html", cad);
  }

/*************************************************/
/*                                               */
/*  Servicio de consulta de estado de            */
/*  las Salidas y las entradas                   */
/*  devuelve un formato json                     */
/*                                               */
/*************************************************/  
void handleEstado(void)
  {
  String cad="";
  
  server.send(200, "text/json", cad); 
  }  

/*********************************************/
/*                                           */
/*  Servicio de test                         */
/*                                           */
/*********************************************/  
void handleTest(void)
  {
  String cad=cabeceraHTML;
  cad += "<h1>" + cacharro.getNombreDispositivo() + "</h1>";
  
  cad += "Test OK<br>";
  cad += pieHTML;
    
  server.send(200, "text/html", cad); 
  }
  
/*********************************************/
/*                                           */
/*  Reinicia el dispositivo mediante         */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleRestart(void)
  {
  String cad=cabeceraHTML;
  cad += "<h1>" + cacharro.getNombreDispositivo() + "</h1>";
  
  cad += "Reiniciando...<br>";
  cad += pieHTML;
    
  server.send(200, "text/html", cad);     
  delay(100);
  ESP.restart();
  }

/*********************************************/
/*                                           */
/*  Lee info del chipset mediante            */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleInfo(void)
  {
  String cad=cabeceraHTML;
  cad += "<h1>" + cacharro.getNombreDispositivo() + "</h1>";

  cad+= "<BR>-----------------Uptime---------------------<BR>";
  char tempcad[20]="";
  sprintf(tempcad,"%" PRIu64, (esp_timer_get_time()/(unsigned long)1000000)); //la funcion esp_timer_get_time() devuelve el contador de microsegundos desde el arranque. rota cada 292.000 años  
  cad += "Uptime: " + String(tempcad) + " segundos"; //la funcion esp_timer_get_time() devuelve el contador de microsegundos desde el arranque. rota cada 292.000 años
  cad += "<BR>-----------------------------------------------<BR>";  

  cad+= "<BR>-----------------info logica-----------------<BR>";
  cad += "IP: " + String(RedWifi.getIP(debugGlobal));
  cad += "<BR>";  
  cad += "nivelActivo: " + String(cacharro.getNivelActivo());
  cad += "<BR>";  
  cad += "-----------------------------------------------<BR>";  
  
  cad += "<BR>-----------------WiFi info-----------------<BR>";
  cad += "SSID: " + RedWifi.nombreSSID();
  cad += "<BR>";    
  cad += "IP: " + WiFi.localIP().toString();
  cad += "<BR>";    
  cad += "Potencia: " + String(WiFi.RSSI());
  cad += "<BR>";    
  cad += "-----------------------------------------------<BR>";    
/*
  cad += "<BR>-----------------MQTT info-----------------<BR>";
  cad += "IP broker: " + IPBroker.toString();
  cad += "<BR>";
  cad += "Puerto broker: " +   puertoBroker=0;
  cad += "<BR>";  
  cad += "Usuario: " + usuarioMQTT="";
  cad += "<BR>";  
  cad += "Password: " + passwordMQTT="";
  cad += "<BR>";  
  cad += "Topic root: " + topicRoot="";
  cad += "<BR>";  
  cad += "-----------------------------------------------<BR>";  
*/
  cad += "<BR>-----------------Hardware info-----------------<BR>";
  cad += "FreeHeap: " + String(ESP.getFreeHeap());
  cad += "<BR>";
  cad += "ChipId: " + String(ESP.getChipRevision());
  cad += "<BR>";  
  cad += "SdkVersion: " + String(ESP.getSdkVersion());
  cad += "<BR>";  
  cad += "CpuFreqMHz: " + String(ESP.getCpuFreqMHz());
  cad += "<BR>";  
  cad += "FlashChipSize: " + String(ESP.getFlashChipSize());
  cad += "<BR>";  
  cad += "FlashChipSpeed: " + String(ESP.getFlashChipSpeed());
  cad += "<BR>";    
  cad += "-----------------------------------------------<BR>";  
  
  cad += pieHTML;
  server.send(200, "text/html", cad);     
  }

/*********************************************/
/*                                           */
/*  Lista los ficheros en el sistema a       */
/*  traves de una peticion HTTP              */ 
/*                                           */
/*********************************************/  
void handleListaFicheros(void)
  {
  String cad=cabeceraHTML;
  String nombreFichero="";
  String contenidoFichero="";

  cad += "<h1>" + cacharro.getNombreDispositivo() + "</h1>";
  cad += "<h2>Lista de ficheros</h2>";

  //Variables para manejar la lista de ficheros
  String contenido="";
  String fichero="";  
  int16_t to=0;
  
  if(SistemaFicheros.listaFicheros(contenido)) 
    {
    Serial.printf("contenido inicial= %s\n",contenido.c_str());      
    //busco el primer separador
    to=contenido.indexOf(SEPARADOR); 

    cad +="<style> table{border-collapse: collapse;} th, td{border: 1px solid black; padding: 10px; text-align: left;}</style>";
    cad += "<TABLE>";
    while(to!=-1)
      {
      fichero=contenido.substring(0, to);//cojo el principio como el fichero
      contenido=contenido.substring(to+1); //la cadena ahora es desde el separador al final del fichero anterior
      to=contenido.indexOf(SEPARADOR); //busco el siguiente separador

      cad += "<TR><TD>" + fichero + "</TD>";           
      cad += "<TD>";
      cad += "<form action=\"manageFichero\" target=\"_blank\">";
      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + fichero + "\">";
      cad += "    <input type=\"submit\" value=\"editar\">";
      cad += "</form>";
      cad += "</TD><TD>";
      cad += "<form action=\"copiarFicheroSD\" target=\"_blank\">";
      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + fichero + "\">";
      cad += "    <input type=\"submit\" value=\"copiarSD\">";
      cad += "</form>";
      cad += "</TD><TD>";
      cad += "<form action=\"borraFichero\" target=\"_blank\">";
      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + fichero + "\">";
      cad += "    <input type=\"submit\" value=\"borrar\">";
      cad += "</form>";
      cad += "</TD></TR>";
      //cad += "<TR><TD><a href=\"borraFichero?nombre="+ fichero +"\" target=\"_blank\">borrar</a></TD><TD><a href=\"manageFichero?nombre="+ fichero +"\" target=\"_blank\">" + fichero + "</a></TD></TR>\n";           
      }
    cad += "</TABLE>\n";
    cad += "<BR>";
    
    //Para crear un fichero nuevo
    cad += "<h2>Crear un fichero nuevo:</h2>";
    cad += "<table><tr><td>";      
    cad += "<form action=\"creaFichero\" target=\"_blank\">";
    cad += "  <p>";
    cad += "    Nombre:<input type=\"text\" name=\"nombre\" value=\"\">";
    cad += "    <BR>";
    cad += "    Contenido:<br><textarea cols=75 rows=20 name=\"contenido\"></textarea>";
    cad += "    <BR>";
    cad += "    <input type=\"submit\" value=\"salvar\">";
    cad += "  </p>";
    cad += "</td></tr></table>";      
    }
  else cad += "<TR><TD>No se pudo recuperar la lista de ficheros</TD></TR>"; 

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Lista los ficheros de la tarjeta SD      */
/*  atraves de una peticion HTTP             */ 
/*                                           */
/*********************************************/  
void handleListaFicherosSD(void)
  {
  String cad=cabeceraHTML;
  String nombreFichero="";
  String contenidoFichero="";

  cad += "<h1>" + cacharro.getNombreDispositivo() + "</h1>";
  cad += "<h2>Lista de ficheros</h2>";

  //Variables para manejar la lista de ficheros
  String contenido="";
  String fichero="";  
  int16_t to=0;
  
  if(SistemaFicherosSD.listaFicheros(contenido)) 
    {
    Serial.printf("contenido inicial= %s\n",contenido.c_str());      
    //busco el primer separador
    to=contenido.indexOf(SEPARADOR); 

    cad +="<style> table{border-collapse: collapse;} th, td{border: 1px solid black; padding: 10px; text-align: left;}</style>";
    cad += "<TABLE>";
    while(to!=-1)
      {
      fichero=contenido.substring(0, to);//cojo el principio como el fichero
      contenido=contenido.substring(to+1); //la cadena ahora es desde el separador al final del fichero anterior
      to=contenido.indexOf(SEPARADOR); //busco el siguiente separador

      cad += "<TR><TD>" + fichero + "</TD>";           
      cad += "<TD>";
      cad += "<form action=\"manageFichero\" target=\"_blank\">";
      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + fichero + "\">";
      cad += "    <input type=\"submit\" value=\"editarSD\">";
      cad += "</form>";
      cad += "</TD><TD>";
      cad += "<form action=\"copiarFicheroSD\" target=\"_blank\">";
      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + fichero + "\">";
      cad += "    <input type=\"submit\" value=\"copiarFD\">";
      cad += "</form>";
      cad += "</TD><TD>";
      cad += "<form action=\"borraFichero\" target=\"_blank\">";
      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + fichero + "\">";
      cad += "    <input type=\"submit\" value=\"borrarSD\">";
      cad += "</form>";
      cad += "</TD></TR>";
      //cad += "<TR><TD><a href=\"borraFichero?nombre="+ fichero +"\" target=\"_blank\">borrar</a></TD><TD><a href=\"manageFichero?nombre="+ fichero +"\" target=\"_blank\">" + fichero + "</a></TD></TR>\n";           
      }
    cad += "</TABLE>\n";
    cad += "<BR>";
    
    //Para crear un fichero nuevo
    cad += "<h2>Crear un fichero nuevo:</h2>";
    cad += "<table><tr><td>";      
    cad += "<form action=\"creaFichero\" target=\"_blank\">";
    cad += "  <p>";
    cad += "    Nombre:<input type=\"text\" name=\"nombre\" value=\"\">";
    cad += "    <BR>";
    cad += "    Contenido:<br><textarea cols=75 rows=20 name=\"contenido\"></textarea>";
    cad += "    <BR>";
    cad += "    <input type=\"submit\" value=\"salvarSD\">";
    cad += "  </p>";
    cad += "</td></tr></table>";      
    }
  else cad += "<TR><TD>No se pudo recuperar la lista de ficheros</TD></TR>"; 

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Copia un fichero de FS a SD a traves     */
/*  de una peticion HTTP                     */ 
/*                                           */
/*********************************************/  
void handleCopiarFicheroSD(void)
  {
  String cad=cabeceraHTML;
  String nombreFichero="";
  uint8_t *contenidoFichero=NULL;
  uint16_t tamano=0;

  cad += "<h1>" + cacharro.getNombreDispositivo() + "</h1>";

  if(!SistemaFicherosSD.SDDisponible()) cad += "La tarjeta SD no esta disponible<br>";
  else if(server.hasArg("nombre")) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");
    tamano=SistemaFicheros.tamanoFichero(nombreFichero);
    contenidoFichero=(uint8_t *)malloc(tamano);
    if (contenidoFichero==NULL) return;
    SistemaFicheros.leeFicheroBin(nombreFichero, contenidoFichero,0, tamano);

    if(SistemaFicherosSD.salvaFicheroBin(nombreFichero, nombreFichero+".bak", contenidoFichero,tamano)) cad += "Fichero copiado con exito<br>";
    else cad += "No se pudo copiar el fichero<br>"; 
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Copia un fichero de SD a FS a traves     */
/*  de una peticion HTTP                     */ 
/*                                           */
/*********************************************/  
void handleCopiarFicheroFS(void)
  {
  String cad=cabeceraHTML;
  String nombreFichero="";
  uint8_t *contenidoFichero=NULL;
  uint16_t tamano=0;

  cad += "<h1>" + cacharro.getNombreDispositivo() + "</h1>";

  if(!SistemaFicherosSD.SDDisponible()) cad += "La tarjeta SD no esta disponible<br>";
  else if(server.hasArg("nombre")) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");
    tamano=SistemaFicherosSD.tamanoFichero(nombreFichero);
    contenidoFichero=(uint8_t *)malloc(tamano);
    if (contenidoFichero==NULL) return;
    SistemaFicherosSD.leeFicheroBin(nombreFichero, contenidoFichero,0, tamano);

    if(SistemaFicheros.salvaFicheroBin(nombreFichero, nombreFichero+".bak", contenidoFichero,tamano)) cad += "Fichero copiado con exito<br>";
    else cad += "No se pudo copiar el fichero<br>"; 
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Crea un fichero a traves de una          */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleCreaFichero(void)
  {
  String cad=cabeceraHTML;
  String nombreFichero="";
  String contenidoFichero="";

  cad += "<h1>" + cacharro.getNombreDispositivo() + "</h1>";

  if(server.hasArg("nombre") && server.hasArg("contenido")) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");
    contenidoFichero=server.arg("contenido");

    if(SistemaFicheros.salvaFichero( nombreFichero, nombreFichero+".bak", contenidoFichero)) cad += "Fichero salvado con exito<br>";
    else cad += "No se pudo salvar el fichero<br>"; 
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Borra un fichero a traves de una         */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleBorraFichero(void)
  {
  String cad=cabeceraHTML;
  String nombreFichero="";
  String contenidoFichero="";

  cad += "<h1>" + cacharro.getNombreDispositivo() + "</h1>";
  
  if(server.hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");

    if(SistemaFicheros.borraFichero(nombreFichero)) cad += "El fichero " + nombreFichero + " ha sido borrado.\n";
    else cad += "No sepudo borrar el fichero " + nombreFichero + ".\n"; 
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Lee un fichero a traves de una           */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleLeeFichero(void)
  {
  String cad=cabeceraHTML;
  String nombreFichero="";
  String contenido="";
  
  cad += "<h1>" + cacharro.getNombreDispositivo() + "</h1>";
  
  if(server.hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");

    if(SistemaFicheros.leeFichero(nombreFichero, contenido))
      {
      cad += "El fichero tiene un tama&ntilde;o de ";
      cad += contenido.length();
      cad += " bytes.<BR>";           
      cad += "El contenido del fichero es:<BR>";
      cad += "<textarea readonly=true cols=75 rows=20 name=\"contenido\">";
      cad += contenido;
      cad += "</textarea>";
      cad += "<BR>";
      }
    else cad += "Error al abrir el fichero " + nombreFichero + "<BR>";      
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Gestion de un fichero a traves de una    */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleManageFichero(void)
  {
  String cad=cabeceraHTML;
  String nombreFichero="";
  String contenido="";
  
  cad += "<h1>" + cacharro.getNombreDispositivo() + "</h1>";
  
  if(server.hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");
    cad += "<h2>Fichero: " + nombreFichero + "</h2><BR>";  

    if(SistemaFicheros.leeFichero(nombreFichero, contenido))
      {
      cad += "El fichero tiene un tama&ntilde;o de ";
      cad += contenido.length();
      cad += " bytes.<BR>";
      cad += "El contenido del fichero es:<BR>";
      cad += "<textarea readonly=true cols=75 rows=20 name=\"contenido\">";
      cad += contenido;
      cad += "</textarea>";
      cad += "<BR>";

      cad += "<table><tr><td>";
      cad += "<form action=\"borraFichero\" target=\"_blank\">";
      cad += "  <p>";
      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + nombreFichero + "\">";
      cad += "    <input type=\"submit\" value=\"borrar\">";
      cad += "  </p>";
      cad += "</form>";
      cad += "</td></tr></table>";
      
      cad += "<table>Modificar fichero<tr><td>";      
      cad += "<form action=\"creaFichero\" target=\"_blank\">";
      cad += "  <p>";
      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + nombreFichero + "\">";
      cad += "    contenido del fichero: <br><textarea cols=75 rows=20 name=\"contenido\">" + contenido + "</textarea>";
      cad += "    <BR>";
      cad += "    <input type=\"submit\" value=\"salvar\">";
      cad += "  </p>";
      cad += "</td></tr></table>";
      }
    else cad += "Error al abrir el fichero " + nombreFichero + "<BR>";
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Pagina no encontrada                     */
/*                                           */
/*********************************************/
void handleNotFound()
  {
  if(handleFileRead(server.uri()))return;

  String message = "";

  message = "<h1>" + cacharro.getNombreDispositivo() + "<br></h1>";
  message += "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP___GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i=0; i<server.args(); i++)
    {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    
  server.send(404, "text/html", message);
  }

/*********************************** Inicializacion y configuracion *****************************************************************/
void inicializaWebServer(void)
  {
  /*******Configuracion del Servicio Web***********/  
  //Inicializo los serivcios  
  //decalra las URIs a las que va a responder
  server.on("/", handleRoot); //Responde con la iodentificacion del modulo
  server.on("/estado", handleEstado); //Servicio de estdo de reles
  
  server.on("/test", handleTest);  //URI de test

  server.on("/restart", handleRestart);  //URI de test
  server.on("/info", handleInfo);  //URI de test
  
  server.on("/listaFicheros", handleListaFicheros);  //URI de leer fichero
  server.on("/creaFichero", handleCreaFichero);  //URI de crear fichero
  server.on("/borraFichero", handleBorraFichero);  //URI de borrar fichero
  server.on("/leeFichero", handleLeeFichero);  //URI de leer fichero
  server.on("/copiarFicheroSD", handleCopiarFicheroSD);  //URI de crear fichero
  server.on("/copiarFicheroFS", handleCopiarFicheroFS);  //URI de crear fichero
  server.on("/manageFichero", handleManageFichero);  //URI de leer fichero

  //En la SD
  server.on("/listaFicherosSD", handleListaFicherosSD);  //URI de leer fichero de la tarjeta SD

  server.onNotFound(handleNotFound);//pagina no encontrada

  server.begin();

  Serial.printf("Servicio web iniciado en puerto %i\n", PUERTO_WEBSERVER);
  }

/**********************************************************************/
String getContentType(String filename) { // determine the filetype of a given filename, based on the extension
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path) 
  { // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  
  if (!path.startsWith("/")) path += "/";
  path = "/www" + path; //busco los ficheros en el SPIFFS en la carpeta www

  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if(SistemaFicheros.existeFichero(pathWithGz) || SistemaFicheros.existeFichero(path))
    { // If the file exists, either as a compressed archive, or normal
    if (SistemaFicheros.existeFichero(pathWithGz)) path += ".gz";  // If there's a compressed version available, use the compressed verion

    String contenido="";
    SistemaFicheros.leeFichero(path, contenido);
    server.send(200, "text/html", contenido);
    Serial.println(String("\tSent file: ") + path);
    return true;
    }
  Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  return false;
  }

/*
void handleFileUpload()
  {
  File fsUploadFile;
  HTTPUpload& upload = server.upload();
  String path;
 
  if(upload.status == UPLOAD_FILE_START)
    {
    path = upload.filename;
    if(!path.startsWith("/")) path = "/"+path;
    if(!path.startsWith("/www")) path = "/www"+path;
    if(!path.endsWith(".gz")) 
      {                          // The file server always prefers a compressed version of a file 
      String pathWithGz = path+".gz";                    // So if an uploaded file is not compressed, the existing compressed
      if(SPIFFS.exists(pathWithGz))                      // version of that file must be deleted (if it exists)
         SPIFFS.remove(pathWithGz);
      }
      
    Serial.print("handleFileUpload Name: "); Serial.println(path);
    fsUploadFile = SPIFFS.open(path, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    path = String();
    }
  else if(upload.status == UPLOAD_FILE_WRITE)
    {
    if(fsUploadFile) fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
    } 
  else if(upload.status == UPLOAD_FILE_END)
    {
    if(fsUploadFile) 
      {                                    // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      server.sendHeader("Location","/success.html");      // Redirect the client to the success page
      server.send(303);
      }
    else 
      {
      server.send(500, "text/plain", "500: couldn't create file");
      }
    }
  }
*/