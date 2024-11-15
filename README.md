# smart-plant

## Codigo arduino 
la carpeta "smart_plant" es la que tiene el codigo para el arduino nano.

Es el encargado de controlar y monitoriar TODO LOS SENSORES.


## Codigo ESP8266
la carpeta "esp_smart_plant" es la que contiene el codigo para el ship "ESP8266".

1. Es el encargado de gestionar y procesar la informacion de los sensores.
2. ES el encargado de proporcionar un servidor api para mandar la informacion.

recordatorio añadir las placas esp8266 al arduino ide revisar el [siguiente tutorial](https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/#:~:text=To%20install%20the%20ESP8266%20board%20in%20your%20Arduino,by%20ESP8266%20Community%20%E2%80%9C%3A%205%20That%E2%80%99s%20it.%20)

---
# cosas que hacer 

## arduino
1. NADA , ESTA LISTO.

## ESP8266
1. Añadir la logica de control automatico.
   Que cuando ciertos valores de los sensores lleguen a un punto encienda la bomba.
3. Que la bomba se encienda y apague luego de x cantidad de tiempo.
4. Añadir una ruta de la api que reciba metodos post para ajustar los eventos de control automatico.
