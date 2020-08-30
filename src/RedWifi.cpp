/**********************************************/
/*                                            */
/*  Gestion del WiFi                          */
/*  Control de la conexion, SSID, clave       */
/*  Control de la IP, DefaulGw, DNS...        */
/*                                            */
/**********************************************/

/***************************** Defines *****************************/
#define TRUE 1
#define FALSE 0
#define MAX_LONG_NOMBRE_DISPOSITIVO 32
#define WIFI_PORTAL_TIMEOUT 5*60 //5 minutos en segundos
#define TIME_OUT 15000//30000
#define DELAY 1000
/***************************** Defines *****************************/

/***************************** Includes *****************************/
//needed for library
#include <Global.h> //Lo tienen todos los modulos
#include <RedWiFi.h> 
/***************************** Includes *****************************/

void WiFiEvent(WiFiEvent_t event);

RedWifiClass::RedWifiClass(void) {};

void miSaveConfigCallback(void)
  {
  String cad="";
  
  Serial.println("---------------------Salvando configuracion---------------");
  Serial.println("Valores que voy a salvar");
  Serial.print("SSID : ");
  Serial.println(WiFi.SSID());
  Serial.print("Password : ");
  Serial.println(WiFi.psk());   

  if(!SistemaFicheros.leeFichero(WIFI_CONFIG_FILE, cad)) Serial.println("No se pudo leer el fichero");
  cad=RedWifi.generaJsonConfiguracionWifi(cad, WiFi.SSID(),WiFi.psk());
  if(!SistemaFicheros.salvaFichero(WIFI_CONFIG_FILE, WIFI_CONFIG_BAK_FILE, cad)) Serial.println("No se pudo salvar el fichero");  
  Serial.println("---------------------Fin salvando configuracion---------------");
  
  ///conectado=true;
  }

/*
void miAPCallback(WiFiManager *myWifiManager)
  {
  Serial.println("Portal de configuracion levantado");  
  Serial.print("Base activada con el nombre ");
  Serial.println(myWifiManager->getConfigPortalSSID());  
  }
*/

/************************************************/
/* Recupera los datos de configuracion          */
/* del archivo de Wifi                          */
/************************************************/
boolean RedWifiClass::recuperaDatosWiFi(boolean debug)
  {
  String cad="";
  if (debug) Serial.println("Recupero configuracion de archivo...");
   
  if(!SistemaFicheros.leeFichero(WIFI_CONFIG_FILE, cad)) 
    {
    //Confgiguracion por defecto
    Serial.printf("No existe fichero de configuracion WiFi\n");
    //cad="{\"wifi\": [ {\"ssid\": \"BASE0\" ,\"password\": \"11223344556677889900abcdef\"}, {\"ssid\": \"BASE1\" ,\"password\": \"11223344556677889900abcdef\"}, {\"ssid\": \"BASE2\" ,\"password\": \"11223344556677889900abcdef\"}, {\"ssid\": \"BASE-1\",\"password\": \"11223344556677889900abcdef\"}]}";
    //cad="{\"wifiIP\": \"0.0.0.0\",\"wifiGW\":\"0.0.0.0\",\"wifiNet\": \"0.0.0.0\",\"wifiDNS1\":\"0.0.0.0\",\"wifiDNS2\": \"0.0.0.0\",\"wifi\": []}";
    //if(SistemaFicheros.salvaFichero(WIFI_CONFIG_FILE, WIFI_CONFIG_BAK_FILE, cad)) Serial.printf("Fichero de configuracion WiFi creado por defecto\n");
    return false;
    }

  return(parseaConfiguracionWifi(cad));
  }

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio Wifi                         */
/*********************************************/
boolean RedWifiClass::parseaConfiguracionWifi(String contenido)
  {  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  if (json.success()) 
    {
    Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
    if (json.containsKey("wifiIP")) wifiIP.fromString((const char *)json["wifiIP"]); 
    if (json.containsKey("wifiGW")) wifiGW.fromString((const char *)json["wifiGW"]);
    if (json.containsKey("wifiNet")) wifiNet.fromString((const char *)json["wifiNet"]); 
    if (json.containsKey("wifiDNS1")) wifiDNS1.fromString((const char *)json["wifiDNS1"]);
    if (json.containsKey("wifiDNS2")) wifiDNS2.fromString((const char *)json["wifiDNS2"]);
    Serial.printf("Configuracion leida:\nIP actuador: %s\nIP Gateway: %s\nIPSubred: %s\nIP DNS1: %s\nIP DNS2: %s\n",wifiIP.toString().c_str(),wifiGW.toString().c_str(),wifiNet.toString().c_str(),wifiDNS1.toString().c_str(),wifiDNS2.toString().c_str());    

    JsonArray& wifi = json["wifi"];
    for(uint8_t i=0;i<wifi.size();i++)
      {
      const char* wifi_ssid = wifi[i]["ssid"];
      const char* wifi_password = wifi[i]["password"];
      MiWiFiMulti.addAP(wifi_ssid , wifi_password);
      Serial.printf("Red *%s* añadida.\n",wifi_ssid);
      }//del for
//************************************************************************************************
    return true;
    }
  return false;
  }

