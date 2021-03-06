/**********************************************/
/*                                            */
/*  Gestion de la conexion MQTT               */
/*  Incluye la conexion al bus y la           */
/*  definicion del callback de suscripcion    */
/*                                            */
/* Librria de sooprte del protocolo MQTT      */
/* para arduino/ESP8266/ESP32                 */
/*                                            */
/* https://pubsubclient.knolleary.net/api.html*/
/**********************************************/

/***************************** Defines *****************************/
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Wifi_MQTT.h>
/***************************** Includes *****************************/

miMQTTClass miMQTT;
PubSubClient clienteMQTT(miMQTT.espClient);

/************************************************/
/* Inicializa valiables y estado del bus MQTT   */
/************************************************/
void miMQTTClass::inicializaMQTT(void)
  {
  //recupero datos del fichero de configuracion
  if (!recuperaDatosMQTT(false)) Serial.printf("error al recuperar config MQTT.\nConfiguracion por defecto.\n");

	//Si va bien inicializo con los valores correstoc, si no con valores por defecto
  //confituro el servidor y el puerto
  clienteMQTT.setServer(IPBroker, puertoBroker);
  //configuro el callback, si lo hay
  clienteMQTT.setCallback(callbackMQTT);

  if (conectaMQTT()) Serial.println("connectado al broker");  
  else Serial.printf("error al conectar al broker con estado %i\n",clienteMQTT.state());
  }

/************************************************/
/* Recupera los datos de configuracion          */
/* del archivo de MQTT                          */
/************************************************/
boolean miMQTTClass::recuperaDatosMQTT(boolean debug)
  {
  String cad="";
  if (debug) Serial.println("Recupero configuracion de archivo...");

  //cargo el valores por defecto
  IPBroker.fromString("0.0.0.0");
  ID_MQTT=String(NOMBRE_FAMILIA); //ID del modulo en su conexion al broker
  puertoBroker=0;
  usuarioMQTT="";
  passwordMQTT="";
  topicRoot=""; 
  publicarEntradas=1; 
  publicarSalidas=1;    

  if(!SistemaFicheros.leeFichero(MQTT_CONFIG_FILE, cad)) 
    {
    //Confgiguracion por defecto
    Serial.printf("No existe fichero de configuracion MQTT\n");
    return false;
    }

  return parseaConfiguracionMQTT(cad);
  }  

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio MQTT                         */
/*********************************************/
boolean miMQTTClass::parseaConfiguracionMQTT(String contenido)
  {  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  //json.printTo(Serial);
  if (json.success()) 
    {
    Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
    IPBroker.fromString((const char *)json["IPBroker"]);
    ID_MQTT=((const char *)json["ID_MQTT"]);
    puertoBroker = atoi(json["puerto"]); 
    usuarioMQTT=((const char *)json["usuarioMQTT"]);
    passwordMQTT=((const char *)json["passwordMQTT"]);
    topicRoot=((const char *)json["topicRoot"]);
    publicarEntradas=atoi(json["publicarEntradas"]); 
    publicarSalidas=atoi(json["publicarSalidas"]); 
    Serial.printf("Configuracion leida:\nID MQTT: %s\nIP broker: %s\nIP Puerto del broker: %i\nUsuario: %s\nPassword: %s\nTopic root: %s\nPublicar entradas: %i\nPublicar salidas: %i\n",ID_MQTT.c_str(),IPBroker.toString().c_str(),puertoBroker,usuarioMQTT.c_str(),passwordMQTT.c_str(),topicRoot.c_str(),publicarEntradas,publicarSalidas);
//************************************************************************************************
    return true;
    }
  return false;
  }

/***********************************************Funciones de gestion de mensajes MQTT**************************************************************/
/***************************************************/
/*    Genera el JSON de respuesta al Ping MQTT     */
/***************************************************/
String miMQTTClass::generaJSONPing(boolean debug)  
  {
  String cad="";

  cad += "{";
  cad += "\"myIP\": \"" + RedWifi.getIP(false) + "\",";
  cad += "\"ID_MQTT\": \"" + ID_MQTT + "\",";
  cad += "\"IPBbroker\": \"" + IPBroker.toString() + "\",";
  cad += "\"IPPuertoBroker\":" + String(puertoBroker) + "";
  cad += "}";

  if (debug) Serial.printf("Respuesta al ping MQTT: \n%s\n",cad.c_str());
  return cad;
  }

