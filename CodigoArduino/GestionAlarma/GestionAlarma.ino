/*
 * GestionAlarma.ino
 *
 * Modulo principal encargado de hacer de conexión con otros modulos
 */
#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <Wire.h>
#include <NoDelay.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "GestionDatos.h"
#include "GestionRed.h"
#include "GestionPeticiones.h"
#include "ESPAsyncWebServer.h"

//Configuración de datos
const unsigned int TAM_COLA = 25;
int colaRuido[TAM_COLA];
int colaMovimiento[TAM_COLA];
int colaMagnetico[TAM_COLA];
const unsigned int N_DATOS = 2000;
int dato = 1;
int nDatos;

//Configuración
const long PAUSA = 1000;
const unsigned int BAUD_RATE = 115200;
noDelay pausa(PAUSA);

//Configuración de conexión a internet
const char* ssid = "MEGACABLE-2E9F";
const char* password = "Uu5raDYY";
AsyncWebServer server(80);

//COnfiguración de tiempo
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -21600, 60000);  // UTC-6 para Sonora, México

//Metodos a usar
void actualizaLectura();
int obtenRuido();
int obtenMovimiento();
int obtenMagnetico();
int obtenEstado();
void revisarAlarma();
void activarAlarma();
void desactivarAlarma();

//Medidas de sensores
int ruido=0;
int ruidoMaximo=0;
int ruidoMinimo=100;
int ruidoPromedio=0;

int movimiento=0;
int movimientoMaximo=0;
int movimientoMinimo=100;
int movimientoPromedio=0;

int magnetico=0;
int magneticoMaximo=0;
int magneticoMinimo=100;
int magneticoPromedio=0;

int estado=0; // 0=Desactivada, 1=Activada

//Parametros ajustables (1-10)
//Sensores
int sensibilidadRuido=5;  
int sensibilidadMovimiento=5;
int sensibilidadMagnetico=5;
//Actuadores
int volumenBuzzer=5;  
int brilloLuz=5;

//Puertos
const unsigned int sensorMovimiento = 0;
const unsigned int SensorMagnetico = 0;
const unsigned int sensorRuido = 0;
const unsigned int buzzer = 0;
const unsigned int luz = 0;

String comandoSerial = "";

void setup() {
  Serial.println("Iniciando");
  Serial.begin(BAUD_RATE);
  delay(100);

  Serial.println("Conectandose a internet");
  conectaRedWiFi(ssid, password);

  Serial.println("Inicializando archivos internos");
  inicializaLittleFS();

  Serial.println("Configurando servidor");
  configuraServidor();

  Serial.println("Inicializando el cliente para el tiempo");
  timeClient.begin();

  Serial.println("Servidor web inicializado");
}

void loop() {
  if (pausa.update()) {
    if (dato <= N_DATOS) {

      // Actualiza la hora NTP
      timeClient.update(); 
      String hora = timeClient.getFormattedTime();
      time_t epochTime = timeClient.getEpochTime();
      struct tm *ptm = gmtime((time_t *)&epochTime);
      int dia = ptm->tm_mday;
      int mes = ptm->tm_mon + 1;
      int año = ptm->tm_year + 1900;

      //Lee lo sensores
      actualizaLectura();

      //Activa la alarma dependiendo de los datos sensados
      revisarAlarma();

      //Impresión de datos Serial
      nDatos = insertaCola(colaRuido, ruido, TAM_COLA);
      nDatos = insertaCola(colaMovimiento, movimiento, TAM_COLA);
      nDatos = insertaCola(colaMagnetico, magnetico, TAM_COLA);

      Serial.printf("[%02d/%02d/%d %s] Estado de la alarma: %d\n", dia, mes, año, hora.c_str(), estado);

      despliegaCola(colaRuido, nDatos);
      despliegaCola(colaMovimiento, nDatos);
      despliegaCola(colaMagnetico, nDatos);

      float promedioMovRuido = obtenPromedioMovil(colaRuido, nDatos);
      float promedioMovMovimiento = obtenPromedioMovil(colaMovimiento, nDatos);
      float promedioMovMagnetico = obtenPromedioMovil(colaMagnetico, nDatos);

      Serial.print("Promedio movil Ruido: ");
      Serial.println(promedioMovRuido);
      Serial.print("Promedio movil Movimiento: ");
      Serial.println(promedioMovMovimiento);
      Serial.print("Promedio movil Magnetico: ");
      Serial.println(promedioMovMagnetico); 
      
      dato++;
      mandarDatos();
    }
  }
  leerComando();
}

