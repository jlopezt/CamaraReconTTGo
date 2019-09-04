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
#ifndef _MQTT_
#define _MQTT_

//#define MQTT_KEEPALIVE 60
#define CLEAN_SESSION true

//definicion de los comodines del MQTT
#define WILDCARD_ALL      "#"
#define WILDCARD_ONELEVEL "+"

//definicion de constantes para WILL
#define WILL_TOPIC  "will"
#define WILL_QOS    1
#define WILL_RETAIN false
#define WILL_MSG    String("¡" + miMQTT.getID_MQTT() + " caido!").c_str()
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
/***************************** Includes *****************************/

//funcion de callback para gestion de mensajes. No puede ser miembro de la clase
void callbackMQTT(char* topic, byte* payload, unsigned int length);

class miMQTTClass
{
private:
//Definicion de variables globales
IPAddress IPBroker; //IP del bus MQTT
uint16_t puertoBroker; //Puerto del bus MQTT
String usuarioMQTT; //usuario par ala conxion al broker
String passwordMQTT; //password parala conexion al broker
String topicRoot; //raiz del topic a publicar. Util para separar mensajes de produccion y prepropduccion
String ID_MQTT; //ID del modulo en su conexion al broker
String topicKeepAlive; //Topic para los mensajes de keepAlive
int8_t enviarKeepAlive; //flag para determinar si se envia o no keepalive MQTT
int8_t publicarEntradas; //Flag para determinar si se envia el json con los valores de las entradas
int8_t publicarSalidas; //Flag para determinar si se envia el json con los valores de las salidas

public:
WiFiClient espClient;
void inicializaMQTT(void);
boolean recuperaDatosMQTT(boolean debug);
boolean parseaConfiguracionMQTT(String contenido);
boolean conectaMQTT(void);
boolean enviarMQTT(String topic, String payload);
void atiendeMQTT(boolean debug);
void enviaDatos(boolean debug);
String stateTexto(void);

String gettopicRoot(void) {return topicRoot;};
String getID_MQTT(void) {return ID_MQTT;};
IPAddress getIPBroker(void) {return IPBroker;};
uint16_t getPuertoBroker(void) {return puertoBroker;};
String getUsuarioMQTT(void) {return usuarioMQTT;};
String getPasswordMQTT(void) {return passwordMQTT;};

int8_t getEnviarKeepAlive(void);
void setEnviarKeepALive(int8_t EnviarKA);

int8_t getPublicarEntradas(void);
void setPublicarEntradas(int8_t pubEnt);

int8_t getPublicarSalidas(void);
void setPublicarSalidas(int8_t pubSal);
};

extern miMQTTClass miMQTT;

#endif