boolean RedWifiClass::inicializaWifi(boolean debug)
  {
  //cargo el valores por defecto
  wifiIP=IPAddress(0,0,0,0); //0.0.0.0 significa que no hay IP fija
  wifiGW=IPAddress(0,0,0,0);
  wifiNet=IPAddress(0,0,0,0);
  wifiDNS1=IPAddress(0,0,0,0);
  wifiDNS2=IPAddress(0,0,0,0);

  conectado=false; //Si el portal de configuracion devolvio OK a la conexion    

  //Registro eventos wifi
  WiFi.onEvent(WiFiEvent);
  // delete old config
  WiFi.disconnect(true);
  //Activo el modo de autoreconexion nuevo en version 1.5 (con el cambio a esp8266 2.4.2)
  WiFi.setAutoReconnect(true);

  if(recuperaDatosWiFi(debug))
    {
    //Configuro la IP fija
    if (wifiIP!=IPAddress(0,0,0,0) && wifiGW!=IPAddress(0,0,0,0))
      {
      Serial.printf("Datos WiFi: IP fija-> %s, GW-> %s, subnet-> %s, DNS1-> %s, DNS2-> %s\n",wifiIP.toString().c_str(), wifiGW.toString().c_str(), wifiNet.toString().c_str(), wifiDNS1.toString().c_str(), wifiDNS2.toString().c_str());
      WiFi.config(wifiIP, wifiGW, wifiNet, wifiDNS1, wifiDNS2);
      }
    else Serial.println("No hay IP fija");

    Serial.println("Conectando multibase");
    if (conectaMultibase(debug)) 
      {
      Serial.println("------------------------WiFi conectada (configuracion almacenada)--------------------------------------");
      Serial.println("WiFi conectada");
      WiFi.printDiag(Serial);
      Serial.print("IP : ");
      Serial.println(WiFi.localIP());
      Serial.println("-------------------------------------------------------------------------------------------------------");

      return true;
      }
    }

  Serial.println("Conectando autodetect");
  if (conectaAutodetect(debug))
    {
    Serial.println("------------------------WiFi conectada(autodetect)--------------------------------------");
    Serial.println("WiFi conectada");
    WiFi.printDiag(Serial);
    Serial.print("IP : ");
    Serial.println(WiFi.localIP());
    Serial.println("----------------------------------------------------------------------------------------");
    
    return true;
    }
  return false;
  }

