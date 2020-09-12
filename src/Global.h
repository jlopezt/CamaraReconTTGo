/*
 * Definiciones comunes para todos los modulos
 */
 
/***************************** Defines *****************************/
#ifndef _GLOBAL_
#define _GLOBAL_

//Defines generales
#define NOMBRE_FAMILIA   "Camara reconocimiento facial"
#define VERSION          "1.3.3 (ESP32 1.0.4|Migracion C++|OTA|MQTT|FicherosWeb|FTP)"
#define SEPARADOR        '|'
#define SUBSEPARADOR     '#'
#define KO               -1
//#define OK                0

//Ficheros de configuracion
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
#define FTP_CONFIG_FILE                  "/FTPConfig.json"
#define FTP_CONFIG_BAK_FILE              "/FTPConfig.json.bak"
#define SECUENCIADOR_CONFIG_FILE         "/SecuenciadorConfig.json"
#define SECUENCIADOR_CONFIG_BAK_FILE     "/SecuenciadorConfig.json.bak"
#define MAQUINAESTADOS_CONFIG_FILE       "/MaqEstadosConfig.json"
#define MAQUINAESTADOS_CONFIG_BAK_FILE   "/MaqEstadosConfig.json.bak"
#define FACE_RECON_CONFIG_FILE           "/FaceReconConfig.json"
#define FACE_RECON_CONFIG_BAK_FILE       "/FaceReconConfig.json.bak"
#define FACE_RECON_FACES_FILE            "/FaceReconConfig.bin"
#define FACE_RECON_FACES_BAK_FILE        "/FaceReconConfig.bin.bak"

//Definicion de pines
#define MAX_PINES        2 //numero de pines disponibles para entradas y salidas
#define MAX_ENTRADAS     2 //numero maximo de entradas soportado
#define MAX_SALIDAS        MAX_PINES-MAX_ENTRADAS //numero maximo de salidas soportado

#define ANCHO_PULSO 1*1000 //Ancho del pulso en milisegundos

#ifndef NO_CONFIGURADO 
#define NO_CONFIGURADO -1
#endif

#ifndef CONFIGURADO 
#define CONFIGURADO     1
#endif

#define CONFIG_SPIRAM_SUPPORT 1
#define CONFIG_PICO_PSRAM_CS_IO 10
#define CONFIG_ESP32_SPIRAM_SUPPORT 1
#define CONFIG_SPIRAM_BOOT_INIT 1
#define CONFIG_SPIRAM_IGNORE_NOTFOUND 0
#define CONFIG_SPIRAM_TYPE_AUTO 1
#define CONFIG_SPIRAM_SIZE -1
#define CONFIG_SPIRAM_USE_MEMMAP 1
#define CONFIG_SPIRAM_SPEED_40M 1
#define SPIRAM_USE_MALLOC 1
#define CONFIG_SPIRAM_CACHE_WORKAROUND 1
#define CONFIG_SPIRAM_BANKSWITCH_ENABLE 1
#define CONFIG_SPIRAM_BANKSWITCH_RESERVE 8
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Arduino.h>
#include <FtpServer.h>
#include <SistemaFicheros.h>
#include <SistemaFicherosSD.h>
#include <ArduinoJson.h>
#include <cacharro.h>
#include <SNTP.h>
#include <Salidas.h>
#include <Entradas.h>
#include <Secuenciador.h>
#include <Wifi_MQTT.h>
#include <ArduinoOTA.h>
#include <RedWifi.h>
#include <camara.h>
#include <faceRecon.h>
#include <oled.h>
/***************************** Includes *****************************/
        
extern int debugGlobal;
extern boolean trazaMemoria;

#endif