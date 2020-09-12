/*****************************************/
/*                                       */
/*     Control de salidas a rele         */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Salidas.h>
/***************************** Includes *****************************/

/************************************** Funciones de salida ****************************************/
void salida::inicializaSalida(void)
  {
  configurado=NO_CONFIGURADO ;//la inicializo a no configurada
  nombre="No configurado";
  estado=0;    
  pin=NO_CONFIGURADO;
  pinLed=NO_CONFIGURADO;   
  secuenciador=NO_CONFIGURADO;
  finPulso=0;
  inicio=0;  
  }

void salida::inicializaSalida(int8_t config, String nom, int8_t est, int8_t p, int8_t pLed, int8_t sec, unsigned long finP,int8_t ini)
  {
  configurado=config;//la inicializo a configurada
  nombre=nom;
  estado=est;    
  pin=p;
  pinLed=pLed;      
  secuenciador=sec;
  finPulso=finP;
  inicio=ini;
  }  
/************************************** Funciones de salida ****************************************/

/************************************** Funciones de configuracion ****************************************/
/*********************************************/
/* Inicializa los valores de los registros de*/
/* las salidas y recupera la configuracion   */
/*********************************************/
void SalidasClass::inicializaSalidas()
  {  
  //inicializo la parte logica
  for(int8_t i=0;i<MAX_SALIDAS;i++) {reles[i].inicializaSalida();}
         
  //leo la configuracion del fichero
  if(!recuperaDatosSalidas(debugGlobal)) Serial.println("Configuracion de los reles por defecto");
  else
    {  
    for(int8_t i=0;i<MAX_SALIDAS;i++)
      {      
      pinMode(reles[i].getPin(), OUTPUT); //es salida
      pinMode(reles[i].getPinLed(), OUTPUT); //es salida
      
      //parte logica
      reles[i].setEstado(reles[i].getInicio());
      //parte fisica
      if(reles[i].getInicio()==1)
        {
        digitalWrite(reles[i].getPin(), cacharro.getNivelActivo());  //lo inicializo a apagado
        digitalWrite(reles[i].getPinLed(), HIGH);  //lo inicializo encendido
        }
      else
        {
        digitalWrite(reles[i].getPin(), !cacharro.getNivelActivo());  //lo inicializo a apagado 
        digitalWrite(reles[i].getPinLed(), LOW);  //lo inicializo a apagado
        }
      }

    //Salidas configuradas
    for(int i=0;i<MAX_SALIDAS;i++) if(reles[i].getConfigurado()==CONFIGURADO) Serial.printf("Nombre rele[%i]=%s | pin rele: %i | pin Led: %i | inicio: %i\n",i,reles[i].getNombre().c_str(),reles[i].getPin(),reles[i].getPinLed(),reles[i].getInicio());
    }    
  }

/*********************************************/
/* Lee el fichero de configuracion de las    */
/* entradas o genera conf por defecto        */
/*********************************************/
boolean SalidasClass::recuperaDatosSalidas(boolean debug)
  {
  String cad="";

  if (debug) Serial.println("Recupero configuracion de archivo...");
  
  if(!SistemaFicheros.leeFichero(SALIDAS_CONFIG_FILE, cad)) 
    {
    //Confgiguracion por defecto
    Serial.printf("No existe fichero de configuracion de Salidas\n");    
    return false;
    }      
    
  return parseaConfiguracionSalidas(cad);
  }

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de los reles                 */
/*********************************************/
boolean SalidasClass::parseaConfiguracionSalidas(String contenido)
  { 
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  
  json.printTo(Serial);
  if (!json.success()) return false;
        
  Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
  JsonArray& Salidas = json["Salidas"];

  int8_t max;
  max=(Salidas.size()<MAX_SALIDAS?Salidas.size():MAX_SALIDAS);
  for(int8_t i=0;i<max;i++)
    { 
    //Si de inicio debe estar activado o desactivado
    int8_t est;
    if(String((const char *)Salidas[i]["inicio"])=="on") est=1;
    else est=0;

    reles[i].inicializaSalida(CONFIGURADO, String((const char *)Salidas[i]["nombre"]), est, atoi(Salidas[i]["GPIO"]),atoi(Salidas[i]["GPIOLED"]), NO_CONFIGURADO, 0, 0);    
    }
    
  Serial.printf("Salidas:\n"); 
  for(int8_t i=0;i<MAX_SALIDAS;i++) Serial.printf("%02i: %s| pin: %i| pinLed: %i| configurado= %i\n",i,reles[i].getNombre().c_str(),reles[i].getPin(),reles[i].getPinLed(),reles[i].getConfigurado()); 
//************************************************************************************************
  return true; 
  }
