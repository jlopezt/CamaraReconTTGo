/*
 * Definiciones comunes para todos los modulos
 */
 
/***************************** Defines *****************************/
#ifndef _GLOBAL_
#define _GLOBAL_

//Defines generales
#define NOMBRE_FAMILIA   "Actuador/Secuenciador (E/S)"
#define VERSION          "5.0.0 (ESP32 1.0.2|Migracion C++|OTA|MQTT|Logic+|Secuenciador|FicherosWeb)"
#define SEPARADOR        '|'
#define SUBSEPARADOR     '#'
#define KO               -1
//#define OK                0


//Ficheros de configuracion
#define FICHERO_CANDADO                  "/Candado"
#define GLOBAL_CONFIG_FILE               "/Config.json"
#define GLOBAL_CONFIG_BAK_FILE           "/Config.json.bak"
#define ENTRADAS_CONFIG_FILE             "/EntradasConfig.json"
#define ENTRADAS_CONFIG_BAK_FILE         "/EntradasConfig.json.bak"
#define SALIDAS_CONFIG_FILE              "/SalidasConfig.json"
#define SALIDAS_CONFIG_BAK_FILE          "/SalidasConfig.json.bak"
#define WIFI_CONFIG_FILE                 "/WiFiConfig.json"
#define WIFI_CONFIG_BAK_FILE             "/WiFiConfig.json.bak"
#define MQTT_CONFIG_FILE                 "/MQTTConfig.json"
#define MQTT_CONFIG_BAK_FILE             "/MQTTConfig.json.bak"
#define SECUENCIADOR_CONFIG_FILE         "/SecuenciadorConfig.json"
#define SECUENCIADOR_CONFIG_BAK_FILE     "/SecuenciadorConfig.json.bak"

//Definicion de pines
#define MAX_PINES        7 //numero de pines disponibles para entradas y salidas
#define MAX_ENTRADAS     4 //numero maximo de reles soportado
#define MAX_RELES        MAX_PINES-MAX_ENTRADAS //numero maximo de salidas

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
#include <Salidas.h>
#include <Entradas.h>
#include <Secuenciador.h>
#include <Wifi_MQTT.h>
#include <ArduinoOTA.h>
#include <RedWifi.h>
/***************************** Includes *****************************/
        
extern int debugGlobal;

#endif