#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

/*
######################################################################################
#                       DEFINICIONES WEB Y REDES
######################################################################################
*/

///////////////////////////////////////////////////////////
// definicion de contraseña de punto de acceso
///////////////////////////////////////////////////////////
#define ACCESS_POINT_SSID         "smart plant"
#define ACCESS_POINT_PASSWORD     "smart plant"

#define ROUTER_WIFI_SSID          "link_home"
#define ROUTER_WIFI_PASSWORD      "chavez8140"

#define SERVER_PUERTO                 80
///////////////////////////////////////////////////////////
//definiciones generales para la comunicacion web
#define version          "0.1"
#define TEXT_PLANO       "text/plain"
#define JSON             "application/json"

#define ESTADO_ENCENDIDO "ENCENDIDO"
#define ESTADO_APAGADO   "APAGADO"

#define ENABLE          true
#define DISABLE         false


////////////////////////////////////////////////////
//           definiciones de urls
///////////////////////////////////////////////////
// definicion de las rutas 

#define WEB_URL_RAIZ           "/"
#define WEB_URL_SENSOR         "/sensor"
#define WEB_URL_ACCION         "/accion"
#define WEB_URL_HOST_CONFING   "/sta" 
#define WEB_URL_MI_IP          "/ip"  

////////////////////////////////////////////////////
//           definiciones de variable urls
///////////////////////////////////////////////////
// define todas las nombre de las variable que recibar desde la urls

#define URL_VAR_SSID           "ssid"
#define URL_VAR_PASSWORD       "password"
#define URL_VAR_SENSOR         "sensor"
#define URL_VAR_ACTUADOR       "actuador"   
#define URL_VAR_ACTUADOR_VALOR "valor"
#define URL_VAR_TIEMPO         "tiempo"
/////////////////////////////////////////////////////
//        definiciones de estructura JSON
/////////////////////////////////////////////////////
// Definie todo lo de la estructuras json

// "{ \"estado\":\"" + _estado + " \",\"IP\":\"" + _ip + "\" }"
const String JS               = " \"";
const String JS_INICIO        = " { ";
const String JS_FIN           = " } ";
const String JS_ADD           = " , ";
const String JS_CV            = " : ";

const String JS_ESTADO        = "\"estado\"";
const String JS_IP            = "\"ip\"";
const String JS_SENSOR        = "\"sensor\"";
const String JS_ACTUADOR      = "\"actuador\"";
const String JS_VALOR         = "\"valor\"";

const String JS_LUZ           = "\"ldr\"";
const String JS_WATER_LEVEL   = "\"agua nivel\"";
const String JS_HUMEDAD_AIRE  = "\"humedad aire\"";
const String JS_TEMPERATURA   = "\"temperatura\"";
const String JS_HUMEDAD_SUELO = "\"humedad suelo\"";
const String JS_LED           = "\"led\"";
const String JS_SERVO         = "\"servo\"";

////////////////////////////////////////////////////
//           definiciones de Errores
///////////////////////////////////////////////////
// definicion de palabras de errores
#define WEB_EXITO              "0 RECIBIDO"
#define WEB_ERROR_RECURSO      "1 RECURSO NO ENCONTRADO"
#define WEB_ERROR_PARAMETROS   "2 PARAMETRO NO ENCONTRADO"
#define WEB_ERROR_METHOD       "3 METODO NO ENCONTRADO"
#define WEB_ERROR_SENSOR       "4 SENSOR NO ENCONTRADO"
#define WEB_ERROR_ACCION       "5 ACTUADOR NO ENCONTRADO"
#define WEB_ERROR_VALOR        "6 VALOR PARA EL ACTUADOR NO ENCONTRADO"
#define WEB_ERROR_ACCION_GET   "7 ACTUADOR NO SE ENCUENTRA VALOR"

// establaciendo valores para el servidor
ESP8266WebServer server(SERVER_PUERTO);

