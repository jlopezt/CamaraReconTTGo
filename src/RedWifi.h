/**********************************************/
/*                                            */
/*  Gestion del WiFi                          */
/*  Control de la conexion, SSID, clave       */
/*  Control de la IP, DefaulGw, DNS...        */
/*                                            */
/**********************************************/
/***************************** Defines *****************************/
#ifndef _REDWIFI_
#define _REDWIFI_

#define TRUE 1
#define FALSE 0
#define MAX_LONG_NOMBRE_DISPOSITIVO 32
#define WIFI_PORTAL_TIMEOUT 5*60 //5 minutos en segundos
#define TIME_OUT 30000
#define DELAY 1000
/***************************** Defines *****************************/

/***************************** Includes *****************************/
//needed for library
#include <Global.h> //Lo tienen todos los modulos
#include <WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <WiFiMulti.h>
/***************************** Includes *****************************/

void miSaveConfigCallback(void);

class RedWifiClass
  {
  private:
  IPAddress wifiIP;
  IPAddress wifiNet;
  IPAddress wifiGW;
  IPAddress wifiDNS1;
  IPAddress wifiDNS2;

  const char* ssid;
  const char* password;

  WiFiMulti MiWiFiMulti;

  boolean conectado; //Si el portal de configuracion devolvio OK a la conexion

  public:
  RedWifiClass();
  
  boolean recuperaDatosWiFi(boolean debug);
  boolean parseaConfiguracionWifi(String contenido);
  boolean inicializaWifi(boolean debug);
  boolean conectaAutodetect(boolean debug);
  boolean conectaMonobase(boolean debug);
  boolean conectaMultibase(boolean debug);

  String getIP(int debug);
  String nombreSSID(void);  
  void WifiWD(void);
  
  String generaJsonConfiguracionWifi(String configActual, String ssid, String password);
  };

extern RedWifiClass RedWifi;

#endif