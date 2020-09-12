/*****************************************/
/*                                       */
/*    Control de la maquina de estados   */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <MaqEstados.h>
/***************************** Includes *****************************/

/************************************** Funciones de salida ****************************************/
/*********************************************/
/* Inicializa los valores de la maquina      */
/* de estados                                */
/*********************************************/
void maqEstadosClass::inicializaMaquinaEstados(void)
    {      
    //Estado de la maquina
    debugMaquinaEstados=false;
    estadoActual=ESTADO_INICIAL;
    for(uint8_t i=0;i<MAX_ENTRADAS;i++) entradasActual[i]=NO_CONFIGURADO;
    for(uint8_t i=0;i<MAX_SALIDAS;i++) salidasActual[i]=NO_CONFIGURADO;
    
    //Variables de configuracion
    numeroEstados=0;
    numeroTransiciones=0;
    numeroEntradas=0;
    numeroSalidas=0;

    //Entradas
    for(uint8_t i=0;i<MAX_ENTRADAS;i++) mapeoEntradas[i]=NO_CONFIGURADO;
        
    //Salidas
    for(uint8_t i=0;i<MAX_SALIDAS;i++) mapeoSalidas[i]=NO_CONFIGURADO;
        
    //Estados
    for(int8_t i=0;i<MAX_ESTADOS;i++)
        {
        //inicializo la parte logica
        estados[i].id=i;
        estados[i].nombre="Estado_" + String(i);
        for(uint8_t j=0;j<MAX_SALIDAS;j++) estados[i].valorSalidas[j]=NO_CONFIGURADO;
        }

    //Transiciones
    for(int8_t i=0;i<MAX_TRANSICIONES;i++)
        {
        //inicializo la parte logica
        transiciones[i].estadoInicial=NO_CONFIGURADO;
        transiciones[i].estadoFinal=NO_CONFIGURADO;
        for(uint8_t j=0;j<MAX_ENTRADAS;j++) transiciones[i].valorEntradas[j]=NO_CONFIGURADO;
        }
        
    //leo la configuracion del fichero
    if(!recuperaDatosMaquinaEstados(debugGlobal)) Serial.printf("Configuracion de la maquina de estados por defecto.\n");
    else
        { 
        
        }  
    }

/************************************************/
/* Lee el fichero de configuracion de la        */
/* maquina de estados o genera conf por defecto */
/************************************************/
boolean maqEstadosClass::recuperaDatosMaquinaEstados(int debug)
  {
  String cad="";

  if (debug) Serial.printf("Recupero configuracion de archivo...\n");

  if(!SistemaFicheros.leeFichero(MAQUINAESTADOS_CONFIG_FILE, cad)) 
    {
    //Confgiguracion por defecto
    Serial.printf("No existe fichero de configuracion de la maquina de estados\n");    
    cad="{\"Estados\":[],\"Transiciones\":[] }";
    //salvo la config por defecto
    //if(salvaFicheroConfig(MAQUINAESTADOS_CONFIG_FILE, MAQUINAESTADOS_CONFIG_BAK_FILE, cad)) Serial.printf("Fichero de configuracion de la maquina de estados creado por defecto\n");
    }      
  return parseaConfiguracionMaqEstados(cad);
  }

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de la maquina de estados     */
/*                                           */
/*********************************************/
boolean maqEstadosClass::parseaConfiguracionMaqEstados(String contenido)
  {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());

  String salida;
  json.printTo(salida);//pinto el json que he leido
  Serial.printf("json creado:\n#%s#\n",salida.c_str());
  
  if (!json.success()) return false;

  Serial.printf("\nparsed json\n");