IPAddress local_ip(192, 168, 10 , 1);
IPAddress gateway (192, 168, 10 , 1);
IPAddress subnet  (255, 255, 255, 0);

/// STA

String ssid_sta       = ROUTER_WIFI_SSID;
String password_sta   = ROUTER_WIFI_PASSWORD;

bool sta_conectandose = false;

/*
##################################################################################
#              DEFINICIONES DE CONTROL Y MONITOREO  
##################################################################################
*/

//////////////////////////////////////////////////
//           hilos de ejecusion
//////////////////////////////////////////////////
//disparadores
#define TReconecion 10000  // 10segundos
#define TMonitoreo  5000    // 1 segundo

//cronomecros
unsigned long TP_reconecion       = 0;
unsigned long TP_monitoreo_planta = 0;

bool estado_hilo_reconexion = DISABLE;
bool estado_hilo_monitoreo_planta = DISABLE;

//////////////////////////////////////////////
//      Definicion de comunicacion Serial
//////////////////////////////////////////////
// configuracion de virutal serial

#include <SoftwareSerial.h>
#define RxEsp 0
#define TxEsp 2
SoftwareSerial softwareSerial(RxEsp,TxEsp);

String mensaje_arduino  = "";
String mandar_mensaje   = "";

String comando          = "";
String comando_objetivo = "";
String comando_valor    = "";

/////////////////////////////////////////////////
//      Definciones de comando arduino ERIC
/////////////////////////////////////////////////

#define C_LECTURA       "GET"
#define C_ACTUADOR      "MOVE"

// sensores
#define C_TODO          "TODO" 
#define C_LDR           "LDR"
#define C_LDR0          "LDR0"
#define C_LDR1          "LDR1"
#define C_LDR2          "LDR2"
#define C_Humedad_aire  "HUM_AIRE"
#define C_Temperatura   "TEMP"
#define C_Humedad_suelo "HUM_PISO"
#define C_Nivel_agua    "WL"

//Actuadores
#define C_servo         "SERVO"
#define C_LED           "LED"

//////////////////////////////////////////////////
//       ERRORES DE COMUNICASION SERIALES
//////////////////////////////////////////////////

#define SERIAL_EXITO                 '0' // LISTO"
#define SERIAL_ERROR_COMANDO         '1' // COMANDO NO ENCONTRADO"
#define SERIAL_ERROR_OBJETIVO        '2' // OBJETIVO NO ENCONTRADO"
#define SERIAL_ERROR_VALOR           '3' // VALOR NO ENCONTRADO"
#define SERIAL_ERROR_VALOR_NO_VALIDO '4' // ERROR AL LEER EL VALOR"
#define SERIAL_ERROR_DESCONOCIDO     '5' // ERROR DESCONOCIDO"
#define SERIAL_SENSORES              '6' // LEER SENSORES

//////////////////////////////////////////////////
//       PETICIONES DE COMUNICASION SERIALES
//////////////////////////////////////////////////

#define LEER_SENSORES         "GET TODO 0"
#define LEER_LDR              "GET LDR 0"
#define LEER_TEMPERATURA      "GET TEMP 0"
#define LEER_HUMEDAD_AIRE     "GET HUM_AIRE 0"
#define LEER_HUMEDAD_SUELO    "GET HUM_SUELO 0"
#define LEER_NIVEL_AGUA       "GET WL 0"
#define LEER_LED              "GET LED 0"
#define LEER_SERVO            "GET SERVO 0"

//////////////////////////////////////////////////
//       PETICIONES DE COMUNICASION SERIALES
//////////////////////////////////////////////////

#define ACTUADOR_SERVO        "MOVE SERVO "
#define ACTUADOR_LED          "MOVE LED "