/***********************************************************/
/*  Levanta el AP del dispositivo y espera conexiones para */
/*  configurar el acceso a al base wifi externa            */
/***********************************************************/ 
boolean RedWifiClass::conectaAutodetect(boolean debug)
  {
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  Serial.println("\n Entrando...");
  
  //WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom);    
  WiFiManagerParameter Nombre_Parametro("1","dispositivo",String(NOMBRE_FAMILIA).c_str(),MAX_LONG_NOMBRE_DISPOSITIVO+1,"Nombre del dispositivo");
  Serial.println(Nombre_Parametro.getID());
  Serial.println(Nombre_Parametro.getValue());
  Serial.println(Nombre_Parametro.getPlaceholder());
  Serial.println(Nombre_Parametro.getValueLength());
  Serial.println(Nombre_Parametro.getCustomHTML());
  wifiManager.addParameter(&Nombre_Parametro);
  
  //preparo la llamada a la funcion para salvar configuracion, 
  wifiManager.setSaveConfigCallback(miSaveConfigCallback);//Preparo la funcion para salvar la configuracion
  wifiManager.setConfigPortalTimeout(WIFI_PORTAL_TIMEOUT);
  //wifiManager.setAPCallback(miAPCallback);//llamada cuando se actie el portal de configuracion
  
  //Si se ha configurado IP fija
  if (wifiIP!=IPAddress(0,0,0,0)) wifiManager.setSTAStaticIPConfig(wifiIP,wifiGW,wifiNet);//Preparo la IP fija (IPAddress ip, IPAddress gw, IPAddress sn) 

  if (!wifiManager.startConfigPortal(("AP_"+String(NOMBRE_FAMILIA)).c_str())) 
    {
    Serial.println("failed to connect and hit timeout");
    ESP.restart();
    }
    
  setModoWifi(MODO_WIFI_AUTODETECT);
  return true;
  }

/***********************************************************/
/*  Conecta a la base definida en las variables SSID y     */
/*  password                                               */
/***********************************************************/ 
boolean RedWifiClass::conectaMonobase(boolean debug)
  { 
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) 
    {
    delay(100);
    if (debug) Serial.print("(MonoBase)Conectando WiFi....");
    }

  setModoWifi(MODO_WIFI_MONOBASE);
  return TRUE;
  }

/***********************************************************/
/*  Conecta a la base mediante las conexiones leidas del   */
/*  fichero, busca la de mayor potencia...                 */
/***********************************************************/ 
boolean RedWifiClass::conectaMultibase(boolean debug)
  {
  // wait for WiFi connection
  int time_out=0;
  while(MiWiFiMulti.run()!=WL_CONNECTED)
    {
    Serial.println("(Multi) Conectando Wifi...");  
    delay(DELAY);  
    time_out += DELAY;
    if (time_out>TIME_OUT) 
      {
      if (debug) Serial.println("No se pudo conectar al Wifi...");    
      return FALSE; //No se ha conectado y sale con KO
      }
    }

  setModoWifi(MODO_WIFI_MULTIBASE);
  return TRUE; //se ha conectado y sale con OK
  }

/***********************************************************/
/*  Intenta la reconexion al WiFi en el mpodo en que       */
/*  lo hizo inicialmente                                   */
/***********************************************************/ 
boolean RedWifiClass::reconectar(boolean debug) 
  {
  if (!WiFi.isConnected() && RedWifi.getModoWifi()!=MODO_WIFI_DESCONECTADO)
    {
    Serial.print("Reconectando al WiFi ");    
    WiFi.disconnect(false);
    WiFi.mode(WIFI_AP_STA);

    Serial.println("Conectando multibase");
    if (conectaMultibase(debug)) 
      {
      Serial.println("------------------------WiFi conectada (configuracion almacenada)--------------------------------------");
      Serial.println("WiFi conectada");
      WiFi.printDiag(Serial);
      Serial.print("IP : ");
      Serial.println(WiFi.localIP());
      Serial.println("-------------------------------------------------------------------------------------------------------");

      setModoWifi(MODO_WIFI_MULTIBASE);
      return true;
      }

    Serial.println("Conectando autodetect");
    if (conectaAutodetect(debug))
      {
      Serial.println("------------------------WiFi conectada(autodetect)--------------------------------------");
      Serial.println("WiFi conectada");
      WiFi.printDiag(Serial);
      Serial.print("IP : ");
      Serial.println(WiFi.localIP());
      Serial.println("----------------------------------------------------------------------------------------");
      
      setModoWifi(MODO_WIFI_AUTODETECT);
      return true;
      }
    
    //Solo llega aqui si no ha conectado
    delay(250);
    if ( WiFi.status() != WL_CONNECTED )return false;   
    }
    return true;//No estaba desconectado
  }


