/*
 * Actuador con E/S
 *
 * Actuador remoto
 * 
 * Reles de conexion y entradas digitales
 * 
 * Servicio web levantado en puerto PUERTO_WEBSERVER
 */
 
/***************************** Defines *****************************/
#define MAX_VUELTAS      UINT16_MAX// 32767 

// Una vuela de loop son ANCHO_INTERVALO segundos 
#define ANCHO_INTERVALO         100 //Ancho en milisegundos de la rodaja de tiempo
#define FRECUENCIA_OTA            5 //cada cuantas vueltas de loop atiende las acciones
#define FRECUENCIA_ENTRADAS       5 //cada cuantas vueltas de loop atiende las entradas
#define FRECUENCIA_SALIDAS        5 //cada cuantas vueltas de loop atiende las salidas
#define FRECUENCIA_SECUENCIADOR  10 //cada cuantas vueltas de loop atiende al secuenciador
#define FRECUENCIA_SERVIDOR_WEB   1 //cada cuantas vueltas de loop atiende el servidor web
#define FRECUENCIA_MQTT          10 //cada cuantas vueltas de loop envia y lee del broker MQTT
#define FRECUENCIA_ENVIO_DATOS   50 //cada cuantas vueltas de loop envia al broker el estado de E/S
#define FRECUENCIA_ORDENES        2 //cada cuantas vueltas de loop atiende las ordenes via serie 
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
#include <cacharro.h>
#include <OTA.h>
#include <ArduinoJson.h>
#include <SistemaFicheros.h>
#include <SNTP.h>
#include <RedWifi.h>
#include <Ordenes.h>
#include <Wifi_MQTT.h>

#include <Secuenciador.h>
#include <ServidorWeb.h>
/***************************** Includes *****************************/

/***************************** variables globales *****************************/
uint16_t vuelta = MAX_VUELTAS-100;//0; //vueltas de loop
int debugGlobal=0; //por defecto desabilitado
boolean candado=false; //Candado de configuracion. true implica que la ultima configuracion fue mal
/***************************** variables globales *****************************/
      
void setup()
  {
  Serial.begin(115200);
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
  SistemaFicheros.inicializaFicheros(debugGlobal);

  //Compruebo si existe candado, si existe la ultima configuracion fue mal
  if(SistemaFicheros.existeFichero(FICHERO_CANDADO)) 
    {
    Serial.printf("Candado puesto. Configuracion por defecto");
    candado=true; 
    debugGlobal=1;
    }
  else
    {
    candado=false;
    //Genera candado
    if(SistemaFicheros.salvaFichero(FICHERO_CANDADO,"","JSD")) Serial.println("Candado creado");
    else Serial.println("ERROR - No se pudo crear el candado");
    }
 
  //Configuracion general
  Serial.printf("\n\nInit General ---------------------------------------------------------------------\n");
  cacharro.inicializaConfiguracion(debugGlobal);

  //Wifi
  Serial.println("\n\nInit WiFi -----------------------------------------------------------------------\n");
  if (RedWifi.inicializaWifi(1))//debugGlobal)) No tiene sentido debugGlobal, no hay manera de activarlo
    {
    /*----------------Inicializaciones que necesitan red-------------*/
    //OTA
    Serial.println("\n\nInit OTA -----------------------------------------------------------------------\n");
    inicializaOTA(debugGlobal);
    //SNTP
    Serial.printf("\n\nInit SNTP ----------------------------------------------------------------------\n");
    reloj.inicializaReloj();    
    //MQTT
    Serial.println("\n\nInit MQTT -----------------------------------------------------------------------\n");
    miMQTT.inicializaMQTT();
    //WebServer
    Serial.println("\n\nInit Web --------------------------------------------------------------------------\n");
    inicializaWebServer();
    }
  else Serial.println("No se pudo conectar al WiFi");

  //Entradas y Salidas
  Serial.println("\n\nInit entradas ------------------------------------------------------------------------\n");
  Entradas.inicializaEntradas();

  Serial.println("\n\nInit salidas ------------------------------------------------------------------------\n");
  Salidas.inicializaSalidas();

  //Secuenciador
  Serial.println("\n\nInit secuenciador ---------------------------------------------------------------------\n");
  Secuenciador.inicializaSecuenciador();
  
  //Ordenes serie
  Serial.println("\n\nInit Ordenes ----------------------------------------------------------------------\n");  
  Ordenes.inicializaOrden();//Inicializa los buffers de recepcion de ordenes desde PC

  //Si ha llegado hasta aqui, todo ha ido bien y borro el candado
  if(SistemaFicheros.borraFichero(FICHERO_CANDADO))Serial.println("Candado borrado");
  else Serial.println("ERROR - No se pudo borrar el candado");
  
  Serial.printf("\n\n");
  Serial.println("***************************************************************");
  Serial.println("*                                                             *");
  Serial.println("*               Fin del setup del modulo                      *");
  Serial.println("*                                                             *");    
  Serial.println("***************************************************************");
  Serial.printf("\n\n");  
  }  

void loop()
  {  
  //referencia horaria de entrada en el bucle
  time_t EntradaBucle=0;
  EntradaBucle=millis();//Hora de entrada en la rodaja de tiempo

  //------------- EJECUCION DE TAREAS --------------------------------------
  //Acciones a realizar en el bucle   
  //Prioridad 0: OTA es prioritario.
  if ((vuelta % FRECUENCIA_OTA)==0) gestionaOTA(); //Gestion de actualizacion OTA
  //Prioridad 2: Funciones de control.
  if ((vuelta % FRECUENCIA_ENTRADAS)==0) Entradas.consultaEntradas(debugGlobal); //comprueba las entradas
  if ((vuelta % FRECUENCIA_SALIDAS)==0) Salidas.actualizaSalidas(debugGlobal); //comprueba las salidas
  if ((vuelta % FRECUENCIA_SECUENCIADOR)==0) Secuenciador.actualizaSecuenciador(debugGlobal); //Actualiza la salida del secuenciador
  //Prioridad 3: Interfaces externos de consulta    
  if ((vuelta % FRECUENCIA_SERVIDOR_WEB)==0) webServer(debugGlobal); //atiende el servidor web
  if ((vuelta % FRECUENCIA_MQTT)==0) miMQTT.atiendeMQTT(debugGlobal);      
  if ((vuelta % FRECUENCIA_ENVIO_DATOS)==0) miMQTT.enviaDatos(debugGlobal); //publica via MQTT los datos de entradas y salidas, segun configuracion
  if ((vuelta % FRECUENCIA_ORDENES)==0) Ordenes.gestionaOrdenes(debugGlobal); //Lee ordenes via serie
  //------------- FIN EJECUCION DE TAREAS ---------------------------------  

  //sumo una vuelta de loop, si desborda inicializo vueltas a cero
  vuelta++;//sumo una vuelta de loop  
      
  //Espero hasta el final de la rodaja de tiempo
  while(millis()<EntradaBucle+ANCHO_INTERVALO)
    {
    if(millis()<EntradaBucle) break; //cada 49 dias el contador de millis desborda
    delayMicroseconds(1000);
    }
  }