/**********************************************************Fin configuracion******************************************************************/  

/**********************************************************SALIDAS******************************************************************/    
/*************************************************/
/*Logica de los reles:                           */
/*Si esta activo para ese intervalo de tiempo(1) */
/*Si esta por debajo de la tMin cierro rele      */
/*si no abro rele                                */
/*************************************************/
void SalidasClass::actualizaSalidas(bool debug)
  {
  for(int8_t id=0;id<MAX_SALIDAS;id++)
    {
    if (reles[id].getConfigurado()==CONFIGURADO && reles[id].getEstado()==2) //esta configurado y pulsando
      {
      if(reles[id].getFinPulso()>ANCHO_PULSO)//el contador de millis no desborda durante el pulso
        {
        if(millis()>=reles[id].getFinPulso()) //El pulso ya ha acabado
          {
          conmutaRele(id,!cacharro.getNivelActivo(),debugGlobal);  
          if(debug) Serial.printf("Fin del pulso. millis()= %lu\n",millis());
          }//del if del fin de pulso
        }//del if de desboda
      else //El contador de millis desbordar durante el pulso
        {
        if(UINT64_MAX-ANCHO_PULSO>millis())//Ya ha desbordado
          {
          if(millis()>=reles[id].getFinPulso()) 
            {
            conmutaRele(id,!cacharro.getNivelActivo(),debugGlobal);
            if(debug) Serial.printf("Fin del pulso. millis()= %lu\n",millis());
            }//del if del fin de pulso
          }//del if ha desbordado ya
        }//else del if de no desborda
      }//del if configurado
    }//del for    
  }//de la funcion

/*************************************************/
/*                                               */
/*  Devuelve el estado  del rele indicado en id  */
/*  puede ser 0 apagado, 1 encendido, 2 pulsando */
/*                                               */
/*************************************************/
int8_t SalidasClass::getEstadoRele(int8_t id)
  {
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO; //Rele fuera de rango
  if(reles[id].getConfigurado()!=CONFIGURADO) return -1; //No configurado
  
  return reles[id].getEstado();
 }

/********************************************************/
/*                                                      */
/*  Devuelve el nombre del rele con el id especificado  */
/*                                                      */
/********************************************************/
String SalidasClass::getNombreRele(int8_t id)
  { 
  if(id <0 || id>=MAX_SALIDAS) return "ERROR"; //Rele fuera de rango    
  return reles[id].getNombre();
  } 

/*************************************************/
/*conmuta el rele indicado en id                 */
/*devuelve 1 si ok, -1 si ko                     */
/*************************************************/
boolean SalidasClass::conmutaRele(int8_t id, boolean estado_final, int debug)
  {
  //validaciones previas
  if(id <0 || id>=MAX_SALIDAS) return false; //Rele fuera de rango
  if(reles[id].getConfigurado()==NO_CONFIGURADO) return false; //El rele no esta configurado
  
  //parte logica
  if(estado_final==cacharro.getNivelActivo()) reles[id].setEstado(1);
  else reles[id].setEstado(0);
  
  //parte fisica
  digitalWrite(reles[id].getPin(), estado_final); //controlo el rele
  //controlo el led asociado
  if(reles[id].getPinLed()!=NO_CONFIGURADO)
    {
    if (cacharro.getNivelActivo()) digitalWrite(reles[id].getPinLed(), estado_final); 
    else digitalWrite(reles[id].getPinLed(), !estado_final); 
    }

  if(debug)
    {
    Serial.printf("id: %i; GPIO: %i; estado: ",(int)id,(int)reles[id].getPin());
    Serial.println(digitalRead(reles[id].getPin()));
    }
    
  return true;
  }

/****************************************************/
/*   Genera un pulso en rele indicado en id         */
/*   devuelve 1 si ok, -1 si ko                     */
/****************************************************/
boolean SalidasClass::pulsoRele(int8_t id)
  {
  //validaciones previas
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO;
      
  //Pongo el rele en nivel Activo  
  if(!conmutaRele(id, cacharro.getNivelActivo(), debugGlobal)) return false; //Si no puede retorna -1

  //cargo el campo con el valor definido para el ancho del pulso
  reles[id].setEstado(2);//estado EN_PULSO
  reles[id].setFinPulso(millis()+ANCHO_PULSO); 

  Serial.printf("Incio de pulso %lu| fin calculado %lu\n",millis(),reles[id].getFinPulso());
  
  return true;  
  }

