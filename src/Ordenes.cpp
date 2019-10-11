/*
 * ordenes.cpp
 *
 * Interfaz serie para la recepcion de ordenes y su ejecución 
 *
 * Permite la puesta en hora del reloj a traves de comandos enviados por el puesto
 * serie desde el PC.
 *
 * Para actualizar la hora <comado> <valor>; Ejemplo: "hora 3;"
 * Se pueden anidar: "hora 2;minuto 33;"
 *
 */

/***************************** Defines *****************************/
#define LONG_COMANDO 40
#define LONG_PARAMETRO 30

/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
#include <SPIFFS.h>
#include <Ordenes.h>
#include <inttypes.h>
/***************************** Includes *****************************/

//Declaracion de las funciones de comando
//Globales
void func_comando_vacio(int iParametro, char* sParametro, float fParametro); ; //vacio
void func_comando_help(int iParametro, char* sParametro, float fParametro); ; //help
void func_comando_echo(int iParametro, char* sParametro, float fParametro); //echo 
void func_comando_debug(int iParametro, char* sParametro, float fParametro); //debug
void func_comando_restart(int iParametro, char* sParametro, float fParametro); //restart
void func_comando_info(int iParametro, char* sParametro, float fParametro); //info
//Cacharro
void func_comando_nivelActivo(int iParametro, char* sParametro, float fParametro); ; //nivelActivo
//Red
void func_comando_IP(int iParametro, char* sParametro, float fParametro); ; //IP
//Ficheros
void func_comando_flist(int iParametro, char* sParametro, float fParametro); //fexist
void func_comando_fexist(int iParametro, char* sParametro, float fParametro); //fexist
void func_comando_fopen(int iParametro, char* sParametro, float fParametro); //fopen
void func_comando_fremove(int iParametro, char* sParametro, float fParametro); //fremove
void func_comando_format(int iParametro, char* sParametro, float fParametro); //format
//Reloj
void func_comando_hora(int iParametro, char* sParametro, float fParametro); //hora"    
void func_comando_minuto(int iParametro, char* sParametro, float fParametro); //minuto"    
void func_comando_segundo(int iParametro, char* sParametro, float fParametro); //segundo"
void func_comando_reloj(int iParametro, char* sParametro, float fParametro); //reloj 
//MQTT
void func_comando_enviarKeepAlive(int iParametro, char* sParametro, float fParametro); //debug
void func_comando_MQTTConfig(int iParametro, char* sParametro, float fParametro); //debug

OrdenesClass::OrdenesClass(void) {}

void OrdenesClass::gestionaOrdenes(int debug)
{
  while(HayOrdenes(debug)) EjecutaOrdenes(debug);
}

int OrdenesClass::HayOrdenes(int debug)
  {
  char inChar=0;
  
  while (Serial.available())
    {
    inChar=(char)Serial.read(); 
    switch (inChar)
      {
      case ';':
        //Recibido final de orden
        //if (debug) Serial.printf("Orden recibida: %s\n",ordenRecibida);
        return(1);
        break;
      default:
        //Nuevo caracter recibido. Añado y sigo esperando
        this->ordenRecibida[lonOrden++]=inChar;
        this->ordenRecibida[lonOrden]=0;
        break;
      }
    }  
  return(0); //No ha llegado el final de orden
  }

int OrdenesClass::EjecutaOrdenes(int debug){
  String comando="";
  String parametros="";
  int iParametro=0;
  char sParametro[LONG_PARAMETRO]="";//LONG_PARAMETRO longitud maxmima del parametro
  float fParametro=0.0;
  int inicioParametro=0;

  if (debug) 
    {
    Serial.print("Orden recibida: #");
    Serial.print(ordenRecibida);
    Serial.println("#");
    }
  
  for(int i=0;i<LONG_COMANDO;i++)
    {
    switch (ordenRecibida[i])
      {
      case ' ':
        //fin del comando, hay parametro
        inicioParametro=i+1;
        
        //Leo el parametro
        for (int j=0;j<LONG_ORDEN;j++)
          {  //Parsea la orden      
          if(ordenRecibida[j+inicioParametro]==0) 
            {
            strncpy(sParametro,ordenRecibida+inicioParametro,j+1);//copio el parametro como texto
            break;//j=LONG_ORDEN;//Cuando encuentro el final de la cadena
            }
          else iParametro=(iParametro*10)+(int)ordenRecibida[j+inicioParametro]-48; //hay que convertir de ASCII a decimal
          }
        fParametro=String(sParametro).toFloat();
        
        i=LONG_COMANDO;
        break;
      case 0:
        //fin de la orden. No hay parametro
        i=LONG_COMANDO;
        break;
      default:
        comando+=ordenRecibida[i];
        break;
      }
    }

  //Limpia las variables que3 recogen la orden
  lonOrden=0;
  ordenRecibida[0]=0;

  if (debug) 
    {
    Serial.println("comando: ");
    Serial.print("#"); Serial.println(comando.c_str()); Serial.println("#");
    Serial.print("#"); Serial.print(comando);          Serial.println("#");
    Serial.println(iParametro);
    Serial.println(sParametro);
    Serial.println(fParametro);
    }
    
/**************Nueva funcion ***************************/
  int8_t indice=0;
  for(indice=0;indice<MAX_COMANDOS;indice++)
    {
    if (this->comandos[indice].comando==comando) 
      {
      //Ejecuta la funcion asociada
      this->comandos[indice].p_func_comando(iParametro, sParametro, fParametro);
      return(0);
      }    
    }

  //Si llega aqui es que no ha encontrado el comando
  Serial.println("Comando no encontrado");
  return(-1);//Comando no encontrado  
/*******************************************************/
}