//////////////////////////////////////////////////
//       valores de los sensores
//////////////////////////////////////////////////
int sensor_ldr0          = 0;
int sensor_ldr1          = 0;
int sensor_ldr2          = 0;
int sensor_ldr_promedio  = 0;
int sensor_humedad_aire  = 0;
int sensor_humedad_suelo = 0;
int sensor_temperatura   = 0;
int sensor_nivel_agua    = 0;
int sensor_led           = 0;
int sensor_servo         = 0;


////////////////////////////////////////////////
// control de peticiones
///////////////////////////////////////////////

#define STOP_COMUNICACION  "STOP"
#define START_COMUNICACION "START"
bool comunicacion_enable = true;




/*
###########################################################################
#
#                        SETUP DEL SISTEMA 
#
###########################################################################
*/


void setup() {

  Serial.begin(115200);
  softwareSerial.begin(9600);
  red_setup();
  softwareSerial.println("LISTO");
}

void loop() {
  // put your main code here, to run repeatedly:
  loop_red();
  
  loop_sistema_comunicacion();
  loop_monitoreo_control();

  hilo_reconection();
  hilo_monitoreo();
}


void hilo_reconection(){
  unsigned long tiempo_transcurrido = millis();                           // hilo de tarea que se ejecuta una vez cada 10 segundo
  if (tiempo_transcurrido - TP_reconecion >= TReconecion) {               //comprueba si ya pasaron 10 segundo para volver a reconectar;
    
    TP_reconecion = tiempo_transcurrido;
    estado_hilo_reconexion = ENABLE;                                              // Disparador del hilo 

    // aqui debajo el codigo que se ejecutara cada 10 segundo
    loop_control_super_lento();
  }else{
    estado_hilo_reconexion = DISABLE;
  }
}

void hilo_monitoreo(){
  unsigned long tiempo_transcurrido = millis();                           // hilo de tarea que se ejecuta una vez cada 5 segundo
  if (tiempo_transcurrido - TP_monitoreo_planta >= TMonitoreo) {               //comprueba si ya pasaron 10 segundo para volver a reconectar;
    TP_monitoreo_planta = tiempo_transcurrido;
    estado_hilo_monitoreo_planta = ENABLE;                                              // Disparador del hilo 

    // Aqui debajo el codigo que se ejecutara cada 5 segund
    loop_control_lento();


  }else{
    estado_hilo_monitoreo_planta = DISABLE;
  }
}
/*
###########################################################################
#
#                        Configuracion de red
#
###########################################################################
*/



//////////////////////////////////////////////////////////////////////////
//              configuracion incial de RED y SERVIDOR
//////////////////////////////////////////////////////////////////////////
// funcion para setup es para red y servidor
void red_setup() {
  ap_setting();          // Configuracion de punto de aceso ( ACESS POINT)
  server_setting();      // Configuracion del Servidor // rutas
  sta_conectarse_red();  // Conectarse a la RED EXISTENTE
}

void loop_red(){
  server.handleClient();
  sta_connection_try();
}



//////////////////////////////////////////////////////////////////////////
//                    ENCENDER EL MODO ACCESS POINTS
//////////////////////////////////////////////////////////////////////////
//configuracion AP
void ap_setting() {
  WiFi.softAPConfig(local_ip, gateway, subnet);  // configurarmos AP
  WiFi.softAP(ACCESS_POINT_SSID, ACCESS_POINT_PASSWORD);                   // Establecemos el SSID y PASSWORD

  Serial.println("Punto de acceso iniciado");    // CAMBIAR SALIDA
  Serial.print("Direccion IP: ");                // CAMBIAR SALIDA
  Serial.println(WiFi.softAPIP());               // CAMBIAR SALIDA
}



//////////////////////////////////////////////////////////////////////////
//          ENCENDER EL MODO WIFI Y CONECTARSE A LA RED
//////////////////////////////////////////////////////////////////////////
//// conectarse a una red existente
void sta_conectarse_red() {

  WiFi.begin(ssid_sta, password_sta);              // conectarse a la RED
  sta_conectandose = true;                         // _Estado de intento de conection

  Serial.println("conectandose a: " + ssid_sta);   // CAMBIAR SALIDA
}


