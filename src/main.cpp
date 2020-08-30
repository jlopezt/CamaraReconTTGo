/*
 * Camara servomotorizada
 * 
 * Camara OV2640 y Micro Servos 9g
 * 
 * Servicio web levantado en puerto PUERTO_WEBSERVER
 */
 
/***************************** Defines *****************************/
#define MAX_VUELTAS      UINT16_MAX // 65535 
#define LED_BUILTIN               4 //GPIO del led de la placa en los ESP32-CAM   

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
#define FRECUENCIA_ORDENES                2 //cada cuantas vueltas de loop atiende las ordenes via serie 
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
#include <OTA.h>
#include <Ordenes.h>
#include <ServidorWeb.h>
#include <camara.h>
#include <faceRecon.h>
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
/***************************** Includes *****************************/

/***************************** variables globales *****************************/
uint16_t vuelta = MAX_VUELTAS-100;//0; //vueltas de loop
int debugGlobal=0; //por defecto desabilitado
boolean trazaMemoria=false;
/***************************** variables globales *****************************/

/************************* FUNCIONES PARA EL BUITIN LED ***************************/
void configuraLed(void){pinMode(LED_BUILTIN, OUTPUT);}
void enciendeLed(void){digitalWrite(LED_BUILTIN, HIGH);}
void apagaLed(void){digitalWrite(LED_BUILTIN, LOW);}
void parpadeaLed(uint8_t veces, uint16_t delayed=100)
  {
  for(uint8_t i=0;i<2*veces;i++)
    {  
    delay(delayed);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  }
/***********************************************************************************/  

/*************************************** SETUP ***************************************/      
void setup()
  {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector  

  Serial.begin(115200);
  configuraLed();
  enciendeLed();
   
  Serial.printf("\n\n\n");
  Serial.printf("*************** %s ***************\n",NOMBRE_FAMILIA);
  Serial.printf("*************** %s ***************\n",VERSION);
  Serial.println("***************************************************************");
  Serial.println("*                                                             *");
  Serial.println("*             Inicio del setup del modulo                     *");
  Serial.println("*                                                             *");
  Serial.println("***************************************************************");

  Serial.printf("\n\nInit Ficheros ---------------------------------------------------------------------\n");
  //Ficheros - Lo primero para poder leer los demas ficheros de configuracion
  if(!SistemaFicheros.inicializaFicheros(debugGlobal))Serial.println("Error al inicializar el sistema de ficheros");

  //Configuracion general
  Serial.printf("\n\nInit General --------------------------------------------------------------------------\n");
  cacharro.inicializaConfiguracion(debugGlobal);
  parpadeaLed(1);

  //Wifi
  Serial.println("\n\nInit WiFi ---------------------------------------------------------------------------\n");
  if (RedWifi.inicializaWifi(1))//debugGlobal)) No tiene sentido debugGlobal, no hay manera de activarlo
    {
    parpadeaLed(2);
    /*----------------Inicializaciones que necesitan red-------------*/
    //OTA
    Serial.println("\n\nInit OTA ----------------------------------------------------------------------------\n");
    inicializaOTA(debugGlobal);
    //SNTP
    Serial.printf("\n\nInit SNTP ----------------------------------------------------------------------------\n");
    reloj.inicializaReloj();    
    //MQTT
    Serial.println("\n\nInit MQTT ---------------------------------------------------------------------------\n");
    miMQTT.inicializaMQTT();
    //WebServer
    Serial.println("\n\nInit Web ----------------------------------------------------------------------------\n");
    inicializaWebServer();
    //FTPServer
    Serial.println("\n\nInit FTP ----------------------------------------------------------------------------\n");
    ftpSrv.inicializaFTP(0);
    }
  else Serial.println("No se pudo conectar al WiFi");
  parpadeaLed(1);
  apagaLed();

  //Entradas
  Serial.println("\n\nInit entradas ----------------------------------------------------------------------\n");
  Entradas.inicializaEntradas();

  //Salidas
  Serial.println("\n\nInit salidas -----------------------------------------------------------------------\n");
  Salidas.inicializaSalidas();

  //Camara
  Serial.println("\n\nInit camara ------------------------------------------------------------------------\n");
  camara_init();

  //Websocket
  Serial.println("\n\nInit websocket ---------------------------------------------------------------------\n");  
  WebSocket_init(true);

  //Reconocimiento facial
  Serial.println("\n\nInit reconocimiento facial ---------------------------------------------------------\n");
  faceRecon_init(true);
  
  //Ordenes serie
  Serial.println("\n\nInit Ordenes -----------------------------------------------------------------------\n");  
  Ordenes.inicializaOrden();//Inicializa los buffers de recepcion de ordenes desde PC

  parpadeaLed(1,500);
  apagaLed();//Por si acaso...
  
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
  if ((vuelta % FRECUENCIA_OTA)==0) gestionaOTA(); //Gestion de actualizacion OTA
  //Prioridad 2: Funciones de control.
  if ((vuelta % FRECUENCIA_RECONOCIMIENTO_FACIAL)==0) reconocimientoFacial(debugGlobal); //atiende el servidor web
  if ((vuelta % FRECUENCIA_ENTRADAS)==0) Entradas.consultaEntradas(debugGlobal); //comprueba las entradas
  if ((vuelta % FRECUENCIA_SALIDAS)==0) Salidas.actualizaSalidas(debugGlobal); //comprueba las salidas
  //Prioridad 3: Interfaces externos de consulta    
  if ((vuelta % FRECUENCIA_SERVIDOR_WEB)==0) webServer(debugGlobal); //atiende el servidor web
  if ((vuelta % FRECUENCIA_SERVIDOR_FTP)==0) ftpSrv.handleFTP(); //atiende el servidor ftp
  if ((vuelta % FRECUENCIA_SERVIDOR_WEBSOCKET)==0) atiendeWebsocket();
  if ((vuelta % FRECUENCIA_MQTT)==0) miMQTT.atiendeMQTT(debugGlobal);      
  if ((vuelta % FRECUENCIA_ENVIO_DATOS)==0) miMQTT.enviaDatos(debugGlobal); //publica via MQTT los datos de entradas y salidas, segun configuracion
  if ((vuelta % FRECUENCIA_ORDENES)==0) Ordenes.gestionaOrdenes(debugGlobal); //Lee ordenes via serie
  //------------- FIN EJECUCION DE TAREAS ---------------------------------  

  //sumo una vuelta de loop, si desborda inicializo vueltas a cero
  vuelta++;//sumo una vuelta de loop  

  //pinto la memoria libre    
  if(trazaMemoria)  Serial.printf("Vuelta: %i| freeMem: actual %i | minima: %i\n",vuelta,esp_get_free_heap_size(),esp_get_minimum_free_heap_size());

  //Espero hasta el final de la rodaja de tiempo
  while(millis()<EntradaBucle+ANCHO_INTERVALO)
    {
    if(millis()<EntradaBucle) break; //cada 49 dias el contador de millis desborda
    delayMicroseconds(1000);
    }
  }