void OrdenesClass::limpiaOrden(void)
  {
  lonOrden=0;
  ordenRecibida[0]=0;
  }
  
void OrdenesClass::inicializaOrden(void)
  { 
  int i =0;  

  limpiaOrden();
  //Globales
  comandos[i].comando="help";
  comandos[i].descripcion="Listado de comandos";
  comandos[i++].p_func_comando=func_comando_help;
  
  comandos[i].comando="echo";
  comandos[i].descripcion="Devuelve el eco del sistema";
  comandos[i++].p_func_comando=func_comando_echo;
   
  comandos[i].comando="debug";
  comandos[i].descripcion="Activa/desactiva el modo debug";
  comandos[i++].p_func_comando=func_comando_debug;

  comandos[i].comando="restart";
  comandos[i].descripcion="Reinicia el modulo";
  comandos[i++].p_func_comando=func_comando_restart;
  
  comandos[i].comando="info";
  comandos[i].descripcion="Devuelve informacion del hardware";
  comandos[i++].p_func_comando=func_comando_info;
  //Cacharro  

  comandos[i].comando="nivelActivo";
  comandos[i].descripcion="Configura el nivel activo de los reles";
  comandos[i++].p_func_comando=func_comando_nivelActivo;
  
  //Red
  comandos[i].comando="IP";
  comandos[i].descripcion="Direccion IP";
  comandos[i++].p_func_comando=func_comando_IP;

  //Ficheros
  comandos[i].comando="flist";
  comandos[i].descripcion="Lista los ficheros en el sistema de ficheros";
  comandos[i++].p_func_comando=func_comando_flist;

  comandos[i].comando="fexist";
  comandos[i].descripcion="Indica si existe un fichero en el sistema de ficheros";
  comandos[i++].p_func_comando=func_comando_fexist;
  
  comandos[i].comando="fopen";
  comandos[i].descripcion="Devuelve el contenido del fichero especificado";
  comandos[i++].p_func_comando=func_comando_fopen;
  
  comandos[i].comando="fremove";
  comandos[i].descripcion="Borra el fichero especificado";
  comandos[i++].p_func_comando=func_comando_fremove;
  
  comandos[i].comando="format";
  comandos[i].descripcion="Formatea el sistema de ficheros";
  comandos[i++].p_func_comando=func_comando_format;
  
  //Reloj
  comandos[i].comando="hora";
  comandos[i].descripcion="Consulta la hora del sistema";
  comandos[i++].p_func_comando=func_comando_hora;
      
  comandos[i].comando="minuto";
  comandos[i].descripcion="Consulta los minutos del sistema";
  comandos[i++].p_func_comando=func_comando_minuto;
       
  comandos[i].comando="segundo";
  comandos[i].descripcion="Consulta los segundos del sistema";
  comandos[i++].p_func_comando=func_comando_segundo;
       
  comandos[i].comando="reloj";
  comandos[i].descripcion="Consulta el reloj del sistema";
  comandos[i++].p_func_comando=func_comando_reloj;

  //MQTT
  comandos[i].comando="enviarKeepAlive";
  comandos[i].descripcion="Activa/desactiva el envio de Kepp Alive MQTT";
  comandos[i++].p_func_comando=func_comando_enviarKeepAlive;
  
  comandos[i].comando="MQTTConfig";
  comandos[i].descripcion="Configuración de MQTT";
  comandos[i++].p_func_comando=func_comando_MQTTConfig;

    //resto
  for(;i<MAX_COMANDOS;)
    {
    this->comandos[i].comando="vacio";
    this->comandos[i].descripcion="Comando vacio";
    this->comandos[i++].p_func_comando=func_comando_vacio;  
    }

  func_comando_help(0, NULL, 0.0);
  }
