#include <ESP8266WiFi.h>       // Incluye la biblioteca para la conectividad WiFi en ESP8266
#include <ESP8266WebServer.h>  // Incluye la biblioteca para el servidor web en ESP8266

/*
######################################################################################
#                       DEFINICIONES WEB Y REDES
######################################################################################
*/

// Definición de la contraseña y el nombre del punto de acceso (AP)
#define ACCESS_POINT_SSID "smart plant"      // Nombre del punto de acceso
#define ACCESS_POINT_PASSWORD "smart plant"  // Contraseña del punto de acceso

// Definición del nombre y la contraseña del WiFi del router
#define ROUTER_WIFI_SSID "link_home"       // Nombre del WiFi del router
#define ROUTER_WIFI_PASSWORD "chavez8140"  // Contraseña del WiFi del router

// Puerto en el que el servidor escuchará las solicitudes
#define SERVER_PUERTO 80  // Puerto del servidor

// Definiciones generales para la comunicación web
#define version "0.1"            // Versión del software
#define TEXT_PLANO "text/plain"  // Tipo de contenido para texto plano
#define JSON "application/json"  // Tipo de contenido para JSON

// Estados de los dispositivos
#define ESTADO_ENCENDIDO "ENCENDIDO"  // Estado de encendido
#define ESTADO_APAGADO "APAGADO"      // Estado de apagado

// Habilitar o deshabilitar ciertas funciones
#define ENABLE true    // Habilitado
#define DISABLE false  // Deshabilitado

////////////////////////////////////////////////////
//           definiciones de urls
///////////////////////////////////////////////////
// Definición de las rutas web
#define WEB_URL_RAIZ "/"             // Ruta raíz del servidor
#define WEB_URL_SENSOR "/sensor"     // Ruta para acceder a los sensores
#define WEB_URL_ACCION "/accion"     // Ruta para acciones de los actuadores
#define WEB_URL_HOST_CONFING "/sta"  // Ruta para la configuración del host
#define WEB_URL_MI_IP "/ip"          // Ruta para obtener la dirección IP

////////////////////////////////////////////////////
//           definiciones de variable urls
///////////////////////////////////////////////////
// Define los nombres de las variables que se recibirán desde las URLs
#define URL_VAR_SSID "ssid"             // Variable para el SSID
#define URL_VAR_PASSWORD "password"     // Variable para la contraseña
#define URL_VAR_SENSOR "sensor"         // Variable para el tipo de sensor
#define URL_VAR_ACTUADOR "actuador"     // Variable para el actuador
#define URL_VAR_ACTUADOR_VALOR "valor"  // Variable para el valor del actuador
#define URL_VAR_TIEMPO "tiempo"         // Variable para el tiempo

/////////////////////////////////////////////////////
//        definiciones de estructura JSON
/////////////////////////////////////////////////////
// Define la estructura de los mensajes JSON

// Formato del JSON: "{ \"estado\":\"" + _estado + " \",\"IP\":\"" + _ip + "\" }"
const String JS = " \"";         // Comilla doble para el valor
const String JS_INICIO = " { ";  // Inicio del objeto JSON
const String JS_FIN = " } ";     // Fin del objeto JSON
const String JS_ADD = " , ";     // Separador entre pares clave-valor
const String JS_CV = " : ";      // Separador entre clave y valor

// Definiciones de claves en el JSON
const String JS_ESTADO = "\"estado\"";      // Clave para el estado
const String JS_IP = "\"ip\"";              // Clave para la IP
const String JS_SENSOR = "\"sensor\"";      // Clave para el sensor
const String JS_ACTUADOR = "\"actuador\"";  // Clave para el actuador
const String JS_VALOR = "\"valor\"";        // Clave para el valor

// Definiciones de sensores y actuadores
const String JS_LUZ = "\"ldr\"";                      // Sensor de luz
const String JS_WATER_LEVEL = "\"agua nivel\"";       // Sensor de nivel de agua
const String JS_HUMEDAD_AIRE = "\"humedad aire\"";    // Sensor de humedad del aire
const String JS_TEMPERATURA = "\"temperatura\"";      // Sensor de temperatura
const String JS_HUMEDAD_SUELO = "\"humedad suelo\"";  // Sensor de humedad del suelo
const String JS_LED = "\"led\"";                      // Estado del LED
const String JS_SERVO = "\"servo\"";                  // Estado del servo

