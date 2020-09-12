/*
 * Camara Lily TTGO T-Camera
 * 
 * Camara OV2640 y PIR
 * 
 * Servicio web levantado en puerto PUERTO_WEBSERVER
 */
 
/***************************** Defines *****************************/
//#define MAX_VUELTAS      UINT16_MAX // 65535 

// Una vuela de loop son ANCHO_INTERVALO segundos 
#define ANCHO_INTERVALO                 100 //Ancho en milisegundos de la rodaja de tiempo
#define FRECUENCIA_OTA                    5 //cada cuantas vueltas de loop atiende las acciones
#define FRECUENCIA_RECONOCIMIENTO_FACIAL  5 //cada cuantas vueltas de loop lee la imagen para reconocer una cara
#define FRECUENCIA_ENTRADAS               5 //cada cuantas vueltas de loop atiende las entradas
#define FRECUENCIA_SALIDAS                5 //cada cuantas vueltas de loop atiende las salidas
#define FRECUENCIA_SECUENCIADOR          10 //cada cuantas vueltas de loop atiende al secuenciador
#define FRECUENCIA_SERVIDOR_WEB           5 //cada cuantas vueltas de loop atiende el servidor web
#define FRECUENCIA_SERVIDOR_WEBSOCKET     1 //cada cuantas vueltas de loop atiende el servidor web
#define FRECUENCIA_SERVIDOR_FTP           5 //cada cuantas vueltas de loop atiende el servidor ftp
#define FRECUENCIA_MQTT                  10 //cada cuantas vueltas de loop envia y lee del broker MQTT
#define FRECUENCIA_ENVIO_DATOS           50 //cada cuantas vueltas de loop envia al broker el estado de E/S
#define FRECUENCIA_BOTONES                5 //cada cuantas vueltas de loop se atienden los botones
#define FRECUENCIA_OLED                   5 //cada cuantas vueltas de loop se atienden la pantalla OLED
#define FRECUENCIA_ORDENES                2 //cada cuantas vueltas de loop atiende las ordenes via serie 
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
#include <OTA.h>
#include <Ordenes.h>
#include <ServidorWeb.h>
#include <camara.h>
#include <faceRecon.h>
#include <botones.h>

#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
/***************************** Includes *****************************/

/***************************** variables globales *****************************/
//uint16_t vuelta = MAX_VUELTAS-100;//0; //vueltas de loop
int debugGlobal=0; //por defecto desabilitado
boolean trazaMemoria=false;
/***************************** variables globales *****************************/