/***************************************************/
/* Funcion que gestiona la respuesta al ping MQTT  */
/***************************************************/
void miMQTTClass::respondePingMQTT(char* topic, byte* payload, unsigned int length)
  {  
  char mensaje[length];    

  Serial.printf("Recibido mensaje Ping:\ntopic: %s\npayload: %s\nlength: %i\n",topic,payload,length);
  
  //copio el payload en la cadena mensaje
  for(int8_t i=0;i<length;i++) mensaje[i]=payload[i];
  mensaje[length]=0;//acabo la cadena

  /**********************Leo el JSON***********************/
  const size_t bufferSize = JSON_OBJECT_SIZE(3) + 50;
  DynamicJsonBuffer jsonBuffer(bufferSize);     
  JsonObject& root = jsonBuffer.parseObject(mensaje);
  if (!root.success()) 
    {
    Serial.println("No se pudo parsear el JSON");
    return; //si el mensaje es incorrecto sale  
    }

  //Si tiene IP se pregunta por un elemento en concreto. Compruebo si soy yo.
  if (root.containsKey("IP")) 
    {
    if (String(root["IP"].as<char*>())!=RedWifi.getIP(false)) return;
    }

  //SI no tenia IP o si tenia la mia, respondo
  String T=TOPIC_PING_RESPUESTA;
  String P= generaJSONPing(false).c_str();
  Serial.printf("Topic: %s\nPayload: %s\n",T.c_str(),P.c_str());
  Serial.printf("Resultado: %i\n", clienteMQTT.publish(T.c_str(),P.c_str()));   
  /**********************Fin JSON***********************/    
  }

/***************************************************/
/* Funcion que recibe el mensaje cuando se publica */
/* en el bus un topic al que esta subscrito        */
/***************************************************/
void callbackMQTT(char* topic, byte* payload, unsigned int length)
  {
  //char mensaje[length+1];  

  //Serial.printf("Entrando en callback: \n Topic: %s\nPayload %s\nLongitud %i\n", topic, payload, length);
  
  /**********compruebo el topic*****************/
  //Sirve para solo atender a los topic suscritos. Si se suscribe a otro habira que gestionarlo aqui
  String cad=String(topic);  //copio el topic a la cadena cad

  //topics descartados
  if(cad==String(miMQTT.gettopicRoot() + "/" + miMQTT.getID_MQTT() + "/keepalive")) return;
  if(cad==String(miMQTT.gettopicRoot() + "/" + miMQTT.getID_MQTT() + "/entradas")) return;
  if(cad==String(miMQTT.gettopicRoot() + "/" + miMQTT.getID_MQTT() + "/salidas")) return;  

  if(cad.equalsIgnoreCase(TOPIC_PING)) miMQTT.respondePingMQTT(topic,payload,length);
  else if(cad.substring(0,String(miMQTT.gettopicRoot() + "/" + miMQTT.getID_MQTT()).length())!=String(miMQTT.gettopicRoot() + "/" + miMQTT.getID_MQTT())) //no deberia, solo se suscribe a los suyos
    {
    Serial.printf("Valor de String(topicRoot + ID_MQTT).length()\n topicRoot: #%s#\nID_MQTT: #%s#\nlongitud: %i\n",miMQTT.gettopicRoot().c_str(),miMQTT.getID_MQTT().c_str(),String(miMQTT.gettopicRoot() + miMQTT.getID_MQTT()).length());
    Serial.printf("Subcadena cad.substring(0,String(topicRoot + ID_MQTT).length()): %s\n",cad.substring(0,String(miMQTT.gettopicRoot() + miMQTT.getID_MQTT()).length()).c_str());
  

    Serial.printf("topic no reconocido: \ntopic: %s\nroot: %s\n", cad.c_str(),cad.substring(0,cad.indexOf("/")).c_str());  
    return;
    }  
  else//topic correcto
    {  
    //No espero nada...
    }
  }