////////////////////////////////////////////////////
//           definiciones de Errores
///////////////////////////////////////////////////
// Definición de mensajes de error
#define WEB_EXITO "0 RECIBIDO"                                    // Mensaje de éxito
#define WEB_ERROR_RECURSO "1 RECURSO NO ENCONTRADO"               // Error: recurso no encontrado
#define WEB_ERROR_PARAMETROS "2 PARAMETRO NO ENCONTRADO"          // Error: parámetro no encontrado
#define WEB_ERROR_METHOD "3 METODO NO ENCONTRADO"                 // Error: método no encontrado
#define WEB_ERROR_SENSOR "4 SENSOR NO ENCONTRADO"                 // Error: sensor no encontrado
#define WEB_ERROR_ACCION "5 ACTUADOR NO ENCONTRADO"               // Error: actuador no encontrado
#define WEB_ERROR_VALOR "6 VALOR PARA EL ACTUADOR NO ENCONTRADO"  // Error: valor no encontrado
#define WEB_ERROR_ACCION_GET "7 ACTUADOR NO SE ENCUENTRA VALOR"   // Error: valor del actuador no encontrado

// Estableciendo valores para el servidor
ESP8266WebServer server(SERVER_PUERTO);  // Crea una instancia del servidor en el puerto definido

// Configuración de la IP local y la puerta de enlace
IPAddress local_ip(192, 168, 10, 1);  // Dirección IP local del ESP8266
IPAddress gateway(192, 168, 10, 1);   // Puerta de enlace
IPAddress subnet(255, 255, 255, 0);   // Máscara de subred

/// STA (Estación)

// Variables para la conexión WiFi en modo estación
String ssid_sta = ROUTER_WIFI_SSID;          // SSID del router
String password_sta = ROUTER_WIFI_PASSWORD;  // Contraseña del router

bool sta_conectandose = false;  // Estado de conexión en modo estación

/*
##################################################################################
#              DEFINICIONES DE CONTROL Y MONITOREO  
##################################################################################
*/

// Hilos de ejecución
//////////////////////////////////////////////////
// Disparadores de temporización
#define TReconecion 10000  // Tiempo de reconexión (10 segundos)
#define TMonitoreo 5000    // Tiempo de monitoreo (5 segundos)

// Variables para almacenar tiempos de cronometraje
unsigned long TP_reconecion = 0;        // Cronómetro para la reconexión
unsigned long TP_monitoreo_planta = 0;  // Cronómetro para el monitoreo de la planta

bool estado_hilo_reconexion = DISABLE;        // Estado del hilo de reconexión
bool estado_hilo_monitoreo_planta = DISABLE;  // Estado del hilo de monitoreo de la planta

//////////////////////////////////////////////
//      Definición de comunicación Serial
//////////////////////////////////////////////
// Configuración de comunicación serial virtual

#include <SoftwareSerial.h>                   // Incluye la biblioteca para la comunicación serial virtual
#define RxEsp 0                               // Pin para recibir datos desde el ESP
#define TxEsp 2                               // Pin para enviar datos al ESP
SoftwareSerial softwareSerial(RxEsp, TxEsp);  // Crea un objeto de SoftwareSerial

// Variables para manejar mensajes
String mensaje_arduino = "";  // Mensaje recibido del Arduino
String mandar_mensaje = "";   // Mensaje a enviar al Arduino

String comando = "";           // Comando recibido
String comando_objetivo = "";  // Objetivo del comando
String comando_valor = "";     // Valor del comando

/////////////////////////////////////////////////
//      Definiciones de comandos para Arduino
/////////////////////////////////////////////////

#define C_LECTURA "GET"    // Comando para lectura
#define C_ACTUADOR "MOVE"  // Comando para mover un actuador

// Definiciones de sensores
#define C_TODO "TODO"               // Comando para leer todos los sensores
#define C_LDR "LDR"                 // Comando para leer el sensor de luz
#define C_LDR0 "LDR0"               // Comando para leer el sensor de luz 0
#define C_LDR1 "LDR1"               // Comando para leer el sensor de luz 1
#define C_LDR2 "LDR2"               // Comando para leer el sensor de luz 2
#define C_Humedad_aire "HUM_AIRE"   // Comando para leer humedad del aire
#define C_Temperatura "TEMP"        // Comando para leer temperatura
#define C_Humedad_suelo "HUM_PISO"  // Comando para leer humedad del suelo
#define C_Nivel_agua "WL"           // Comando para leer nivel de agua

// Definiciones de actuadores
#define C_servo "SERVO"  // Comando para el servo
#define C_LED "LED"      // Comando para el LED

//////////////////////////////////////////////////
//       ERRORES DE COMUNICACIÓN SERIALES
//////////////////////////////////////////////////