//******************************Parte especifica del json a leer********************************
  if(!json.containsKey("Estados"))  return false; 
  if(!json.containsKey("Transiciones"))  return false; 
  if(!json.containsKey("Entradas"))  return false; 
  if(!json.containsKey("Salidas"))  return false; 

  /********************Entradas******************************/
  JsonArray& E = json["Entradas"];
  numeroEntradas=(E.size()<MAX_ENTRADAS?E.size():MAX_ENTRADAS);
  for(uint8_t i=0;i<numeroEntradas;i++) mapeoEntradas[i]=E[i];   

  Serial.printf("Entradas asociadas a la maquina de estados: %i\n",numeroEntradas);
  for(uint8_t i=0;i<numeroEntradas;i++) Serial.printf("orden %i | id general %i\n", i,mapeoEntradas[i]);
  
  /********************Salidas******************************/
  JsonArray& S = json["Salidas"];
  numeroSalidas=(S.size()<MAX_SALIDAS?S.size():MAX_SALIDAS);
  for(uint8_t i=0;i<numeroSalidas;i++) mapeoSalidas[i]=S[i];   

  Serial.printf("Salidas asociadas a la maquina de estados: %i\n",numeroSalidas);
  for(uint8_t i=0;i<numeroSalidas;i++) Serial.printf("orden %i | id general %i\n", i,mapeoSalidas[i]);
  
  /********************Estados******************************/
  JsonArray& Estados = json["Estados"];

  numeroEstados=(Estados.size()<MAX_ESTADOS?Estados.size():MAX_ESTADOS);
  for(int8_t i=0;i<numeroEstados;i++)
    { 
    JsonObject& est = Estados[i];   
    estados[i].id=est["id"]; 
    estados[i].nombre=String((const char *)est["nombre"]);

    JsonArray& Salidas = est["salidas"];
    int8_t num_salidas;
    num_salidas=(Salidas.size()<MAX_SALIDAS?Salidas.size():MAX_SALIDAS);
    if(num_salidas!=numeroSalidas) 
      {
      Serial.printf("Numero de salidas incorrecto en estado %i. definidas %i, esperadas %i\n",i,num_salidas,numeroSalidas);
      return false;
      }
    //////EL ID NO VALE PARA NADA, LA REFERENCIA ES POSICIONAL. QUITAR ID/////////
    for(int8_t s=0;s<num_salidas;s++) estados[i].valorSalidas[s]=Salidas.get<int>(s);//Salidas[s]["valor"];
    }

  Serial.printf("*************************\nEstados:\n"); 
  Serial.printf("Se han definido %i estados\n",numeroEstados);
  for(int8_t i=0;i<numeroEstados;i++) 
    {
    Serial.printf("%01i: id= %i| nombre: %s\n",i,estados[i].id,estados[i].nombre.c_str());
    Serial.printf("salidas:\n");
    for(int8_t s=0;s<numeroSalidas;s++) 
      {
      Serial.printf("salida[%02i]: valor: %i\n",s,estados[i].valorSalidas[s]);
      }
    }
  Serial.printf("*************************\n");  
  
  /********************Transiciones******************************/
  JsonArray& Transiciones = json["Transiciones"];

  numeroTransiciones=(Transiciones.size()<MAX_TRANSICIONES?Transiciones.size():MAX_TRANSICIONES);
  for(int8_t i=0;i<numeroTransiciones;i++)
    { 
    JsonObject& trans = Transiciones[i];   
    transiciones[i].estadoInicial=trans["inicial"]; 
    transiciones[i].estadoFinal=trans["final"];
    
    JsonArray& Entradas = trans["entradas"];   
    int8_t num_entradas;
    num_entradas=(Entradas.size()<MAX_ENTRADAS?Entradas.size():MAX_ENTRADAS);
    if(num_entradas!=numeroEntradas) 
      {
      Serial.printf("Numero de entradas incorrecto en estado %i. definidas %i, esperadas %i\n",i,num_entradas,numeroEntradas);
      return false;
      }
    
    for(int8_t e=0;e<num_entradas;e++) transiciones[i].valorEntradas[e]=Entradas.get<int>(e);//atoi(Entradas[e]["valor"]);//Puede ser -1, significa que no importa el valor
    }

  Serial.printf("*************************\nTransiciones:\n"); 
  Serial.printf("Se han definido %i transiciones\n",numeroTransiciones);
  for(int8_t i=0;i<numeroTransiciones;i++) 
    {
    Serial.printf("%01i: estado inicial= %i| estado final: %i\n",i,transiciones[i].estadoInicial,transiciones[i].estadoFinal);
    Serial.printf("entradas:\n");
    for(int8_t e=0;e<numeroEntradas;e++) 
      {
      Serial.printf("entradas[%02i]: valor: %i\n",e,transiciones[i].valorEntradas[e]);
      }
    }
  Serial.printf("*************************\n");  