/**********************************************************************/
/*       Actualiza el modo de conecxion wifi monobase/multibase       */
/**********************************************************************/ 
void RedWifiClass::setModoWifi(modoWifi_t modo){ modoWifi=modo;}

/**********************************************************************/
/*       Devuelve el modo de conecxion wifi monobase/multibase        */
/**********************************************************************/ 
modoWifi_t RedWifiClass::getModoWifi(void){return modoWifi;}

/**********************************************************************/
/*            Devuelve la IP configurada en el dispositivo            */
/**********************************************************************/ 
String RedWifiClass::getIP(int debug) { return WiFi.localIP().toString();}

/*********************************************************************/
/*       Devuelve el nombre de la red a la que se ha conectado       */
/*********************************************************************/ 
String RedWifiClass::nombreSSID(void) {return WiFi.SSID();}  

/*********************************************************************/
/*             Watchdog de control para la conexion WiFi             */
/*********************************************************************/ 
void RedWifiClass::WifiWD(void) {if(WiFi.status() != WL_CONNECTED) ESP.restart();}

/**********************************************************************/
/* Salva la configuracion de las bases wifi conectada en formato json */
/**********************************************************************/  
String RedWifiClass::generaJsonConfiguracionWifi(String configActual, String ssid, String password)
  {
  boolean nuevo=true;
  String salida="";

  if(configActual=="") 
    {
    Serial.println("No existe el fichero. Se genera uno nuevo");
    return "{\"wifi\":[{\"ssid\":\"" + ssid + "\",\"password\":\"" + password + "\"}]}";;
    }
    
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(configActual.c_str());
  json.printTo(Serial);
  if (json.success()) 
    {
    Serial.println("parsed json");          
    JsonArray& wifi = json["wifi"];//parseo del ficehro que he leido
/************************/
    for(uint8_t i=0;i<wifi.size();i++)
      {
      if(strcmp(wifi[i]["ssid"],"NONE")) //Si la base no es NONE que es la que pongo cuando no hay fichero
        {
        //Comparo el que he leido del json con el que acabao de usar para conectar
        if (!strcmp(wifi[i]["ssid"],ssid.c_str())) //si ya existe actualizo la password
          {
          wifi[i]["password"] = password; //si son iguales guardo la password nueva
          Serial.printf("Se ha modificado la pass de %s\n",(const char *)wifi[i]["ssid"]);
          nuevo=false;//no es nuevo, ya he guardado el que acabo de usar
          }        
        }
      }//del for
      
    if(nuevo==true)
      {
      JsonObject& elemento = wifi.createNestedObject();
      elemento["ssid"] = ssid;
      elemento["password"] = password;
      Serial.printf("Red %s añadida\n",ssid.c_str());
      }
/************************/
    json.printTo(salida);//pinto el json que he creado
    Serial.printf("json creado:\n#%s#\n",salida.c_str());

    }//la de parsear el json

  return salida;  
  }

RedWifiClass RedWifi;