// Definición de mensajes de error en la comunicación serial
#define SERIAL_EXITO '0'                  // Indica éxito
#define SERIAL_ERROR_COMANDO '1'          // Error: comando no encontrado
#define SERIAL_ERROR_OBJETIVO '2'         // Error: objetivo no encontrado
#define SERIAL_ERROR_VALOR '3'            // Error: valor no encontrado
#define SERIAL_ERROR_VALOR_NO_VALIDO '4'  // Error: valor no válido
#define SERIAL_ERROR_DESCONOCIDO '5'      // Error desconocido
#define SERIAL_SENSORES '6'               // Comando para leer sensores

//////////////////////////////////////////////////
//       PETICIONES DE COMUNICACIÓN SERIALES
//////////////////////////////////////////////////

// Definiciones de comandos para solicitar datos de los sensores
#define LEER_SENSORES "GET TODO 0"            // Comando para leer todos los sensores
#define LEER_LDR "GET LDR 0"                  // Comando para leer el sensor de luz
#define LEER_TEMPERATURA "GET TEMP 0"         // Comando para leer temperatura
#define LEER_HUMEDAD_AIRE "GET HUM_AIRE 0"    // Comando para leer humedad del aire
#define LEER_HUMEDAD_SUELO "GET HUM_SUELO 0"  // Comando para leer humedad del suelo
#define LEER_NIVEL_AGUA "GET WL 0"            // Comando para leer nivel de agua
#define LEER_LED "GET LED 0"                  // Comando para leer estado del LED
#define LEER_SERVO "GET SERVO 0"              // Comando para leer estado del servo

//////////////////////////////////////////////////
//       PETICIONES DE COMUNICACIÓN SERIALES
//////////////////////////////////////////////////

// Comandos para controlar actuadores
#define ACTUADOR_SERVO "MOVE SERVO "  // Comando para mover el servo
#define ACTUADOR_LED "MOVE LED "      // Comando para mover el LED

//////////////////////////////////////////////////
//       Valores de los sensores
//////////////////////////////////////////////////

// Variables para almacenar los valores de diferentes sensores
int sensor_ldr0 = 0;           // Valor del sensor de luz 0
int sensor_ldr1 = 0;           // Valor del sensor de luz 1
int sensor_ldr2 = 0;           // Valor del sensor de luz 2
int sensor_ldr_promedio = 0;   // Promedio de los sensores de luz
int sensor_humedad_aire = 0;   // Valor de humedad del aire
int sensor_humedad_suelo = 0;  // Valor de humedad del suelo
int sensor_temperatura = 0;    // Valor de temperatura
int sensor_nivel_agua = 0;     // Valor de nivel de agua
int sensor_led = 0;            // Estado del LED
int sensor_servo = 0;          // Estado del servo


///////////////////////////////////////////////////
//        control
///////////////////////////////////////////////////

#define COMUNICACION_SERIAL_STOP "STOP"
#define COMUNICACION_SERIAL_START "START"

bool comunicacion_enable = true;

#define WIFI_DESCONECTAR "WIFI STOP"
#define WIFI_CONECTAR "WIFI START"

bool wifi_conection_enable = true;

/*
###########################################################################
#
#                        SETUP DEL SISTEMA 
#
###########################################################################
*/

// Función de configuración inicial del sistema
void setup() {
  pinMode(RxEsp, LOW);
  Serial.begin(115200);        // Inicia la comunicación serial a 115200 bps
  softwareSerial.begin(9600);  // Inicia la comunicación serial virtual a 9600 bps

  red_setup();                      // Configura la red
  softwareSerial.println("LISTO");  // Indica que el sistema está listo
}

void loop() {
  // Código principal que se ejecuta repetidamente
  loop_red();  // Maneja las solicitudes del servidor

  loop_sistema_comunicacion();  // Maneja la comunicación con el sistema

  loop_monitoreo_control();  // Monitorea y controla la planta

  hilo_reconection();  // Ejecuta el hilo de reconexión
  hilo_monitoreo();    // Ejecuta el hilo de monitoreo
}

// Hilo para manejar la reconexión a la red
void hilo_reconection() {
  unsigned long tiempo_transcurrido = millis();              // Tiempo transcurrido desde el inicio
  if (tiempo_transcurrido - TP_reconecion >= TReconecion) {  // Comprueba si han pasado 10 segundos
    TP_reconecion = tiempo_transcurrido;                     // Actualiza el cronómetro
    estado_hilo_reconexion = ENABLE;                         // Habilita el hilo de reconexión

    // Código que se ejecutará cada 10 segundos
    loop_control_super_lento();
  } else {
    estado_hilo_reconexion = DISABLE;  // Desactiva el hilo si no ha pasado el tiempo
  }
}