//************************************************************************************************
  return true; 
  }
/**********************************************************Fin configuracion******************************************************************/  

/*********************************************************MAQUINA DE ESTADOS******************************************************************/    
/****************************************************/
/* Analiza el estado de la maquina y evoluciona     */
/* los estados y las salidas asociadas              */
/****************************************************/
void maqEstadosClass::actualizaMaquinaEstados(int debug)
  {
  boolean localDebug=debug || debugMaquinaEstados;
    
  //Actualizo el vaor de las entradas
  for(uint8_t i=0;i<numeroEntradas;i++) entradasActual[i]=Entradas.estadoEntrada(mapeoEntradas[i]);

  if(localDebug) 
    {
    Serial.printf("Estado inicial: (%i) %s\n",estadoActual,estados[estadoActual].nombre.c_str());
    Serial.printf("Estado de las entradas:\n");
    for(uint8_t i=0;i<numeroEntradas;i++) Serial.printf("Entrada %i (dispositivo %i)=> valor %i\n",i, mapeoEntradas[i],entradasActual[i]);
    }
    
  //busco en las transiciones a que estado debe evolucionar la maquina
  estadoActual=mueveMaquina(estadoActual, entradasActual, localDebug);

  //Actualizo las salidas segun el estado actual
  if(actualizaSalidasMaquinaEstados(estadoActual)!=1) Serial.printf("Error al actualizar las salidas\n");

  if(localDebug) Serial.printf("Estado actual: (%i) %s\n",estadoActual,estados[estadoActual].nombre.c_str());
  }

/****************************************************/
/* busco en las transiciones a que estado debe      */
/* evolucionar la maquina                           */
/****************************************************/
uint8_t maqEstadosClass::mueveMaquina(uint8_t estado, int8_t entradasActual[], boolean debug)
  {
  for(uint8_t regla=0;regla<numeroTransiciones;regla++) //las reglas se evaluan por orden
    {
    if(transiciones[regla].estadoInicial==estado)//Solo analizo las que tienen como estado inicial el indicado
      {
      if(debug) Serial.printf("Revisando regla %i\n",regla);
  
      boolean coinciden=true;  
      for(uint8_t entrada=0;entrada<numeroEntradas;entrada++) 
        {
        if (transiciones[regla].valorEntradas[entrada]!=NO_CONFIGURADO) coinciden=coinciden &&(entradasActual[entrada]==transiciones[regla].valorEntradas[entrada]);
        if(debug) Serial.printf("Revisando entradas %i de regla %i (valor actual: %i vs valor regla: %i). Resultado %i\n",entrada,regla,entradasActual[entrada],transiciones[regla].valorEntradas[entrada],coinciden);
        }

      if(coinciden) return transiciones[regla].estadoFinal;
      }
    }
  return ESTADO_ERROR;  //Si no coincide ninguna regla, pasa a estado error
  }
  
/****************************************************/
/* Actualizo las salidas segun el estado actual     */
/****************************************************/
int8_t maqEstadosClass::actualizaSalidasMaquinaEstados(uint8_t estado)
  {
  int8_t retorno=1; //si todo va bien salidaMaquinaEstados devuelve 1, si hay error -1 
  //Serial.printf("Estado: %s\n",estados[estado].nombre);
  for(uint8_t i=0;i<numeroSalidas;i++) 
    {
    //if(salidaMaquinaEstados(mapeoSalidas[i], estados[estado].valorSalidas[i])==NO_CONFIGURADO) retorno=0;
    }

  return retorno;
  }


