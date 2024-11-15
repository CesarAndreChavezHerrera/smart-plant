/* MATENER LAS COSAS SIMPLES */
/*
########################################################################
##                DEfinicio de comandos de comunicacion serial
########################################################################
*/

// COMANDO ERIC
// COMANDOS            
#define C_LECTURA            "GET"
#define C_ACTUADOR           "MOVE"

// sensores
#define C_TODO               "TODO" 
#define C_LDR                "LDR" 
#define C_Humedad_aire       "HUM_AIRE"
#define C_Temperatura        "TEMP"
#define C_Humedad_suelo      "HUM_PISO"
#define C_Nivel_agua         "WL"

//Actuadores
#define C_servo              "SERVO"
#define C_LED                "LED"


// errores 
#define ERROR_0               0
#define ERROR_COMANDO         1
#define ERROR_OBJETIVO        2
#define ERROR_VAlOR           3
#define ERROR_VAlOR_NO_VALIDO 4
#define ERROR_DESCONOCIDO     5
#define ERROR_0_SENSORES      6

// Respuestas
#define R_EXITO                 "0 LISTO"
#define R_ERROR_COMANDO         "1 COMANDO NO ENCONTRADO"
#define R_ERROR_OBJETIVO        "2 OBJETIVO NO ENCONTRADO"
#define R_ERROR_VALOR           "3 VALOR NO ENCONTRADO"
#define R_ERROR_VALOR_NO_VALIDO "4 ERROR AL LEER EL VALOR"
#define R_ERROR_DESCONOCIDO     "5 ERROR DESCONOCIDO"
#define R_SENSORES              "6 SENSORES " 

/*
#####################################################################
#                                 sensores
#####################################################################
*/


//////////////////////////////////////////////////////////////////// 
//                            LDR
#define LDR0 A0
#define LDR1 A1
#define LDR2 A2
#define LDR_MAX 100
#define LDR_MIN 0
#define lDR_V_MAX 1023
#define LDR_V_MIN 0
int ldr_v0 = 0;
int ldr_v1 = 0;
int ldr_v2 = 0;

/////////////////////////////////////////////////////////////////////
//                       DHT 11 sensor 
#include <DHT11.h>
#define dht_pin 4
DHT11 dht11(dht_pin);

int humedad_aire = 0;
int temperatura = 0;

///////////////////////////////////////////////////////////////////////
//                           FC 28
#define fc_28_pin A4
int humedad_suelo = 0;



////////////////////////////////////////////////////////////////////////////
//                          sensor de agua
#define power_sensor 5
#define water_sensor_pin A5 
int water_nivel = 0;

///////////////////////////////////////////////////////////////////////////
//                          software serial
#include <SoftwareSerial.h>
#define RxEsp 3
#define TxEsp 2
SoftwareSerial serialEsp(RxEsp,TxEsp);

String comando_serial = "MOVE servo 180";
String comando = "GET";
String objetivo = "SERVO";
String valor = "180";
String Respuesta = "";

//////////////////////////////////////////////////////////////////////////////
//                           servo acciones
#include <Servo.h>
#define servoPin 6
Servo myservo;
int myservo_grado = 180;
//////////////////////////////////////////////////////////////////////////////////////
#define led_pin 8
int led_valor = 0; 
/*
######################################################################################
#                                                                                    #
#                              Parte PRINCIPAL                                       #
#                                                                                    #
######################################################################################
*/
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  serialEsp.begin(9600);
  Serial.println("conectado");
  comando_serial = "";
  pinMode(dht_pin,INPUT);
  pinMode(led_pin,OUTPUT);
  myservo.attach(servoPin);
  myservo.write(180);
}

void loop() {
  
  leer_serial_esp();
  leer_serial();
  
}



/*
#############################################################################################
#                                COMUNICACION SERIAL                                        #
#############################################################################################
*/

///////////////////////////////
// gestiona que ACCION REALIZAR
/////////////////////////////////

void control_lectura(){
  //Serial.println(valor);
  if (comando == C_LECTURA){
    
    if(objetivo == C_LDR){
      escribir(control_ldr());
    
    }else if (objetivo == C_Humedad_suelo){
      escribir(control_humedad_suelo());
    
    }else if (objetivo == C_Nivel_agua){

      escribir(control_nivel_agua());
    
    }else if (objetivo == C_Temperatura){
      escribir(control_temperatura());
    
    }else if(objetivo == C_Humedad_aire){
      escribir(control_humedad_aire());

    }else if(objetivo == C_LED){
      escribir(control_leer_led());
    
    }else if(objetivo == C_servo){
      escribir(control_leer_servo());
    
    }else if(objetivo == C_TODO){
      escribir(controL_leer_sensores());
    

    }else{
      CONTROL_ERROR_LECTURA(ERROR_OBJETIVO);
      return;
    }
  
  }else if (comando == C_ACTUADOR){
    
    if (valor != NULL && valor != ' ' && valor != '\n' && valor != "" ){
      if (objetivo == C_servo){
        control_servo();
      }else if (objetivo == C_LED){
        control_led();
      }
      else {
        CONTROL_ERROR_LECTURA(ERROR_OBJETIVO);
      }
    }else {
      CONTROL_ERROR_LECTURA(ERROR_VAlOR_NO_VALIDO);
      }

  }else{
    CONTROL_ERROR_LECTURA(ERROR_COMANDO);
  }
}