// Hilo para manejar el monitoreo de la planta
void hilo_monitoreo() {
  unsigned long tiempo_transcurrido = millis();                   // Tiempo transcurrido desde el inicio
  if (tiempo_transcurrido - TP_monitoreo_planta >= TMonitoreo) {  // Comprueba si han pasado 5 segundos
    TP_monitoreo_planta = tiempo_transcurrido;                    // Actualiza el cronómetro
    estado_hilo_monitoreo_planta = ENABLE;                        // Habilita el hilo de monitoreo

    // Código que se ejecutará cada 5 segundos
    loop_control_lento();
  } else {
    estado_hilo_monitoreo_planta = DISABLE;  // Desactiva el hilo si no ha pasado el tiempo
  }
}

/*
###########################################################################
#
#                        Configuración de red
#
###########################################################################
*/

// Función para configurar la red y el servidor
//////////////////////////////////////////////////////////////////////////
//              configuración inicial de RED y SERVIDOR
//////////////////////////////////////////////////////////////////////////
void red_setup() {
  ap_setting();          // Configura el punto de acceso (Access Point)
  server_setting();      // Configura el servidor y las rutas
  sta_conectarse_red();  // Conéctate a la red existente
}

// Función para manejar las solicitudes del servidor
void loop_red() {
  server.handleClient();  // Maneja las solicitudes del cliente
  sta_connection_try();   // Intenta conectarse a la red
}

//////////////////////////////////////////////////////////////////////////
//                    ENCENDER EL MODO ACCESS POINT
//////////////////////////////////////////////////////////////////////////

// Configuración del modo Access Point (AP)
void ap_setting() {
  WiFi.softAPConfig(local_ip, gateway, subnet);           // Configuramos la dirección IP, gateway y subnet del AP
  WiFi.softAP(ACCESS_POINT_SSID, ACCESS_POINT_PASSWORD);  // Establecemos el SSID y la contraseña del AP

  Serial.println("Punto de acceso iniciado");  // Mensaje de inicio del AP
  Serial.print("Direccion IP: ");              // Muestra la dirección IP del AP
  Serial.println(WiFi.softAPIP());             // Imprime la dirección IP del AP
}

//////////////////////////////////////////////////////////////////////////
//          ENCENDER EL MODO WIFI Y CONECTARSE A LA RED
//////////////////////////////////////////////////////////////////////////

// Conectar a una red existente
void sta_conectarse_red() {
  WiFi.begin(ssid_sta, password_sta);  // Inicia la conexión a la red
  sta_conectandose = true;             // Estado de intento de conexión

  Serial.println("conectandose a: " + ssid_sta);  // Mensaje de conexión
}

// Si se desconecta de la red, intenta reconectarse
void sta_connection_try() {
  if (wifi_conection_enable == false) {
    return;
  }


  byte __estado = WiFi.status();  // Obtiene el estado de la conexión

  // Si no está conectado y no está intentando conectarse
  if (__estado != WL_CONNECTED && sta_conectandose == false) {
    sta_conectarse_red();     // Intenta reconectarse
    sta_conectandose = true;  // Actualiza el estado de conexión

    Serial.println("conectandose a la red: " + ssid_sta);  // Mensaje de intento de conexión
  }

  // Si está conectado
  if (__estado == WL_CONNECTED) {
    if (sta_conectandose == true) {                                         // Si estaba intentando conectarse
      Serial.println("conexion exitosa, IP:" + WiFi.localIP().toString());  // Muestra la IP local
    }
    sta_conectandose = false;  // Resetea el estado de conexión
    return;                    // Sale de la función
  }

  // Si el hilo de reconexión está habilitado, permite que intente reconectar
  if (estado_hilo_reconexion == ENABLE) {
    sta_conectandose = false;  // Resetea el estado si no está conectado
  }
}

/*
########################################################################
#
#                      Configuración de SERVIDOR
#
########################################################################
*/

//////////////////////////////////////////////////////////////////////////
//                     Definición de rutas
//////////////////////////////////////////////////////////////////////////

// Definición de todas las rutas del servidor
void server_setting() {
  server.on("/", raiz_get);             // Ruta raíz
  server.on("/ip", mi_ip);              // Ruta para obtener la IP
  server.on("/sta", sta_metodo);        // Ruta para configuración del cliente
  server.on("/sensor", sensor_get);     // Ruta para obtener datos del sensor
  server.on("/accion", accion_method);  // Ruta para ejecutar acciones

  server.onNotFound(no_encontrado);  // Maneja rutas no encontradas

  // Inicializa el servidor
  server.begin();
  // Ninguna ruta debe ir por debajo de server.begin
}

