/*
 * Definiciones comunes para todos los modulos
 */
 
/***************************** Defines *****************************/
#ifndef _GLOBAL_
#define _GLOBAL_

//Defines generales
#define NOMBRE_FAMILIA   "Camara servomotorizada"
#define VERSION          "1.0.0 (ESP32 1.0.2|Migracion C++|OTA|MQTT|FicherosWeb)"
#define SEPARADOR        '|'
#define SUBSEPARADOR     '#'
#define KO               -1
//#define OK                0


//Ficheros de configuracion
#define FICHERO_CANDADO                  "/Candado"
#define GLOBAL_CONFIG_FILE               "/Config.json"
#define GLOBAL_CONFIG_BAK_FILE           "/Config.json.bak"
#define WIFI_CONFIG_FILE                 "/WiFiConfig.json"
#define WIFI_CONFIG_BAK_FILE             "/WiFiConfig.json.bak"
#define MQTT_CONFIG_FILE                 "/MQTTConfig.json"
#define MQTT_CONFIG_BAK_FILE             "/MQTTConfig.json.bak"

#define ANCHO_PULSO 1*1000 //Ancho del pulso en milisegundos

#ifndef NO_CONFIGURADO 
#define NO_CONFIGURADO -1
#endif

#ifndef CONFIGURADO 
#define CONFIGURADO     1
#endif
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Arduino.h>
#include <SistemaFicheros.h>
#include <ArduinoJson.h>
#include <cacharro.h>
#include <SNTP.h>
#include <Wifi_MQTT.h>
#include <ArduinoOTA.h>
#include <RedWifi.h>
/***************************** Includes *****************************/
        
extern int debugGlobal;

#endif