/*************************************** SETUP ***************************************/      
void setup()
  {
  uint8_t tareasEnSetup=16;
  uint8_t orden=0;
  uint16_t x_init=14;
  uint16_t y_init=40;
  uint16_t ancho=100;
  uint16_t alto=10;
  int8_t desp_x=0;
  int8_t desp_y=-15;

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector  

  Serial.begin(115200);
   
  Serial.printf("\n\n\n");
  Serial.printf("*************** %s ***************\n",NOMBRE_FAMILIA);
  Serial.printf("*************** %s ***************\n",VERSION);
  Serial.println("***************************************************************");
  Serial.println("*                                                             *");
  Serial.println("*             Inicio del setup del modulo                     *");
  Serial.println("*                                                             *");
  Serial.println("***************************************************************");

  Serial.printf("\n\nInit Pantalla ---------------------------------------------------------------------\n");
  inicializaOled();

  Serial.printf("\n\nInit Ficheros ---------------------------------------------------------------------\n");
  //Ficheros - Lo primero para poder leer los demas ficheros de configuracion
  pintaProgresoTexto(x_init,y_init,ancho,alto,++orden,tareasEnSetup,"Ficheros...",desp_x,desp_y);
  if(!SistemaFicheros.inicializaFicheros(debugGlobal))Serial.println("Error al inicializar el sistema de ficheros");

  //Configuracion general
  Serial.printf("\n\nInit General --------------------------------------------------------------------------\n");
  pintaProgresoTexto(x_init,y_init,ancho,alto,++orden,tareasEnSetup,"General...",desp_x,desp_y);
  cacharro.inicializaConfiguracion(debugGlobal);

  //Wifi
  Serial.println("\n\nInit WiFi ---------------------------------------------------------------------------\n");
  pintaProgresoTexto(x_init,y_init,ancho,alto,++orden,tareasEnSetup,"WiFi...",desp_x,desp_y);
  if (RedWifi.inicializaWifi(1))//debugGlobal)) No tiene sentido debugGlobal, no hay manera de activarlo
    {
    /*----------------Inicializaciones que necesitan red-------------*/
    //OTA
    Serial.println("\n\nInit OTA ----------------------------------------------------------------------------\n");
    pintaProgresoTexto(x_init,y_init,ancho,alto,++orden,tareasEnSetup,"OTA...",desp_x,desp_y);
    inicializaOTA(debugGlobal);

    //SNTP
    Serial.printf("\n\nInit SNTP ----------------------------------------------------------------------------\n");
    pintaProgresoTexto(x_init,y_init,ancho,alto,++orden,tareasEnSetup,"Reloj...",desp_x,desp_y);
    reloj.inicializaReloj();    

    //MQTT
    Serial.println("\n\nInit MQTT ---------------------------------------------------------------------------\n");
    pintaProgresoTexto(x_init,y_init,ancho,alto,++orden,tareasEnSetup,"MQTT...",desp_x,desp_y);
    miMQTT.inicializaMQTT();

    //WebServer
    Serial.println("\n\nInit Web ----------------------------------------------------------------------------\n");
    pintaProgresoTexto(x_init,y_init,ancho,alto,++orden,tareasEnSetup,"FTP server...",desp_x,desp_y);
    inicializaWebServer();

    //FTPServer
    Serial.println("\n\nInit FTP ----------------------------------------------------------------------------\n");
    pintaProgresoTexto(x_init,y_init,ancho,alto,++orden,tareasEnSetup,"FTP server...",desp_x,desp_y);
    ftpSrv.inicializaFTP(0);
    }
  else Serial.println("No se pudo conectar al WiFi");

  //Entradas
  Serial.println("\n\nInit entradas ----------------------------------------------------------------------\n");
  pintaProgresoTexto(x_init,y_init,ancho,alto,++orden,tareasEnSetup,"Entradas...",desp_x,desp_y);
  Entradas.inicializaEntradas();

  //Salidas
  Serial.println("\n\nInit salidas -----------------------------------------------------------------------\n");
  pintaProgresoTexto(x_init,y_init,ancho,alto,++orden,tareasEnSetup,"Salidas...",desp_x,desp_y);
  Salidas.inicializaSalidas();

  //Camara
  Serial.println("\n\nInit camara ------------------------------------------------------------------------\n");
  pintaProgresoTexto(x_init,y_init,ancho,alto,++orden,tareasEnSetup,"Camara...",desp_x,desp_y);
  camara_init();

  //Websocket
  Serial.println("\n\nInit websocket ---------------------------------------------------------------------\n");  
  pintaProgresoTexto(x_init,y_init,ancho,alto,++orden,tareasEnSetup,"WebSockets...",desp_x,desp_y);WebSocket_init(true);

  //Reconocimiento facial
  Serial.println("\n\nInit reconocimiento facial ---------------------------------------------------------\n");
  pintaProgresoTexto(x_init,y_init,ancho,alto,++orden,tareasEnSetup,"FaceRecon...",desp_x,desp_y);
  faceRecon_init(true);
  
  //botones
  Serial.println("\n\nInit botones -----------------------------------------------------------------------\n");
  pintaProgresoTexto(x_init,y_init,ancho,alto,++orden,tareasEnSetup,"Botones...",desp_x,desp_y);
  botones_init(true);
  
  //Ordenes serie
  Serial.println("\n\nInit Ordenes -----------------------------------------------------------------------\n");  
  pintaProgresoTexto(x_init,y_init,ancho,alto,++orden,tareasEnSetup,"Ordenes...",desp_x,desp_y);
  Ordenes.inicializaOrden();//Inicializa los buffers de recepcion de ordenes desde PC

  pintaProgresoTexto(x_init,y_init,ancho,alto,++orden,tareasEnSetup,"Config OK",desp_x,desp_y);
  
  Serial.printf("\n\n");
  Serial.println("***************************************************************");
  Serial.println("*                                                             *");
  Serial.println("*               Fin del setup del modulo                      *");
  Serial.println("*                                                             *");    
  Serial.println("***************************************************************");
  Serial.printf("\n\n");  
  }  