//////////////////////////////////////////////////////////////////////////
//                     RUTA: /   Métodos GET
//////////////////////////////////////////////////////////////////////////

// Manejo de la ruta raíz
void raiz_get() {
  server.send(200, TEXT_PLANO, version);  // Responde con el estado 200 y la versión del servidor
}

//////////////////////////////////////////////////////////////////////////
//                 RUTA: no encontrado   Métodos GET
//////////////////////////////////////////////////////////////////////////

// Manejo de la página no encontrada
void no_encontrado() {
  server.send(404, TEXT_PLANO, WEB_ERROR_RECURSO);  // Responde con error 404
}

//////////////////////////////////////////////////////////////////////////
//                    RUTA: /ip   Métodos GET
//////////////////////////////////////////////////////////////////////////

// Devuelve la IP del dispositivo
void mi_ip() {
  IPAddress clientIP = server.client().remoteIP();  // Obtiene la IP del cliente

  Serial.print("Dispositivo conectado desde la IP: ");  // Mensaje para el monitor serie
  Serial.println(clientIP);                             // Imprime la IP del dispositivo

  server.send(200, TEXT_PLANO, "Mi IP es : " + clientIP.toString());  // Envía la IP al cliente
}

//////////////////////////////////////////////////////////////////////////
//                 RUTA: /sta   Métodos GET y POST
//////////////////////////////////////////////////////////////////////////

// Diferenciación de métodos POST y GET para la ruta /sta
void sta_metodo() {
  if (server.method() == HTTP_GET) {                     // Si el método es GET
    sta_get();                                           // Llama al manejador de GET
  } else if (server.method() == HTTP_POST) {             // Si el método es POST
    sta_post();                                          // Llama al manejador de POST
  } else {                                               // Si es un método diferente
    server.send(400, TEXT_PLANO, WEB_ERROR_PARAMETROS);  // Envía un error de parámetros
  }
}

// Gestor del método GET para la ruta /sta
void sta_get() {
  String jsonResponse;  // String para la respuesta en formato JSON
  String _estado = "";  // Variable para guardar el estado de la red
  String _ip = "";      // Variable para guardar la IP que toma el HOST

  if (WiFi.status() == WL_CONNECTED) {  // Comprueba si se pudo conectar a la red
    _estado = ESTADO_ENCENDIDO;         // Estado de conexión
    _ip = WiFi.localIP().toString();    // IP del HOST
  } else {
    _estado = ESTADO_APAGADO;          // Estado de desconexión
    _ip = WiFi.softAPIP().toString();  // IP del HOST en modo AP
  }

  // Formato JSON de respuesta
  jsonResponse = JS_INICIO + JS_ESTADO + JS_CV + JS + _estado + JS + JS_ADD + JS_IP + JS_CV + JS + _ip + JS + JS_FIN;

  server.send(200, JSON, jsonResponse);  // Envía la respuesta JSON
}

// Gestor del método POST para la ruta /sta
void sta_post() {
  // Comprobar si recibió los argumentos de SSID y PASSWORD
  if (server.hasArg(URL_VAR_SSID) && server.hasArg(URL_VAR_PASSWORD)) {
    String _ssid_sta = server.arg(URL_VAR_SSID);          // Obtener el valor de SSID
    String _password_sta = server.arg(URL_VAR_PASSWORD);  // Obtener el valor de PASSWORD

    ssid_sta = _ssid_sta;          // Actualiza el SSID
    password_sta = _password_sta;  // Actualiza la contraseña

    sta_conectarse_red();  // Conectar a la nueva red

    Serial.println(_ssid_sta + _password_sta);  // Mensaje de estado
    server.send(200, TEXT_PLANO, WEB_EXITO);    // Responde con éxito al cliente
  } else {
    server.send(400, TEXT_PLANO, WEB_ERROR_PARAMETROS);  // Responde con error
  }
}
//////////////////////////////////////////////////////////////////////////
//                 RUTA: /Sensor   Métodos GET
//////////////////////////////////////////////////////////////////////////

