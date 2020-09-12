/*****************************************/
/*                                       */
/*  Control de entradas digitales        */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Entradas.h>
/***************************** Includes *****************************/

/************************************** Funciones de entrada ****************************************/
void entrada::inicializaEntrada(void)
  {
  configurada=NO_CONFIGURADO ;//la inicializo a no configurada
  nombre="No configurado";
  estado=0;    
  tipo="INPUT";
  pin=NO_CONFIGURADO;      
  }
      
void entrada::inicializaEntrada(int8_t conf, String nom, int8_t est, String tip, int8_t p)
  {
  configurada=conf ;//la inicializo a no configurada
  nombre=nom;
  estado=est;    
  tipo=tip;
  pin=p;      
  }  
/************************************** Funciones de entrada ****************************************/

/************************************** Funciones de configuracion ****************************************/
/* Inicializa los valores de los registros de*/
/* las entradas y recupera la configuracion  */
/*********************************************/
void EntradasClass::inicializaEntradas(void)
  {  
  //inicializo la parte logica
  for(int8_t i=0;i<MAX_ENTRADAS;i++) {entradas[i].inicializaEntrada();}
         
  //leo la configuracion del fichero
  if(!recuperaDatosEntradas(debugGlobal)) Serial.println("Configuracion de entradas por defecto");
  else
    {  
    for(int8_t i=0;i<MAX_ENTRADAS;i++)
      {
      if(entradas[i].getTipo()!="INPUT_PULLUP") pinMode(entradas[i].getPin(), INPUT_PULLUP);
      else if(entradas[i].getTipo()!="INPUT_PULLDOWN") pinMode(entradas[i].getPin(), INPUT_PULLDOWN);
      else pinMode(entradas[i].getPin(), INPUT); //PULLUP
      }

    //Entradas configuradas
    for(int i=0;i<MAX_ENTRADAS;i++) if(entradas[i].getConfigurada()==CONFIGURADO) Serial.printf("Nombre entrada[%i]=%s | pin entrada: %i\n",i,entradas[i].getNombre().c_str(),entradas[i].getPin());
    }  
  }

int EntradasClass::recuperaDatosEntradas(boolean debug)
  {
  String cad="";

  if (debug) Serial.println("Recupero configuracion de archivo...");
  
  if(!SistemaFicheros.leeFichero(ENTRADAS_CONFIG_FILE, cad)) 
    {
    //Confgiguracion por defecto
    Serial.printf("No existe fichero de configuracion de Entradas\n");    
    return false;
    }      
    
  return parseaConfiguracionEntradas(cad);
  }

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de los reles                 */
/*********************************************/
boolean EntradasClass::parseaConfiguracionEntradas(String contenido)
  { 
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  
  json.printTo(Serial);
  if (!json.success()) return false;
        
  Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
    JsonArray& Entradas = json["Entradas"];

  int8_t max;
  max=(Entradas.size()<MAX_ENTRADAS?Entradas.size():MAX_ENTRADAS);
  for(int8_t i=0;i<max;i++)
    { 
    entradas[i].inicializaEntrada(CONFIGURADO, String((const char *)Entradas[i]["nombre"]), 0, String((const char *)Entradas[i]["tipo"]), atoi(Entradas[i]["GPIO"]));
    /* 
    entradas[i].setConfigurada(CONFIGURADO) ;//la inicializo a no configurada
    entradas[i].setNombre(String((const char *)Entradas[i]["nombre"]));
    entradas[i].setEstado(0);    
    entradas[i].setTipo(String((const char *)Entradas[i]["tipo"]));
    entradas[i].setPin(atoi(Entradas[i]["GPIO"]));
    */
    }

  Serial.printf("Entradas:\n"); 
  for(int8_t i=0;i<MAX_ENTRADAS;i++) Serial.printf("%02i: %s| pin: %i| configurado= %i| tipo=%s\n",i,entradas[i].getNombre().c_str(),entradas[i].getPin(),entradas[i].getConfigurada(),entradas[i].getTipo().c_str());
//************************************************************************************************
  return true; 
  }
/**********************************************************Fin configuracion******************************************************************/  