/*
* Esta funcion llama a las funciones de recolección de datos
*/
void actualizaLectura() {

  ruido=obtenRuido();
  ruidoPromedio = obtenPromedioMovil(colaRuido, TAM_COLA); 
  if (ruido > ruidoMaximo) {
    ruidoMaximo = ruido;
  }
  if (ruido < ruidoMinimo) {
    ruidoMinimo = ruido;
  }

  movimiento=obtenMovimiento();
  movimientoPromedio = obtenPromedioMovil(colaMovimiento, TAM_COLA); 
  if (movimiento > movimientoMaximo) {
    movimientoMaximo = movimiento;
  }
  if (movimiento < movimientoMinimo) {
    movimientoMinimo = movimiento;
  }

  magnetico=obtenMagnetico();
  magneticoPromedio = obtenPromedioMovil(colaMagnetico, TAM_COLA); 
  if (magnetico > magneticoMaximo) {
    magneticoMaximo = magnetico;
  }
  if (magnetico < magneticoMinimo) {
    magneticoMinimo = magnetico;
  }
}

/*
* Esta funcion obtiene el ruido del sensor
*/
int obtenRuido() {
}

/*
* Esta funcion obtiene el movimiento del sensor
*/
int obtenMovimiento() {
}

/*
* Esta funcion obtiene el magnetismo del sensor
*/
int obtenMagnetico() {
}

/*
* Revisar si la alarma se debe activar
*/
void revisarAlarma(){
  //Si ya esta encendida la alarma, no hacer nada
  if(estado==1){
    return;
  }
  //Comparar si los sensores cumplen la condición para activar la alarma
  if(sensibilidadRuido*ruido>100){
    activarAlarma();
  }else if(sensibilidadMovimiento*movimiento>100){
    activarAlarma();
  }else if(sensibilidadMagnetico*magnetico>100){
    activarAlarma();
  }
}

/*
* Activa la alarma
*/
void activarAlarma(){
  if(estado==1){
    return;
  }
  estado=1;
  //Activar el buzzer
  //Activar la luz
  Serial.println("Alarma activada");
}

/*
* Desactiva la alarma
*/
void desactivarAlarma(){
  if(estado==0){
    return;
  }
  estado=0;
  //Desactivar el buzzer
  //Desactivar la luz
  Serial.println("Alarma desactivada");
}

/*
* Leer comandos por Serial
*/
void leerComando(){
  if (Serial.available()) {
    comandoSerial = Serial.readStringUntil('\n');
    comandoSerial.trim();

    if (comandoSerial.equalsIgnoreCase("on")) {
      activarAlarma();
    }
    else if (comandoSerial.equalsIgnoreCase("off")) {
      desactivarAlarma();
    }
    else if (comandoSerial.startsWith("mag ")) {
      int val = comandoSerial.substring(4).toInt();
      if (val >= 0 && val <= 10) {
        sensibilidadMagnetico = val;
        Serial.printf("Sensibilidad Magnetico ajustada a %d\n", val);
      }
    }
    else if (comandoSerial.startsWith("mov ")) {
      int val = comandoSerial.substring(4).toInt();
      if (val >= 0 && val <= 10) {
        sensibilidadMovimiento = val;
        Serial.printf("Sensibilidad Movimiento ajustada a %d\n", val);
      }
    }
    else if (comandoSerial.startsWith("rui ")) {
      int val = comandoSerial.substring(4).toInt();
      if (val >= 0 && val <= 10) {
        sensibilidadRuido = val;
        Serial.printf("Sensibilidad Ruido ajustada a %d\n", val);
      }
    }
    else if (comandoSerial.startsWith("luz ")) {
      int val = comandoSerial.substring(4).toInt();
      if (val >= 0 && val <= 10) {
        brilloLuz = val;
        Serial.printf("Brillo Luz ajustado a %d\n", val);
      }
    }
    else if (comandoSerial.startsWith("vol ")) {
      int val = comandoSerial.substring(4).toInt();
      if (val >= 0 && val <= 10) {
        volumenBuzzer = val;
        Serial.printf("Volumen Buzzer ajustado a %d\n", val);
      }
    }
    else {
      Serial.println("Comando no reconocido");
    }
  }
}

/*
* Envia los datos a un servidor local mysql
*/
void mandarDatos(){
   Serial.println("MYSQL ESTADO: " + String(estado) + 
               " RUIDO: " + String(ruido) + 
               " MOVIMIENTO: " + String(movimiento) + 
               " MAGNETICO: " + String(magnetico));

}