// Devuelve los valores de los sensores
void sensor_get() {
  String tipo_sensor;
  if (server.hasArg(URL_VAR_SENSOR)) {
    tipo_sensor = server.arg(URL_VAR_SENSOR);                    // Obtiene el tipo de sensor solicitado
    String _valor = String(detectar_valor_sensor(tipo_sensor));  // Detecta el valor del sensor

    String jsonResponse;
    jsonResponse = JS_INICIO + JS_SENSOR + JS_CV + tipo_sensor +  // Incluye el tipo de sensor
                   JS_ADD + JS_VALOR + JS_CV + _valor +           // Incluye el valor del sensor
                   JS_FIN;

    server.send(200, JSON, jsonResponse);  // Envía la respuesta JSON con el valor del sensor específico
  } else {
    // Responde con todos los valores de los sensores
    String jsonResponse;
    jsonResponse = JS_INICIO + JS_LUZ + JS_CV + JS + String(sensor_ldr_promedio) + JS + JS_ADD + JS_WATER_LEVEL + JS_CV + JS + String(sensor_nivel_agua) + JS + JS_ADD + JS_HUMEDAD_AIRE + JS_CV + JS + String(sensor_humedad_aire) + JS + JS_ADD + JS_HUMEDAD_SUELO + JS_CV + JS + String(sensor_humedad_suelo) + JS + JS_ADD + JS_TEMPERATURA + JS_CV + JS + String(sensor_temperatura) + JS + JS_ADD + JS_LED + JS_CV + JS + String(sensor_led) + JS + JS_ADD + JS_SENSOR + JS_CV + JS + String(sensor_servo) + JS + JS_FIN;

    server.send(200, JSON, jsonResponse);  // Envía todos los valores de los sensores
  }
}

//////////////////////////////////////////////////////////////////////////
//                 RUTA: /ACCION   Métodos GET y POST
//////////////////////////////////////////////////////////////////////////

// Gestor de los métodos POST y GET para la ruta /accion
void accion_method() {
  if (server.method() == HTTP_GET) {
    accion_get();  // Llama al gestor de GET
  } else if (server.method() == HTTP_POST) {
    accion_post();  // Llama al gestor de POST
  } else {
    server.send(404, TEXT_PLANO, "RECURSO NO ENCONTRADO");  // Responde con error 404 si el método no es válido
  }
}

// Métodos GET de la ruta "/accion"
void accion_get() {
  // Verifica si se ha recibido un argumento para el actuador
  if (server.hasArg(URL_VAR_ACTUADOR)) {
    // Obtiene el valor del actuador recibido
    String actuador = server.arg(URL_VAR_ACTUADOR);

    // Detecta el valor actual del sensor correspondiente al actuador
    String _valor = String(detectar_valor_sensor(actuador));

    // Genera una respuesta JSON con el actuador y su valor
    String jsonResponse = json_actuador(actuador, _valor);

    // Envía una respuesta HTTP 200 con el contenido JSON
    server.send(200, JSON, jsonResponse);
  } else {
    // Si no se recibió un actuador, genera una respuesta JSON con todos los estados de los actuadores
    String jsonResponse;
    jsonResponse = JS_INICIO +                                           // Inicio del JSON
                   JS_LED + JS_CV + JS + String(sensor_led) + JS +       // Estado del LED
                   JS_ADD +                                              // Agrega un separador
                   JS_SENSOR + JS_CV + JS + String(sensor_servo) + JS +  // Estado del sensor servo
                   JS_FIN;                                               // Fin del JSON

    // Envía una respuesta HTTP 200 con el contenido JSON
    server.send(200, JSON, jsonResponse);
  }
}

// Métodos POST de la ruta "/accion"
void accion_post() {
  // Verifica si se han recibido argumentos para el actuador y su valor
  if (server.hasArg(URL_VAR_ACTUADOR) && server.hasArg(URL_VAR_ACTUADOR_VALOR)) {
    // Obtiene el actuador y su valor del POST
    String _actuador = server.arg(URL_VAR_ACTUADOR);
    String _valor = server.arg(URL_VAR_ACTUADOR_VALOR);

    // Genera una respuesta JSON para el actuador y su nuevo valor
    String jsonResponse = json_actuador(_actuador, _valor);

    // Solicita mover el actuador al nuevo valor
    solictar_actuadores("MOVE " + _actuador + " ", _valor);

    // Envía una respuesta HTTP 200 con el contenido JSON
    server.send(200, JSON, jsonResponse);
  } else {
    // Si faltan parámetros, envía una respuesta HTTP 400 con un mensaje de error
    server.send(400, TEXT_PLANO, "ERROR POST");
  }
}

