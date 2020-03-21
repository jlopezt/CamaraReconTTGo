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

typedef enum{
  MODO_WIFI_DESCONECTADO, //sin conexion, modo incial
  MODO_WIFI_MONOBASE,     //Conectado a una base definida
  MODO_WIFI_MULTIBASE,    //Conectado a una de las bases de la lista
  MODO_WIFI_AUTODETECT,   //Conectado por el portal de lconfiguracion
  MODO_WIFI_MAX           //Final de la lista, no es un valor valido
}modoWifi_t;

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

  modoWifi_t modoWifi=MODO_WIFI_DESCONECTADO;

  void setModoWifi(modoWifi_t modo);

  public:
  RedWifiClass();
  
  boolean recuperaDatosWiFi(boolean debug);
  boolean parseaConfiguracionWifi(String contenido);
  boolean inicializaWifi(boolean debug);
  boolean conectaAutodetect(boolean debug);
  boolean conectaMonobase(boolean debug);
  boolean conectaMultibase(boolean debug);
  boolean reconectar(boolean debug);

  String getIP(int debug);
  String nombreSSID(void);  
  void WifiWD(void);
  
  modoWifi_t getModoWifi(void);

  String generaJsonConfiguracionWifi(String configActual, String ssid, String password);
  };

extern RedWifiClass RedWifi;

#endif