/**********************************************************ENTRADAS******************************************************************/  
/*************************************************/
/*                                               */
/*       Lee el estado de las entradas           */
/*                                               */
/*************************************************/
void EntradasClass::consultaEntradas(bool debug)
  {
  //Actualizo las entradas  
  for(int8_t i=0;i<MAX_ENTRADAS;i++)
    {
    if(entradas[i].getConfigurada()==CONFIGURADO) 
      {
      entradas[i].setEstado(digitalRead(entradas[i].getPin())); //si la entrada esta configurada
      //Serial.printf("Entrada %i en pin %i leido %i\n",i,entradas[i].pin,entradas[i].estado);
      }
    }   
  }

/*************************************************/
/*                                               */
/*   Devuelve el estado 0|1 de la entrada        */
/*   indicada en id                              */
/*                                               */
/*************************************************/
int8_t EntradasClass::estadoEntrada(int8_t id)
  {
  if(id <0 || id>=MAX_ENTRADAS) return NO_CONFIGURADO; //Entrada fuera de rango
  
  if(entradas[id].getConfigurada()==CONFIGURADO) return (entradas[id].getEstado()); //si la entrada esta configurada
  else return NO_CONFIGURADO;
 }

/*************************************************/
/*                                               */
/*  Devuelve el nombre de la entradarele con el  */
/*  id especificado                              */
/*                                               */
/*************************************************/
String EntradasClass::nombreEntrada(int8_t id)
  { 
  if(id <0 || id>=MAX_ENTRADAS) return "ERROR"; //Entrada fuera de rango    
  return entradas[id].getNombre();
  } 

/*************************************************/
/*                                               */
/*  Devuelve el tipo de la entrada con el        */
/*  id especificado                              */
/*                                               */
/*************************************************/
String EntradasClass::tipoEntrada(int8_t id)
  { 
  if(id <0 || id>=MAX_ENTRADAS) return "ERROR"; //Entrada fuera de rango    
  return entradas[id].getTipo();
  }

/*************************************************/
/*                                               */
/*   Devuelve el pin asociado a la entrada       */
/*                                               */
/*************************************************/
int8_t EntradasClass::pinEntrada(int8_t id)
  {
  if(id <0 || id>=MAX_ENTRADAS) return NO_CONFIGURADO; //Entrada fuera de rango
  
  if(entradas[id].getConfigurada()==CONFIGURADO) return (entradas[id].getPin()); //si la entrada esta configurada
  else return NO_CONFIGURADO;
 }

/*************************************************/
/*                                               */
/*  Devuelve el numero de entradas configuradas  */
/*                                               */
/*************************************************/ 
int EntradasClass::entradasConfiguradas(void)
  {
  int resultado=0;
  
  for(int8_t i=0;i<MAX_ENTRADAS;i++)
    {
    if(entradas[i].getConfigurada()==CONFIGURADO) resultado++;
    }
  return resultado;
  }

/*************************************************/
/*                                               */
/*    Devuelve si la entrada esta configrada     */
/*                                               */
/*************************************************/
int8_t EntradasClass::entradaConfigurada(int8_t id)
  {
  if(id <0 || id>=MAX_ENTRADAS) return NO_CONFIGURADO; //Entrada fuera de rango    
  return entradas[id].getConfigurada();
 }
/********************************************* Fin entradas *******************************************************************/
  
/****************************************** Funciones de estado ***************************************************************/
/***********************************************************/
/*                                                         */
/*   Devuelve el estado de las entradas en formato json    */
/*   devuelve un json con el formato:                      */
/* {
    "Entradas": [  
      {"id":  "0", "nombre": "P. abierta", "valor": "1" },
      {"id":  "1", "nombre": "P. cerrada", "valor": "0" }
    ]
  }
                                                           */
/***********************************************************/   
String EntradasClass::generaJsonEstadoEntradas(void)
  {
  String salida="";

  const size_t bufferSize = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(3);
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  JsonObject& root = jsonBuffer.createObject();
  
  JsonArray& Entradas = root.createNestedArray("Entradas");
  for(int8_t id=0;id<MAX_ENTRADAS;id++)
    {
    if(entradas[id].getConfigurada()==CONFIGURADO)
      {
      JsonObject& Entradas_0 = Entradas.createNestedObject();
      Entradas_0["id"] = id;
      Entradas_0["nombre"] = entradas[id].getNombre();
      Entradas_0["valor"] = entradas[id].getEstado();
      }
    }

  root.printTo(salida);
  return salida;  
  }

EntradasClass Entradas;