// Genera una respuesta JSON para el actuador y su nuevo valor
String json_actuador(String _url_var, String _new_valor) {
  String actuador = _url_var;  // Guarda el actuador recibido
  String _valor = _new_valor;  // Guarda el nuevo valor del actuador
  String _jsonResponse;        // Variable para almacenar la respuesta JSON

  // Construye la respuesta JSON
  _jsonResponse = JS_INICIO +                       // Inicio del JSON
                  JS_ACTUADOR + JS_CV + actuador +  // Agrega el actuador
                  JS_ADD +                          // Agrega un separador
                  JS_VALOR + JS_CV + _valor +       // Agrega el nuevo valor
                  JS_FIN;                           // Fin del JSON

  return _jsonResponse;  // Devuelve la respuesta JSON generada
}

// Detecta el valor del sensor según el tipo proporcionado
int detectar_valor_sensor(String _tipo) {
  // Compara el tipo de sensor y devuelve su valor correspondiente
  if (_tipo == C_LDR) {
    return sensor_ldr_promedio;  // Retorna el valor promedio del LDR
  } else if (_tipo == C_Humedad_aire) {
    return sensor_humedad_aire;  // Retorna el valor de humedad del aire
  } else if (_tipo == C_Humedad_suelo) {
    return sensor_humedad_suelo;  // Retorna el valor de humedad del suelo
  } else if (_tipo == C_Temperatura) {
    return sensor_temperatura;  // Retorna el valor de temperatura
  } else if (_tipo == C_Nivel_agua) {
    return sensor_nivel_agua;  // Retorna el valor del nivel de agua
  } else if (_tipo == C_servo) {
    return sensor_servo;  // Retorna el valor del servo
  } else if (_tipo == C_LED) {
    return sensor_led;  // Retorna el estado del LED
  } else {
    return -1;  // Retorna -1 si el tipo no es reconocido
  }
}

/*
##################################################################################
#
#               COMUNICACION ENTRE ARDUINO Y ESP
#
###################################################################################
*/

// Función principal para manejar la comunicación entre el Arduino y el ESP
void loop_sistema_comunicacion() {
  leer_serial_esp();      // Llama a la función para leer datos del ESP
  leer_serial_arduino();  // Llama a la función para leer datos del Arduino
}

// Lectura de monitor serial físico
void leer_serial_arduino() {
  // Verifica si hay datos disponibles en el puerto serial del Arduino
  if (Serial.available()) {
    // Lee un mensaje completo hasta el salto de línea
    mandar_mensaje = Serial.readStringUntil('\n');

    if (mandar_mensaje == COMUNICACION_SERIAL_STOP) {
      comunicacion_enable = false;
      Serial.println("APAGANDO COMUNICACION SERIAL");

    } else if (mandar_mensaje == COMUNICACION_SERIAL_START) {
      comunicacion_enable = true;
      Serial.println("ENCENDER COMUNICACION SERIAL");

    } else if (mandar_mensaje == WIFI_CONECTAR) {
      ap_setting();
      sta_conectarse_red();
      wifi_conection_enable = true;
      Serial.println("Encendiendo redes");

    } else if (mandar_mensaje == WIFI_DESCONECTAR) {
      WiFi.disconnect(true);
      WiFi.softAPdisconnect(true);
      wifi_conection_enable = false;
      Serial.println("Apagando redes");


    } else {
      // Envía el mensaje leído al puerto serial del ESP
      softwareSerial.println(mandar_mensaje);

      // Imprime en consola el mensaje que se envió al Arduino
      Serial.println("mensaje enviado al arduino: " + mandar_mensaje);
    }
  }
}

// Lectura de monitor serial que está conectado al Arduino
void leer_serial_esp() {
  // Verifica si hay datos disponibles en el puerto serial del ESP
  if (softwareSerial.available()) {  // Lee si hay un dato por serial
    // Lee un mensaje completo hasta el salto de línea
    mensaje_arduino = softwareSerial.readStringUntil('\n');

    // Guarda el mensaje leído en la variable 'comando'
    comando = mensaje_arduino;

    // Imprime en consola el mensaje recibido del ESP
    Serial.println("mensaje recibido: " + mensaje_arduino);

    // Llama a la función para procesar los datos recibidos
    obtencion_data();
  }
}

// Corte de datos recibidos por serial
String cortar_comando(String dato) {
  // Encuentra la posición del primer espacio en el dato
  byte _inicio_valor = dato.indexOf(' ');
  // Encuentra la posición del segundo espacio
  byte _fin_valor = dato.indexOf(' ', _inicio_valor + 1);

  // Extrae el primer comando hasta el primer espacio
  comando_objetivo = dato.substring(0, _inicio_valor);
  // Extrae el valor del comando entre los dos espacios
  comando_valor = dato.substring(_inicio_valor + 1, _fin_valor);
  // Retorna el resto del comando después del segundo espacio
  return comando = dato.substring(_fin_valor + 1);
}