// si se desconecta la red volverse a conectar 
void sta_connection_try() {

  byte __estado = WiFi.status();                                          // sabemos si esta conectado a la red o no

  if (__estado != WL_CONNECTED && sta_conectandose == false) {            // si no esta coenctado y no esta intentando conectarse
    sta_conectarse_red();                                                 // vuelve a intentar conectarse
    sta_conectandose = true;                                              // _Estado de intento de conection

    Serial.println("conectandose a la red: " + ssid_sta);                 // CAMBIAR SALIDA manda la red que esta intentando conectarse
  }


  if (__estado == WL_CONNECTED) {                                          // si esta conectado
    if (sta_conectandose == true) {                                        // nos muestra la ip
      Serial.println("conexion exitosa, IP:" + WiFi.localIP().toString()); // CAMBIAR SALIDA
    }
    sta_conectandose = false;                                              // _Estado de intento de conection
    return;                                                                // Forsamos salida de la funcion
  }
  if(estado_hilo_reconexion == ENABLE){                                    //comprueba si ya pasaron 10 segundo para volver a reconectar;
    sta_conectandose = false;
  }

}



/*
########################################################################
#
#                      configuracion de SERVIDOR
#
########################################################################
*/




//////////////////////////////////////////////////////////////////////////
//                     Definicion de rutas 
//////////////////////////////////////////////////////////////////////////
// Definicion de todas las rutas del servidor 
void server_setting() {
  server.on("/", raiz_get);
  server.on("/ip", mi_ip);
  server.on("/sta", sta_metodo);
  server.on("/sensor",sensor_get);
  server.on("/accion",accion_method);

  server.onNotFound(no_encontrado);

  // importante inicialza el servidor
  server.begin();
  //ninguna ruta debe ir por debajo de server.begin
}

//////////////////////////////////////////////////////////////////////////
//                     RUTA: /   metodos get
//////////////////////////////////////////////////////////////////////////
// RAIZ
void raiz_get() {
  server.send(200, TEXT_PLANO, version);
}



//////////////////////////////////////////////////////////////////////////
//                 RUTA: no encontrado   metodos get
//////////////////////////////////////////////////////////////////////////
// pagina no encontrada
void no_encontrado() {
  server.send(404, TEXT_PLANO, WEB_ERROR_RECURSO);
}



//////////////////////////////////////////////////////////////////////////
//                    RUTA: /ip   metodos get
//////////////////////////////////////////////////////////////////////////
// devuelve la ip del dispositivo
void mi_ip() {
  IPAddress clientIP = server.client().remoteIP();                   // OBtiene IP

  Serial.print("Dispositivo conectado desde la IP: ");               // CAMBIAR SALIDA 
  Serial.println(clientIP);                                          // CAMBIAR SALIDA Imprime la IP del dispositivo 

  server.send(200, TEXT_PLANO, "Mi IP es : " + clientIP.toString());// MANDA LA IP DEL cliente del server 
}



//////////////////////////////////////////////////////////////////////////
//                 RUTA: /sta   metodos get y post
//////////////////////////////////////////////////////////////////////////
// diferenciacion de metodos post y get para STA
void sta_metodo() {                                       
  if (server.method() == HTTP_GET) {                      // Metodo GET
    sta_get();
  } else if (server.method() == HTTP_POST) {              // Metodo POST
    sta_post();
  } else {                                                // si es un metodo diferente
    server.send(400, TEXT_PLANO, WEB_ERROR_PARAMETROS);  // manda mensaje de error al cliente
  }
}