/********************************************************/
/*                                                      */
/*     Recubre las dos funciones anteriores para        */
/*     actuar sobre un rele                             */
/*                                                      */
/********************************************************/ 
boolean SalidasClass::actuaRele(int8_t id, int8_t estado)
  {
  switch(estado)
    {
    case 0:
      return conmutaRele(id, !cacharro.getNivelActivo(), debugGlobal);
      break;
    case 1:
      return conmutaRele(id, cacharro.getNivelActivo(), debugGlobal);
      break;
    case 2:
      return pulsoRele(id);
      break;      
    default://no deberia pasar nunca!!
      return -1;
    }
  }

/********************************************************/
/*                                                      */
/*     Devuelve si el reles esta configurados           */
/*                                                      */
/********************************************************/ 
int8_t SalidasClass::getReleConfigurado(uint8_t id)
  {
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO;
    
  return reles[id].getConfigurado();
  } 
  

/********************************************************/
/*                                                      */
/*     Devuelve el pin asoiciado al rele                */
/*                                                      */
/********************************************************/ 
int8_t SalidasClass::getPinReleLed(int8_t id)
  {
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO;
    
  return reles[id].getPinLed();
  }   

/********************************************************/
/*                                                      */
/*     Devuelve el pin asoiciado al rele                */
/*                                                      */
/********************************************************/ 
int8_t SalidasClass::getPinRele(int8_t id)
  {
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO;
    
  return reles[id].getPin();
  }   

/********************************************************/
/*                                                      */
/*  Devuelve el modo inicial del pin asociado al rele   */
/*                                                      */
/********************************************************/ 
int8_t SalidasClass::getInicioRele(int8_t id)
  {
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO;
    
  return reles[id].getInicio();
  }   

/********************************************************/
/*                                                      */
/*     Devuelve el numero de reles configurados         */
/*                                                      */
/********************************************************/ 
int SalidasClass::relesConfigurados(void)
  {
  int resultado=0;
  
  for(int8_t i=0;i<MAX_SALIDAS;i++)
    {
    if(reles[i].getConfigurado()==CONFIGURADO) resultado++;
    }
  return resultado;
  } 

/********************************************************/
/*                                                      */
/*     Asocia la salida a un plan de secuenciador       */
/*                                                      */
/********************************************************/ 
boolean SalidasClass::asociarSecuenciador(int8_t id, int8_t plan)
  {
  //validaciones previas
  if(id <0 || id>=MAX_SALIDAS) return false;

  reles[id].setSecuenciador(plan); 
  return true;
  }  

/********************************************************/
/*                                                      */
/*     Devuelve si la salida esta asociada              */
/*     a un plan de secuenciador                        */
/*                                                      */
/********************************************************/ 
int8_t SalidasClass::getAsociadaASecuenciador(int8_t id)
  {
  //validaciones previas
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO;
      
  return reles[id].getSecuenciador();  
  }   
/********************************************************** Fin salidas ******************************************************************/  
  
/****************************************** Funciones de estado ***************************************************************/
/********************************************************/
/*                                                      */
/*   Devuelve el estado de los reles en formato json    */
/*   devuelve un json con el formato:                   */
/* {
    "Salidas": [  
      {"id":  "0", "nombre": "Pulsador", "valor": "1" },
      {"id":  "1", "nombre": "Auxiliar", "valor": "0" }
      ]
   }
                                                        */
/********************************************************/   
String SalidasClass::generaJsonEstadoSalidas(void)
  {
  String salida="";

  const size_t bufferSize = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(3);
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  JsonObject& root = jsonBuffer.createObject();
  
  JsonArray& Salidas = root.createNestedArray("Salidas");
  for(int8_t id=0;id<MAX_SALIDAS;id++)
    {
    if(reles[id].getConfigurado()==CONFIGURADO)
      {
      JsonObject& Salidas_0 = Salidas.createNestedObject();
      Salidas_0["id"] = id;
      Salidas_0["nombre"] = reles[id].getNombre();
      Salidas_0["valor"] = reles[id].getEstado();    
      }
    }
    
  root.printTo(salida);
  return salida;  
  }  
/****************************************** Fin funciones de estado ***************************************************************/

SalidasClass Salidas;