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
/***************************** Defines *****************************/

/***************************** Includes *****************************/
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