// Comprueba si es un número y devuelve su valor entero
int comprobar_valor() {
  return comando_valor.toInt();  // Convierte el comando_valor a entero
}

// Compara los datos obtenidos para actualizar los valores de los sensores
void interpretacion_objetivo() {
  // Compara el comando objetivo y asigna el valor correspondiente a cada sensor
  if (comando_objetivo == C_LDR0) {
    sensor_ldr0 = comprobar_valor();  // Actualiza sensor LDR0
  } else if (comando_objetivo == C_LDR1) {
    sensor_ldr1 = comprobar_valor();  // Actualiza sensor LDR1
  } else if (comando_objetivo == C_LDR2) {
    sensor_ldr2 = comprobar_valor();  // Actualiza sensor LDR2
  } else if (comando_objetivo == C_Humedad_aire) {
    sensor_humedad_aire = comprobar_valor();  // Actualiza sensor de humedad del aire
  } else if (comando_objetivo == C_Humedad_suelo) {
    sensor_humedad_suelo = comprobar_valor();  // Actualiza sensor de humedad del suelo
  } else if (comando_objetivo == C_Temperatura) {
    sensor_temperatura = comprobar_valor();  // Actualiza sensor de temperatura
  } else if (comando_objetivo == C_Nivel_agua) {
    sensor_nivel_agua = comprobar_valor();  // Actualiza sensor de nivel de agua
  } else if (comando_objetivo == C_LED) {
    sensor_led = comprobar_valor();  // Actualiza estado del LED
  } else if (comando_objetivo == C_servo) {
    sensor_servo = comprobar_valor();  // Actualiza estado del servo
  } else {
    Serial.println("ERROR");  // Imprime un mensaje de error si el comando no es reconocido
  }
}

// Gestiona los errores recibidos
void interpretacion_errores_serial(char _data) {
  // Compara el dato recibido y muestra un mensaje correspondiente
  if (_data == SERIAL_EXITO) {
    Serial.println("COMANDO EJECUTADO CON EXITO");  // Indica éxito en la ejecución del comando
  } else if (_data == SERIAL_SENSORES) {
    Serial.println("DATO RECIBIDO");  // Indica que se recibió un dato de los sensores
  } else {
    //Serial.println(mensaje_arduino); // (opcional) Descomentar para mostrar mensaje en caso de error
  }
}

// Corte de todos los comandos
void obtencion_data() {

  if (!isDigit(comando[0])) {  // Verifica que el primer carácter no sea un dígito

    while (comando.length() >= 3) {  // Mientras la longitud del comando sea mayor o igual a 3

      comando = cortar_comando(comando);  // Corta el comando y extrae el primer objetivo y el primer valor
      interpretacion_objetivo();          // Procesa el objetivo obtenido
    }
  }
}

// SOLICITA DATOS
void solictar_sensores(String _comando) {

  softwareSerial.println(_comando);                // Envía el comando al ESP
  Serial.println("Comando enviado: " + _comando);  // Imprime en consola el comando enviado
}

// SOLICITUD DE EJECUCIÓN
void solictar_actuadores(String _actuadores, String _valor) {

  softwareSerial.println(_actuadores + _valor);                // Envía el comando de actuación junto con el valor al ESP
  Serial.println("Comando enviado: " + _actuadores + _valor);  // Imprime en consola el comando enviado
}

/*
###################################################################################################
#
#             CONTROL Y MONITOREO DE LA PLANTA 
#
###################################################################################################
*/

// Aquí todo lo que se debe de ejecutar lo más rápido posible
void loop_monitoreo_control() {
  // Este espacio puede ser usado para monitoreo continuo
}

// Aquí todo lo que se debe de ejecutar cada 5 segundos
void loop_control_lento() {
  //Serial.println(millis()); // (opcional) Descomentar para ver el tiempo en milisegundos
}

// Aquí todo lo que se debe de ejecutar cada 10 segundos
void loop_control_super_lento() {
  actualizar_serial();  // Llama a la función para actualizar el estado de los sensores
}

// Actualiza el estado de los sensores y calcula el promedio del LDR
void actualizar_serial() {
  if (comunicacion_enable == true) {
    solictar_sensores(LEER_SENSORES);  // Solicita la lectura de los sensores
    promedio_ldr();                    // Calcula el promedio de los valores del LDR
  }
}

// Calcula el promedio de los valores del LDR
void promedio_ldr() {
  // Calcula el promedio de los sensores LDR y lo almacena en sensor_ldr_promedio
  sensor_ldr_promedio = int((sensor_ldr0 + sensor_ldr1 + sensor_ldr2) / 3);
}
