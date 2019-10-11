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
#define TIME_OUT 30000
#define DELAY 1000
/***************************** Defines *****************************/

/***************************** Includes *****************************/
//needed for library
#include <Global.h> //Lo tienen todos los modulos
#include <RedWiFi.h> 
/***************************** Includes *****************************/

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

  if(!SistemaFicheros.leeFicheroConfig(WIFI_CONFIG_FILE, cad)) Serial.println("No se pudo leer el fichero");
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
   
  if(!SistemaFicheros.leeFicheroConfig(WIFI_CONFIG_FILE, cad)) 
    {
    //Confgiguracion por defecto
    Serial.printf("No existe fichero de configuracion WiFi\n");
    //cad="{\"wifi\": [ {\"ssid\": \"BASE0\" ,\"password\": \"11223344556677889900abcdef\"}, {\"ssid\": \"BASE1\" ,\"password\": \"11223344556677889900abcdef\"}, {\"ssid\": \"BASE2\" ,\"password\": \"11223344556677889900abcdef\"}, {\"ssid\": \"BASE-1\",\"password\": \"11223344556677889900abcdef\"}]}";
    cad="{\"wifiIP\": \"0.0.0.0\",\"wifiGW\":\"0.0.0.0\",\"wifiNet\": \"0.0.0.0\",\"wifiDNS1\":\"0.0.0.0\",\"wifiDNS2\": \"0.0.0.0\",\"wifi\": []}";
    if(SistemaFicheros.salvaFicheroConfig(WIFI_CONFIG_FILE, WIFI_CONFIG_BAK_FILE, cad)) Serial.printf("Fichero de configuracion WiFi creado por defecto\n");
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

  if(recuperaDatosWiFi(debug))
    {
    //Configuro la IP fija
    if (wifiIP!=IPAddress(0,0,0,0) && wifiGW!=IPAddress(0,0,0,0))
      {
      Serial.printf("Datos WiFi: IP fija-> %s, GW-> %s, subnet-> %s, DNS1-> %s, DNS2-> %s\n",wifiIP.toString().c_str(), wifiGW.toString().c_str(), wifiNet.toString().c_str(), wifiDNS1.toString().c_str(), wifiDNS2.toString().c_str());
      WiFi.config(wifiIP, wifiGW, wifiNet, wifiDNS1, wifiDNS2);
      }
    else Serial.println("No hay IP fija");

    //Activo el modo de autoreconexion nuevo en version 1.5 (con el cambio a esp8266 2.4.2)
    WiFi.setAutoReconnect(true);

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
    
  return true;
  }

boolean RedWifiClass::conectaMonobase(boolean debug)
  { 
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) 
    {
    delay(100);
    if (debug) Serial.print("(MonoBase)Conectando WiFi....");
    }
 
  return TRUE;
  }

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
    
  return TRUE; //se ha conectado y sale con OK
  }

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