/********************************************************
 WiFi Events
0  SYSTEM_EVENT_WIFI_READY               < ESP32 WiFi ready
1  SYSTEM_EVENT_SCAN_DONE                < ESP32 finish scanning AP
2  SYSTEM_EVENT_STA_START                < ESP32 station start
3  SYSTEM_EVENT_STA_STOP                 < ESP32 station stop
4  SYSTEM_EVENT_STA_CONNECTED            < ESP32 station connected to AP
5  SYSTEM_EVENT_STA_DISCONNECTED         < ESP32 station disconnected from AP
6  SYSTEM_EVENT_STA_AUTHMODE_CHANGE      < the auth mode of AP connected by ESP32 station changed
7  SYSTEM_EVENT_STA_GOT_IP               < ESP32 station got IP from connected AP
8  SYSTEM_EVENT_STA_LOST_IP              < ESP32 station lost IP and the IP is reset to 0
9  SYSTEM_EVENT_STA_WPS_ER_SUCCESS       < ESP32 station wps succeeds in enrollee mode
10 SYSTEM_EVENT_STA_WPS_ER_FAILED        < ESP32 station wps fails in enrollee mode
11 SYSTEM_EVENT_STA_WPS_ER_TIMEOUT       < ESP32 station wps timeout in enrollee mode
12 SYSTEM_EVENT_STA_WPS_ER_PIN           < ESP32 station wps pin code in enrollee mode
13 SYSTEM_EVENT_AP_START                 < ESP32 soft-AP start
14 SYSTEM_EVENT_AP_STOP                  < ESP32 soft-AP stop
15 SYSTEM_EVENT_AP_STACONNECTED          < a station connected to ESP32 soft-AP
16 SYSTEM_EVENT_AP_STADISCONNECTED       < a station disconnected from ESP32 soft-AP
17 SYSTEM_EVENT_AP_STAIPASSIGNED         < ESP32 soft-AP assign an IP to a connected station
18 SYSTEM_EVENT_AP_PROBEREQRECVED        < Receive probe request packet in soft-AP interface
19 SYSTEM_EVENT_GOT_IP6                  < ESP32 station or ap or ethernet interface v6IP addr is preferred
20 SYSTEM_EVENT_ETH_START                < ESP32 ethernet start
21 SYSTEM_EVENT_ETH_STOP                 < ESP32 ethernet stop
22 SYSTEM_EVENT_ETH_CONNECTED            < ESP32 ethernet phy link up
23 SYSTEM_EVENT_ETH_DISCONNECTED         < ESP32 ethernet phy link down
24 SYSTEM_EVENT_ETH_GOT_IP               < ESP32 ethernet got IP from connected AP
25 SYSTEM_EVENT_MAX
*****************************************************************/
void WiFiEvent(WiFiEvent_t event)
  {
  boolean reconectar=false;

  Serial.printf("[WiFi-event] event: %d\n", event);

  switch (event) 
    {
    case SYSTEM_EVENT_WIFI_READY: 
        Serial.println("WiFi interface ready");
        break;
    case SYSTEM_EVENT_SCAN_DONE:
        Serial.println("Completed scan for access points");
        break;
    case SYSTEM_EVENT_STA_START:
        Serial.println("WiFi client started");
        break;
    case SYSTEM_EVENT_STA_STOP:
        Serial.println("WiFi clients stopped");
        reconectar=true;
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        Serial.println("Connected to access point");
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("Disconnected from WiFi access point");
        reconectar=true;
        break;
    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
        Serial.println("Authentication mode of access point has changed");
        reconectar=true;
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.print("Obtained IP address: ");
        Serial.println(WiFi.localIP());
        break;
    case SYSTEM_EVENT_STA_LOST_IP:
        Serial.println("Lost IP address and IP address is reset to 0");
        reconectar=true;
        break;
    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
        Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
        break;
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:
        Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
        break;
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
        Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
        break;
    case SYSTEM_EVENT_STA_WPS_ER_PIN:
        Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
        break;
    case SYSTEM_EVENT_AP_START:
        Serial.println("WiFi access point started");
        break;
    case SYSTEM_EVENT_AP_STOP:
        Serial.println("WiFi access point  stopped");
        break;
    case SYSTEM_EVENT_AP_STACONNECTED:
        Serial.println("Client connected");
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        Serial.println("Client disconnected");
        break;
    case SYSTEM_EVENT_AP_STAIPASSIGNED:
        Serial.println("Assigned IP address to client");
        break;
    case SYSTEM_EVENT_AP_PROBEREQRECVED:
        Serial.println("Received probe request");
        break;
    case SYSTEM_EVENT_GOT_IP6:
        Serial.println("IPv6 is preferred");
        break;
    case SYSTEM_EVENT_ETH_START:
        Serial.println("Ethernet started");
        break;
    case SYSTEM_EVENT_ETH_STOP:
        Serial.println("Ethernet stopped");
        break;
    case SYSTEM_EVENT_ETH_CONNECTED:
        Serial.println("Ethernet connected");
        break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
        Serial.println("Ethernet disconnected");
        break;
    case SYSTEM_EVENT_ETH_GOT_IP:
        Serial.println("Obtained IP address");
        break;
    default: break;
    }

  if(reconectar) RedWifi.reconectar(true);
  }