/**********************************************************************************************/

/****************************************** LOOP **********************************************/
void loop()
  {  
  //referencia horaria de entrada en el bucle
  unsigned long EntradaBucle=0;
  EntradaBucle=millis();//Hora de entrada en la rodaja de tiempo

  //------------- EJECUCION DE TAREAS --------------------------------------
  //Acciones a realizar en el bucle   
  //Prioridad 0: OTA es prioritario.
  if ((cacharro.getVuelta() % FRECUENCIA_OTA)==0) gestionaOTA(); //Gestion de actualizacion OTA
  //Prioridad 2: Funciones de control.
  if ((cacharro.getVuelta() % FRECUENCIA_RECONOCIMIENTO_FACIAL)==0) reconocimientoFacial(debugGlobal); //atiende el servidor web
  if ((cacharro.getVuelta() % FRECUENCIA_ENTRADAS)==0) Entradas.consultaEntradas(debugGlobal); //comprueba las entradas
  if ((cacharro.getVuelta() % FRECUENCIA_SALIDAS)==0) Salidas.actualizaSalidas(debugGlobal); //comprueba las salidas
  //Prioridad 3: Interfaces externos de consulta    
  if ((cacharro.getVuelta() % FRECUENCIA_SERVIDOR_WEB)==0) webServer(debugGlobal); //atiende el servidor web
  if ((cacharro.getVuelta() % FRECUENCIA_SERVIDOR_FTP)==0) ftpSrv.handleFTP(); //atiende el servidor ftp
  if ((cacharro.getVuelta() % FRECUENCIA_SERVIDOR_WEBSOCKET)==0) atiendeWebsocket();
  if ((cacharro.getVuelta() % FRECUENCIA_MQTT)==0) miMQTT.atiendeMQTT(debugGlobal);      
  if ((cacharro.getVuelta() % FRECUENCIA_ENVIO_DATOS)==0) miMQTT.enviaDatos(debugGlobal); //publica via MQTT los datos de entradas y salidas, segun configuracion
  if ((cacharro.getVuelta() % FRECUENCIA_BOTONES)==0) atiendeBotones();
  if ((cacharro.getVuelta() % FRECUENCIA_OLED)==0) atiendeOled();
  if ((cacharro.getVuelta() % FRECUENCIA_ORDENES)==0) Ordenes.gestionaOrdenes(debugGlobal); //Lee ordenes via serie
  //------------- FIN EJECUCION DE TAREAS ---------------------------------  

  //sumo una vuelta de loop, si desborda inicializo vueltas a cero
  cacharro.incrementaVuelta();//sumo una vuelta de loop  

  //pinto la memoria libre    
  if(trazaMemoria)  Serial.printf("Vuelta: %i| freeMem: actual %i | minima: %i\n",cacharro.getVuelta(),esp_get_free_heap_size(),esp_get_minimum_free_heap_size());

  //Espero hasta el final de la rodaja de tiempo
  while(millis()<EntradaBucle+ANCHO_INTERVALO)
    {
    if(millis()<EntradaBucle) break; //cada 49 dias el contador de millis desborda
    delayMicroseconds(1000);
    }
  }