// gestor de metodo get para la ruta /sta 
void sta_get() {                        
  
  String jsonResponse;                  // string para la respuesta en formato JSON
  String _estado = "";                  // Variable para guardar el estado de la RED
  String _ip = "";                      // Variable para guardar la ip que toma el HOST

  if (WiFi.status() == WL_CONNECTED) {  // comprueba Si se pudo conectar a la Red
    _estado = ESTADO_ENCENDIDO;                // Estado de la conection
    _ip = WiFi.localIP().toString();    // IP del HOST
  } else {
    _estado = ESTADO_APAGADO;                  // Estado de ka conection
    _ip = WiFi.softAPIP().toString();   // IP del host pero como punto de Acceso
  }

  // respuesta
  //jsonResponse = "{ \"estado\":\"" + _estado + " \",\"IP\":\"" + _ip + "\" }";  // json de respuesta
  jsonResponse = JS_INICIO + 
                 JS_ESTADO + JS_CV + JS + _estado + JS + 
                 JS_ADD +
                 JS_IP     + JS_CV + JS + _ip     + JS +
                 JS_FIN;

  server.send(200, JSON, jsonResponse);
}


// gestor de metodo post para la ruta /sta 
void sta_post() { 

  if (server.hasArg(URL_VAR_SSID) &&                            // Comprobar si recibio los argumentos de SSID y PASSWORD  
      server.hasArg(URL_VAR_PASSWORD)) {                         

    String _ssid_sta = server.arg(URL_VAR_SSID);                // OBtener el valor mandado de SSID
    String _password_sta = server.arg(URL_VAR_PASSWORD);        // Obtener el valor mandado de PASSWORD

    ssid_sta = _ssid_sta;                                       // Pasamos la red la cual nos conectaremos
    password_sta = _password_sta;                               // Pasamos la Password de la red a la cual nos conectaremos

    sta_conectarse_red();                                       // conectarse a la nueva red 

    //respuestaaa
    Serial.println(_ssid_sta + _password_sta);                  // CAMBIAR SALIDA 
    server.send(200, TEXT_PLANO, WEB_EXITO);                   // Manda respuesta al cliente Web
  } else {
    server.send(400, TEXT_PLANO, WEB_ERROR_PARAMETROS);        // Manda respuesta de error
  }
}


//////////////////////////////////////////////////////////////////////////
//                 RUTA: /Sensor   metodos get 
//////////////////////////////////////////////////////////////////////////
//devuelve los valores de los sensores 
void sensor_get(){
  String tipo_sensor;
  if (server.hasArg(URL_VAR_SENSOR)){
    tipo_sensor   = server.arg(URL_VAR_SENSOR);                 // Mandar solo el sensor recibido
    String _valor = String(detectar_valor_sensor(tipo_sensor));

    String jsonResponse;
    jsonResponse = JS_INICIO + 
                   JS_SENSOR + JS_CV + tipo_sensor +
                   JS_ADD    +
                   JS_VALOR  + JS_CV + _valor + 
                   JS_FIN;
    
    server.send(200,JSON,jsonResponse);
  }else{
    String jsonResponse;
    jsonResponse = JS_INICIO         +
                    JS_LUZ           + JS_CV + JS + String(sensor_ldr_promedio)  + JS +
                    JS_ADD           +
                    JS_WATER_LEVEL   + JS_CV + JS + String(sensor_nivel_agua)    + JS +
                    JS_ADD           +
                    JS_HUMEDAD_AIRE  + JS_CV + JS + String(sensor_humedad_aire)  + JS +
                    JS_ADD           +
                    JS_HUMEDAD_SUELO + JS_CV + JS + String(sensor_humedad_suelo) + JS +
                    JS_ADD           +
                    JS_TEMPERATURA   + JS_CV + JS + String(sensor_temperatura)   + JS +
                    JS_ADD           +
                    JS_LED           + JS_CV + JS + String(sensor_led)           + JS +
                    JS_ADD           +
                    JS_SENSOR        + JS_CV + JS + String(sensor_servo)         + JS +
                    JS_FIN;

    server.send(200,JSON,jsonResponse);             // MANDAR TODO LOS VALORES DE LOS SENSORES 
  }

}