//Declaro la instancia unica
OrdenesClass Ordenes;

/*********************************************************************/
/*  Funciones para los comandos                                      */
/*  void (*p_func_comando) (int, char*, float)                       */
/*********************************************************************/  
void func_comando_vacio(int iParametro, char* sParametro, float fParametro) //"vacio"
{}

void func_comando_help(int iParametro, char* sParametro, float fParametro) //"help"
  {
  Serial.println("\nComandos:"); 
  for(int8_t i=0;i<MAX_COMANDOS;i++) 
    {
    if (Ordenes.getComando(i)!="vacio") 
      {
      Serial.printf("Comando %i: [%s]\n",i, Ordenes.getComando(i).c_str());
      }
    }

  Serial.print("\n------------------------------------------------------------------------------\n");
  }

void func_comando_echo(int iParametro, char* sParametro, float fParametro)//"echo") 
  {
  Serial.printf("echo; %s\n",sParametro);
  }

void func_comando_debug(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  //cacharro.swapDebugGlobal();
  ++debugGlobal=debugGlobal % 2;
  if (debugGlobal) Serial.println("debugGlobal esta on");
  else Serial.println("debugGlobal esta off");
  }

void func_comando_restart(int iParametro, char* sParametro, float fParametro)//"restart")
  {
  ESP.restart();
  }
  
void func_comando_info(int iParametro, char* sParametro, float fParametro)//"info")
  {
  Serial.printf("\n-----------------info logica-----------------\n");
  Serial.printf("Uptime: %" PRId64 " segundos\n", (esp_timer_get_time()/(unsigned long)1000000)); //la funcion esp_timer_get_time() devuelve el contador de microsegundos desde el arranque. rota cada 292.000 años
  Serial.printf("-----------------------------------------------\n");  

  Serial.printf("\n-----------------info logica-----------------\n");
  Serial.printf("IP: %s\n", String(RedWifi.getIP(debugGlobal)).c_str());
  Serial.printf("nivelActivo: %s\n", String(cacharro.getNivelActivo()).c_str());  
  Serial.printf("-----------------------------------------------\n");  
  
  Serial.printf("-------------------WiFi info-------------------\n");
  Serial.printf("SSID: %s\n",RedWifi.nombreSSID().c_str());
  Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("Potencia: %s\n",String(WiFi.RSSI()).c_str());
  Serial.printf("-----------------------------------------------\n");   
      
  Serial.printf("-----------------Hardware info-----------------\n");
  //Serial.printf("Vcc: %i\n",ESP.getVcc());
  Serial.printf("FreeHeap: %i\n",ESP.getFreeHeap());
  Serial.printf("ChipId: %i\n",ESP.getChipRevision());
  Serial.printf("SdkVersion: %s\n",ESP.getSdkVersion());
  //Serial.printf("CoreVersion: %s\n",ESP.getCoreVersion().c_str());
  //Serial.printf("FullVersion: %s\n",ESP.getFullVersion().c_str());
  //Serial.printf("BootVersion: %i\n",ESP.getBootVersion());
  //Serial.printf("BootMode: %i\n",ESP.getBootMode());
  Serial.printf("CpuFreqMHz: %i\n",ESP.getCpuFreqMHz());

  //Serial.printf("FlashChipId: %i\n",ESP.getFlashChipId());
      //gets the actual chip size based on the flash id
  //Serial.printf("FlashChipRealSize: %i\n",ESP.getFlashChipRealSize());

      //gets the size of the flash as set by the compiler
  Serial.printf("FlashChipSize: %i\n",ESP.getFlashChipSize());
  Serial.printf("FlashChipSpeed: %i\n",ESP.getFlashChipSpeed());
      //FlashMode_t ESP.getFlashChipMode());
      
  //Serial.printf("FlashChipSizeByChipId: %i\n",ESP.getFlashChipSizeByChipId()); 
  Serial.printf("-----------------------------------------------\n");
  }  

void func_comando_nivelActivo(int iParametro, char* sParametro, float fParametro) //"nivelActivo"
  {
  if(sParametro[0]!=0) 
    {
    cacharro.setNivelActivo(iParametro);

    String cad="";
    
    if(!SistemaFicheros.leeFicheroConfig(GLOBAL_CONFIG_FILE, cad)) Serial.println("No se pudo leer el fichero");
    cad=cacharro.generaJsonConfiguracionNivelActivo(cad, cacharro.getNivelActivo());
    if(!SistemaFicheros.salvaFicheroConfig(GLOBAL_CONFIG_FILE, GLOBAL_CONFIG_BAK_FILE, cad)) Serial.println("No se pudo salvar el fichero");      
    }
  Serial.printf("\nNivel activo: %i\n",cacharro.getNivelActivo());  
  }  