/********************************************/
/* Funcion que gestiona la conexion al bus  */
/* MQTT del broker                          */
/********************************************/
boolean miMQTTClass::conectaMQTT(void)  
  {
  int8_t intentos=0;

  if(IPBroker==IPAddress(0,0,0,0)) return (false);
    
  while (!clienteMQTT.connected()) 
    {    
    if(debugGlobal) Serial.println("No conectado, intentando conectar.");
  
    // Attempt to connect
    //boolean connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage, boolean cleanSession);
    if (clienteMQTT.connect(ID_MQTT.c_str(), usuarioMQTT.c_str(), passwordMQTT.c_str(), (topicRoot+"/"+String(WILL_TOPIC)).c_str(), WILL_QOS, WILL_RETAIN, ("¡"+ID_MQTT+" caido!").c_str(), CLEAN_SESSION))
      {
      if(debugGlobal) Serial.println("conectado");
      //Inicio la subscripcion al topic de todolo que empiece por <topicRoot>/<ID_MQTT>/<WILDCARD_ALL> boolean subscribe(const char* topic);
      String topic = topicRoot + "/" + ID_MQTT + "/" + WILDCARD_ALL; 
      if (clienteMQTT.subscribe(topic.c_str())) Serial.printf("-->Subscrito al topic %s\n", topic.c_str());
      else Serial.printf("Error al subscribirse al topic %s\n", topic.c_str());       

      //Suscripcion al topic de ping
      topic=TOPIC_PING;
      if (clienteMQTT.subscribe(topic.c_str())) Serial.printf("-->Subscrito al topic %s\n", topic.c_str());
      else Serial.printf("Error al subscribirse al topic %s\n", topic.c_str());

      return(true);
      }

    if(intentos++>3) return (false);
    
    if(debugGlobal) Serial.printf("Error al conectar al broker. Estado: %s\n",stateTexto().c_str());
    delay(500);      
    }
  return true;
  }

/********************************************/
/* Funcion que envia un mensaje al bus      */
/* MQTT del broker                          */
/* Eliminado limite del buffer de envio     */
/********************************************/
boolean miMQTTClass::enviarMQTT(String topic, String payload)
  {
  //si no esta conectado, conecto
  if (!clienteMQTT.connected()) conectaMQTT();

  //si y esta conectado envio, sino salgo con error
  if (clienteMQTT.connected()) 
    {
    //String topicCompleto=topicRoot+"/"+ID_MQTT+"/"+topic;  
    String topicCompleto;
    topicCompleto=topicRoot;
    if(!topic.startsWith("/")) topicCompleto += "/"; 
    topicCompleto += topic;  

    Serial.printf("Enviando:\ntopic:  %s | payload: %s\n",topicCompleto.c_str(),payload.c_str());
  
    if(clienteMQTT.beginPublish(topicCompleto.c_str(), payload.length(), false))//boolean beginPublish(const char* topic, unsigned int plength, boolean retained)
      {
      for(uint8_t i=0;i<payload.length();i++) clienteMQTT.write((uint8_t)payload.charAt(i));//virtual size_t write(uint8_t);
      return(clienteMQTT.endPublish()); //int endPublish();
      }
    return (false);  
    }
  else return (false);
  }

/********************************************/
/* Funcion que revisa el estado del bus y   */
/* si se ha recibido un mensaje             */
/********************************************/
void miMQTTClass::atiendeMQTT(boolean debug)
  {  
  clienteMQTT.loop();
  }  

/*********************************************/
/* Funcion que gestiona la publicacion en el */
/* bus de los mensajes de entradas y salidas */
/*********************************************/
void miMQTTClass::enviaDatos(boolean debug)
  {
 
  }

/******************************* UTILIDADES *************************************/
/********************************************/
/* Funcion que devuleve el estado           */
/* de conexion MQTT al bus                  */
/********************************************/
String miMQTTClass::stateTexto(void)  
  {
  int r = clienteMQTT.state();

  String cad=String(r) + " : ";
  
  switch (r)
    {
    case -4:
      cad += "MQTT_CONNECTION_TIMEOUT";
      break;
    case -3:
      cad += "MQTT_CONNECTION_LOST";
      break;
    case -2:
      cad += "MQTT_CONNECT_FAILED";
      break;
    case -1:
      cad += "MQTT_DISCONNECTED";
      break;
    case  0:
      cad += "MQTT_CONNECTED";
      break;
    case  1:
      cad += "MQTT_CONNECT_BAD_PROTOCOL";
      break;
    case  2:
      cad += "MQTT_CONNECT_BAD_CLIENT_ID";
      break;
    case  3:
      cad += "MQTT_CONNECT_UNAVAILABLE";
      break;
    case  4:
      cad += "MQTT_CONNECT_BAD_CREDENTIALS";
      break;     
    case  5:
      cad += "MQTT_CONNECT_UNAUTHORIZED";
      break;
    default:
      cad += "????";
    }
      
  return (cad);
  }
  
int8_t miMQTTClass::getPublicarEntradas(void){return publicarEntradas;};
void miMQTTClass::setPublicarEntradas(int8_t pubEnt){publicarEntradas=pubEnt;};

int8_t miMQTTClass::getPublicarSalidas(void){return publicarSalidas;};
void miMQTTClass::setPublicarSalidas(int8_t pubSal){publicarEntradas=pubSal;};