//////////////////////////////////////////////////////////////////////////
//                 RUTA: /ACCION   metodos get y post
//////////////////////////////////////////////////////////////////////////
// gestor de los metodos post y get 
void accion_method(){

  if(server.method() == HTTP_GET){
    accion_get();
  }else if(server.method() == HTTP_POST) {
    accion_post();
  }else{
    server.send(404,TEXT_PLANO,"RECURSO NO ENCONTRADO");
  }
}

// metodos get de la ruta "/accion"
void accion_get(){
  if(server.hasArg(URL_VAR_ACTUADOR)){
    String actuador = server.arg(URL_VAR_ACTUADOR);           // Mandar solo el sensor recibido
    String _valor = String(detectar_valor_sensor(actuador));

    String jsonResponse = json_actuador(actuador,_valor);

    server.send(200,JSON,jsonResponse);
  }else{
    String jsonResponse;
    jsonResponse = JS_INICIO   +
                    JS_LED     + JS_CV + JS + String(sensor_led)   + JS +
                    JS_ADD     +
                    JS_SENSOR  + JS_CV + JS + String(sensor_servo) + JS +
                    JS_FIN;

    server.send(200,JSON,jsonResponse);
  }
}

// metodo post de la ruta "/accion"
void accion_post(){
  if (server.hasArg(URL_VAR_ACTUADOR) && server.hasArg(URL_VAR_ACTUADOR_VALOR)){
    String _actuador = server.arg(URL_VAR_ACTUADOR);
    String _valor = server.arg(URL_VAR_ACTUADOR_VALOR);

    String jsonResponse = json_actuador(_actuador,_valor);
    solictar_actuadores("MOVE "+_actuador+" ",_valor);
    
    server.send(200,JSON,jsonResponse);
  }else{
    server.send(400,TEXT_PLANO,"ERROR POST");
  }
}

String json_actuador(String _url_var,String _new_valor){

  String actuador = _url_var;                     // Mandar solo el sensor recibido
  String _valor = _new_valor;
  String _jsonResponse;
  _jsonResponse = JS_INICIO   + 
                  JS_ACTUADOR + JS_CV + actuador +
                  JS_ADD      +
                  JS_VALOR    + JS_CV + _valor + 
                  JS_FIN;
  return _jsonResponse;
}

int detectar_valor_sensor(String _tipo){
  if(_tipo == C_LDR){
    return sensor_ldr_promedio;
  }else if (_tipo == C_Humedad_aire){
    return sensor_humedad_aire;
  }else if (_tipo == C_Humedad_suelo){
    return sensor_humedad_suelo;
  }else if (_tipo == C_Temperatura){
    return sensor_temperatura;
  }else if (_tipo == C_Nivel_agua){
    return sensor_nivel_agua;
  }else if (_tipo == C_servo){
    return sensor_servo;
  }else if (_tipo == C_LED){
    return sensor_led;
  }else{
    return -1;
  }
}

/*
##################################################################################
#
#               COMUNICACION ENTRE ARDUINO Y ESP
#
###################################################################################
*/

void loop_sistema_comunicacion(){
  leer_serial_esp();
  leer_serial_arduino();
}


/////////////////////////////////////////////////////////
// Lectura de monitor serial fisico

void leer_serial_arduino (){
  if(Serial.available()){
    mandar_mensaje = Serial.readStringUntil('\n');
    softwareSerial.println(mandar_mensaje);
    Serial.println("mensaje enviado al arduino: " + mandar_mensaje);

    if (mandar_mensaje == STOP_COMUNICACION ) {
      comunicacion_enable = false;
    }else if (mandar_mensaje == START_COMUNICACION) {
      comunicacion_enable = true;
    
    }

    
  }

}

////////////////////////////////////////////////////////////
// Lectura de monitor serial que esta coenctado al arduino
void leer_serial_esp (){
  if(softwareSerial.available()){                             // lee si hay un dato por serial
    mensaje_arduino = softwareSerial.readStringUntil('\n');   // espera que lleguen todos los datos
    comando = mensaje_arduino;

    Serial.println("mensaje recibido: " + mensaje_arduino);   // imprime el dato en consola
    obtencion_data();
    
  }

}