void func_comando_IP(int iParametro, char* sParametro, float fParametro) //"IP"
  {
  boolean debug=false;
  Serial.println(RedWifi.getIP(debug));  
  }  

void func_comando_flist(int iParametro, char* sParametro, float fParametro)//"fexist")
  {
  String contenido="";  
  if(SistemaFicheros.listaFicheros(contenido)) 
    {
    contenido.replace("|","\n");
    Serial.printf("Contendio del sistema de ficheros:\n %s\n",contenido.c_str());
    }
  else Serial.printf("Ha habido un problema.....\n");
  }

void func_comando_fexist(int iParametro, char* sParametro, float fParametro)//"fexist")
  {
  if (strlen(sParametro)==0) Serial.println("Es necesario indicar un nombre de fichero");
  else
    {
    if(SPIFFS.exists(sParametro)) Serial.printf("El fichero %s existe.\n",sParametro);
    else Serial.printf("NO existe el fichero %s.\n",sParametro);
    }
  }

void func_comando_fopen(int iParametro, char* sParametro, float fParametro)//"fopen")
  {
  if (strlen(sParametro)==0) Serial.println("Es necesario indicar un nombre de fichero");
  else
    {
    File f = SPIFFS.open(sParametro, "r");
    if (f)
      { 
      Serial.println("Fichero abierto");
      size_t tamano_fichero=f.size();
      Serial.printf("El fichero tiene un tamaño de %i bytes.\n",tamano_fichero);
      char buff[tamano_fichero+1];
      f.readBytes(buff,tamano_fichero);
      buff[tamano_fichero+1]=0;
      Serial.printf("El contenido del fichero es:\n******************************************\n%s\n******************************************\n",buff);
      f.close();
      }
    else Serial.printf("Error al abrir el fichero %s.\n", sParametro);
    } 
  } 

void func_comando_fremove(int iParametro, char* sParametro, float fParametro)//"fremove")
  {
  if (strlen(sParametro)==0) Serial.println("Es necesario indicar un nombre de fichero");
  else
    { 
    if (SPIFFS.remove(sParametro)) Serial.printf("Fichero %s borrado\n",sParametro);
    else Serial.printf("Error al borrar el fichero%s\n",sParametro);
    } 
 }

void func_comando_format(int iParametro, char* sParametro, float fParametro)//"format")
  {     
  if (SistemaFicheros.formatearFS()) Serial.println("Sistema de ficheros formateado");
  else Serial.println("Error al formatear el sistema de ficheros");
  } 

void func_comando_hora(int iParametro, char* sParametro, float fParametro)//"hora"    
  {
  Serial.printf("La hora es %i\n",reloj.hora());
  }
  
void func_comando_minuto(int iParametro, char* sParametro, float fParametro)//"minuto"    
  {
  Serial.printf("Los minutos son %i\n",reloj.minuto());
  }
  
void func_comando_segundo(int iParametro, char* sParametro, float fParametro)//"segundo"
  {
  Serial.printf("Los segundos son %i\n",reloj.segundo());
  }

void func_comando_reloj(int iParametro, char* sParametro, float fParametro)//"reloj") 
  {
  reloj.imprimeDatosReloj();  
  if(reloj.cambioHorario()==1) Serial.println("Horario de verano");
  else Serial.println("Horario de invierno");
  } 
  
void func_comando_enviarKeepAlive(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  if(iParametro!=0) miMQTT.setEnviarKeepALive(1);
  else  miMQTT.setEnviarKeepALive(0);

  Serial.printf("enviarKeepAlive=%i\n", miMQTT.getEnviarKeepAlive());
  }

void func_comando_MQTTConfig(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  Serial.printf("Configuracion leida:\nID MQTT: %s\nIP broker: %s\nIP Puerto del broker: %i\nUsuario: %s\nPassword: %s\nTopic root: %s\nEnviar KeepAlive: %i\nPublicar entradas: %i\nPublicar salidas: %i\nWill topic: %s\nWill msg: %s\nCelan session: %i\n",miMQTT.getID_MQTT().c_str(),miMQTT.getIPBroker().toString().c_str(),miMQTT.getPuertoBroker(),miMQTT.getUsuarioMQTT().c_str(),miMQTT.getPasswordMQTT().c_str(),miMQTT.gettopicRoot().c_str(),miMQTT.getEnviarKeepAlive(),miMQTT.getPublicarEntradas(),miMQTT.getPublicarSalidas(),(miMQTT.gettopicRoot()+"/"+String(WILL_TOPIC)).c_str(),WILL_MSG, (CLEAN_SESSION?1:0));
  }  
/***************************** FIN funciones para comandos ******************************************/ 