/////////////////////////////
// lee la comunicacion serial
//////////////////////////////
void leer_serial(){

  if(Serial.available()> 0){

    char dato = Serial.read();
    
    if (dato == '\n'){
      serialEsp.println(comando_serial);
      Serial.println("MENSAJE ENVIADO: " + comando_serial);
      corte_comando(comando_serial);
      control_lectura();

      comando_serial = "";
    }else{
      comando_serial += dato;
    }
  }
}

void leer_serial_esp(){
  if(serialEsp.available()){
     
     char dato = serialEsp.read();
    if (dato == '\n'){
      //Serial.println(comando_serial);
      Serial.println("COMANDO RECIBIDO: " + comando_serial);
      corte_comando(comando_serial);
      control_lectura();

      comando_serial = "";
    }else{
      comando_serial += dato;
    }
  }
}

/////////////////////////////////////
// corta en parte el comando recibido
/////////////////////////////////////
void corte_comando(String dato){

  byte inicio_objetivo = dato.indexOf(' ');
  byte inicio_valor = dato.indexOf(' ',inicio_objetivo + 1);

  comando = dato.substring(0,inicio_objetivo);
  objetivo = dato.substring(inicio_objetivo + 1 , inicio_valor );
  valor = dato.substring(inicio_valor + 1 );

}




///////////////////////////////////////
// SISTEMA DE MANEJO DE ERROR
////////////////////////////////////////

void CONTROL_ERROR_LECTURA(byte codigo_error){
  
  switch(codigo_error){
    case ERROR_0:
      escribir(R_EXITO);
      break;
    case ERROR_COMANDO:
      escribir(R_ERROR_COMANDO);
      break;
    case ERROR_OBJETIVO:
      escribir(R_ERROR_OBJETIVO);
      break;
    case ERROR_VAlOR:
      escribir(R_ERROR_VALOR);
      break;
    case ERROR_VAlOR_NO_VALIDO:
      escribir(R_ERROR_VALOR_NO_VALIDO);
      break;
    case ERROR_0_SENSORES:
      escribir(R_SENSORES);
      break;

    default:
      escribir(R_ERROR_DESCONOCIDO);
      break;
  }
}

void escribir(String texto){
  Serial.println("COMANDO ENVIADO AL ESP: "+ texto);
  serialEsp.println(texto);
}



/*
#############################################################################################
#                                 SENSORES
#############################################################################################
*/



////////////////////////////////////////////////////////
//                     LDR

String control_ldr(){
  ldr_v0 = analogRead(LDR0);
  ldr_v1 = analogRead(LDR1);
  ldr_v2 = analogRead(LDR2);
  
  ldr_v0 = mapeo_ldr(ldr_v0);
  ldr_v1 = mapeo_ldr(ldr_v1);
  ldr_v2 = mapeo_ldr(ldr_v2);
  return "LDR0 "+String(ldr_v0) + " LDR1 "+String(ldr_v1)+ " LDR2 "+String(ldr_v2);
}

byte mapeo_ldr(int ldr){
  return map(ldr, ///               Variable
            LDR_V_MIN,lDR_V_MAX, // valores minimo y maximo de la variable
            LDR_MIN,LDR_MAX      // valores en porcentaje
            );
}


////////////////////////////////////////////////////////
//                     DH 11          
String control_humedad_aire(){
  humedad_aire = dht11.readHumidity();
  String _aire = C_Humedad_aire;
  return _aire +" " +String(humedad_aire);
}
String control_temperatura(){
  temperatura = dht11.readTemperature();
  String _temperatura = C_Temperatura;
  return _temperatura+' '+String(temperatura);
}

//////////////////////////////////////////////////////////
//           SENSORES NIVEL AGUAAAA

String control_nivel_agua(){
  water_nivel = analogRead(water_sensor_pin);
  String _nl = C_Nivel_agua;
  return _nl + " "+water_sensor_pin;
}

/////////////////////////////////////////////////////////
//           Humedad de suelo

String control_humedad_suelo(){
  humedad_suelo = analogRead(fc_28_pin);
  String _nl = C_Humedad_suelo;
  return _nl + " "+humedad_suelo;
}
////////////////////////////////////////////////////////
//         Devuelve el estado del led
String control_leer_led(){
  String _led = C_LED;
  return _led + " "+ led_valor;
}

/////////////////////////////////////////////////////////
String control_leer_servo(){
  String _servo = C_servo;
  return _servo + " " + myservo_grado;
}

///////////////////////////////////////////////////////////
//          LEER TODO LOS SENSORES
String controL_leer_sensores(){
  return control_ldr()          +" "+ 
         control_humedad_aire() +" " +
         control_temperatura()  +" " + 
         control_nivel_agua()   +" " + 
         control_humedad_suelo()+" "+
         control_leer_led()     +" "+
         control_leer_servo();
}

/*
############################################################################
#          ACTUADORESSSSS
############################################################################
*/

int get_valor(){
  //Serial.println(valor[0]);
  for (int i = 0 ; i < valor.length()-1; i++){
    char a =  valor[i];
    if (!isDigit(a)){
      return 0;
    }
  }
  return valor.toInt();
}

void control_servo(){
  myservo.write(get_valor());
  myservo_grado = get_valor();
  escribir(R_EXITO);
}

void control_led(){
  if (get_valor() != 0){
    digitalWrite(led_pin,HIGH);
    led_valor = HIGH;

  }else{
    digitalWrite(led_pin,LOW);
    led_valor = LOW;
  }

  escribir(R_EXITO);
}