// corte de datos recibido por serial
String cortar_comando(String dato){
  byte _inicio_valor = dato.indexOf(' ');
  byte _fin_valor    = dato.indexOf(' ',_inicio_valor + 1);

  comando_objetivo = dato.substring(0,_inicio_valor);
  comando_valor    = dato.substring(_inicio_valor +1 , _fin_valor);
  return comando   = dato.substring(_fin_valor +1);
}

// comprueba si es un numero
int comprobar_valor(){
  return comando_valor.toInt();
}

// compara con los datos obtenido para sacar un valor 
void interpretacion_objetivo(){
  if(comando_objetivo == C_LDR0){
    sensor_ldr0 = comprobar_valor();
  }else if (comando_objetivo == C_LDR1){
    sensor_ldr1 = comprobar_valor();
  }else if (comando_objetivo == C_LDR2){
    sensor_ldr2 = comprobar_valor();
  }else if (comando_objetivo == C_Humedad_aire){
    sensor_humedad_aire = comprobar_valor();
  }
  else if (comando_objetivo == C_Humedad_suelo){
    sensor_humedad_suelo = comprobar_valor();
  }
  else if (comando_objetivo == C_Temperatura){
    sensor_temperatura = comprobar_valor();
  }else if (comando_objetivo == C_Nivel_agua){
    sensor_nivel_agua = comprobar_valor();
  }else if (comando_objetivo == C_LED){
    sensor_led = comprobar_valor();
  }else if (comando_objetivo == C_servo){
    sensor_servo = comprobar_valor();
  }else{
    Serial.println("ERROR");
  }

}

// gestiona los errores 
void interpretacion_errores_serial (char _data){
  if(_data == SERIAL_EXITO){
    Serial.println("COMANDO EJECUTADO CON EXITO");
  }else if(_data == SERIAL_SENSORES){
    Serial.println("DATO RECIBIDO");
  }else {
    //Serial.println(mensaje_arduino);
  }

}


// corte de todo los comandos 
void obtencion_data(){
  if(!isDigit(comando[0])){
      while (comando.length() >= 3){
        comando = cortar_comando(comando);                      // corta el comando y saca el primer objetivo y el primer valor
        interpretacion_objetivo();

        //Serial.println(comando_objetivo);
        //Serial.println(comando_valor);
        //Serial.println(comando);
        }
    }
}

// SOLICITA DATOS 
void solictar_sensores(String _comando){
  softwareSerial.println(_comando);
  Serial.println("Comando enviado: "+ _comando);
}

// SOLICITUD DE EJECUTION
void solictar_actuadores(String _actuadores,String _valor){
  softwareSerial.println(_actuadores + _valor);
  Serial.println("Comando enviado: "+ _actuadores + _valor);
}

/*
###################################################################################################
#
#             CONTROL Y MONITOREO DE LA PLANTA 
#
###################################################################################################
*/

// AQUI TODO LO QUE SE DEBE DE EJECUTAR LO MAS RAPIDO POSIBLE
void loop_monitoreo_control(){

}

// AQUI TODO LO QUE SE DEBE DE EJECUTAR CADA 5 SEGUNDO
void loop_control_lento(){
  
  //Serial.println(millis());
}

// AQUI TODO LO QUE SE DEBE DE EJECUTAR CADA 10 SEGUNDO
void loop_control_super_lento(){
  if (comunicacion_enable == true ){
    actualizar_serial();
  }
  
}



void actualizar_serial(){
  solictar_sensores(LEER_SENSORES);
  promedio_ldr();
}

void promedio_ldr(){
 sensor_ldr_promedio = int((sensor_ldr0 + sensor_ldr1 +sensor_ldr2) / 3); 
}



























