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
#include "ESPAsyncWebServer.h"
#include "DHT.h"



//Configuración de datos
const unsigned int TAM_COLA = 25;
int colaRuido[TAM_COLA];
////int colaMovimiento[TAM_COLA];
int colaMagnetico[TAM_COLA];
float colaTemperatura[TAM_COLA];
const unsigned int N_DATOS = 2000;
int dato = 1;
int nDatos;

//Configuración
const long PAUSA = 500;
const unsigned int BAUD_RATE = 115200;
noDelay pausa(PAUSA);

//Configuración de conexión a internet
const char* ssid = "IoT_ITSON";  //MEGACABLE-2E9F   IoT_ITSON    MEGACABLE-319E
const char* password = "lv323-iot";    //Uu5raDYY           lv323-iot   pGbKefc9
AsyncWebServer server(80);

//COnfiguración de tiempo
WiFiUDP ntpUDP;

//Metodos a usar
void actualizaLectura();
int obtenRuido();
////int obtenMovimiento();
int obtenMagnetico();
float obtenTemperatura();
int obtenEstado();
void revisarAlarma();
void activarAlarma();
void desactivarAlarma();

//Medidas de sensores
int ruido = 0;
int ruidoMaximo = 0;
int ruidoMinimo = 4095;
int ruidoPromedio = 0;

//// int movimiento = 0;
//// int movimientoMaximo = 0;
//// int movimientoMinimo = 4095;
//// int movimientoPromedio = 0;


int magnetico = 0;
int magneticoMaximo = 0;
int magneticoMinimo = 4095;
int magneticoPromedio = 0;

float temperatura = 0.0;
float temperaturaMaximo = -1000;
float temperaturaMinimo = 1000;
float temperaturaPromedio = 0.0;

int estado = 0;  // 0=Desactivada, 1=Activada

//Parametros ajustables (1-10)
//Sensores
int sensibilidadRuido = 10;
////int sensibilidadMovimiento = 5;
float sensibilidadTemperatura = 34.0;

//Actuadores
int volumenBuzzer = 15;
int brilloLuz = 5;

//Puertos
// Sensor DHT
#define DHTPIN 4       // Pin del sensor
#define DHTTYPE DHT11  // Tipo de sensor
DHT dht(DHTPIN, DHTTYPE);
////const unsigned int sensorMovimiento = 4;
const unsigned int SensorMagnetico = 35;
const unsigned int sensorRuido = 34;
const unsigned int buzzer = 25;
const unsigned int luz = 33;

String comandoSerial = "";

void setup() {
  Serial.begin(BAUD_RATE);
  Serial.println("Iniciando");
  delay(100);

  ////pinMode(sensorMovimiento, INPUT);
  //Se cambio, por eso daba valores raros
  pinMode(SensorMagnetico, INPUT_PULLUP);
  pinMode(sensorRuido, INPUT);

  pinMode(buzzer, OUTPUT);
  pinMode(luz, OUTPUT);

  Serial.println("Conectandose a internet");
  conectaRedWiFi(ssid, password);

  Serial.println("Iniciando sensor DHT");
  dht.begin();

  Serial.println("Inicializando archivos internos");
  inicializaLittleFS();

  Serial.println("Configurando servidor");
  configuraServidor();

  Serial.println("Configuración inicializado");
}

void loop() {
  if (pausa.update()) {
    if (dato <= N_DATOS) {

      //Lee los sensores
      actualizaLectura();


      //Activa la alarma dependiendo de los datos sensados
      revisarAlarma();

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

  ruido = obtenRuido();
  Serial.println("Ruido crudo: " + String(ruido));
  nDatos = insertaCola(colaRuido, ruido, TAM_COLA);
  ruidoPromedio = obtenPromedioMovil(colaRuido, TAM_COLA);
  if (ruido > ruidoMaximo) {
    ruidoMaximo = ruido;
  }
  if (ruido < ruidoMinimo) {
    ruidoMinimo = ruido;
  }

  //// movimiento = obtenMovimiento();
  //// nDatos = insertaCola(colaMovimiento, movimiento, TAM_COLA);
  //// movimientoPromedio = obtenPromedioMovil(colaMovimiento, TAM_COLA);
  //// if (movimiento > movimientoMaximo) movimientoMaximo = movimiento;
  //// if (movimiento < movimientoMinimo) movimientoMinimo = movimiento;

  magnetico = obtenMagnetico();
  nDatos = insertaCola(colaMagnetico, magnetico, TAM_COLA);
  magneticoPromedio = obtenPromedioMovil(colaMagnetico, TAM_COLA);
  if (magnetico > magneticoMaximo) {
    magneticoMaximo = magnetico;
  }
  if (magnetico < magneticoMinimo) {
    magneticoMinimo = magnetico;
  }

  temperatura = obtenTemperatura();
  Serial.println("Temperatura: " + String(temperatura));
  nDatos = insertaColaFloat(colaTemperatura, temperatura, TAM_COLA);
  temperaturaPromedio = obtenPromedioMovilFloat(colaTemperatura, TAM_COLA);
  if (temperatura > temperaturaMaximo) temperaturaMaximo = temperatura;
  if (temperatura < temperaturaMinimo) temperaturaMinimo = temperatura;
}

/*
* Esta funcion obtiene el ruido del sensor
*/
int obtenRuido() {
  return analogRead(sensorRuido);
}

float obtenTemperatura() {
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println("Fallo al leer la temperatura");
    return temperatura;
  }
  return t;
}

/*
* Esta funcion obtiene el movimiento del sensor
*/
////int obtenMovimiento() {
////  return digitalRead(sensorMovimiento);
////}

/*
* Esta funcion obtiene el magnetismo del sensor
*/
int obtenMagnetico() {
  return digitalRead(SensorMagnetico);
}

/*
* Revisar si la alarma se debe activar
*/
void revisarAlarma() {
  //Si ya esta encendida la alarma, no hacer nada
  /*
  if (estado == 1) {
    return;
  }*/
  //Comparar si los sensores cumplen la condición para activar la alarma
  if (ruido >= 2000) {
    activarAlarma();
  }
   else if (temperatura >= sensibilidadTemperatura) {
    activarAlarma();
  }
  //else
  //if ((sensibilidadMovimiento/5) * movimiento < 2000) {
  //  activarAlarma();
  //} else
  else if (magnetico < 1) {
    activarAlarma();
  } else {
    desactivarAlarma();
  }
}

/*
* Activa la alarma
*/
void activarAlarma() {
  if (estado == 1) {
    return;
  }
  estado = 1;
  //Activar el buzzer y luz
  analogWrite(buzzer, volumenBuzzer * 25);
  analogWrite(luz, brilloLuz * 25);
  Serial.println("Alarma activada");
}

/*
* Desactiva la alarma
*/
void desactivarAlarma() {
  if (estado == 0) {
    return;
  }
  estado = 0;
  //Desactivar el buzzer y luz
  analogWrite(buzzer, 0);
  analogWrite(luz, 0);
  Serial.println("Alarma desactivada");
}

/*
* Leer comandos por Serial
*/
void leerComando() {
  if (Serial.available()) {
    comandoSerial = Serial.readStringUntil('\n');
    comandoSerial.trim();

    if (comandoSerial.equalsIgnoreCase("on")) {
      activarAlarma();
    } else if (comandoSerial.equalsIgnoreCase("off")) {
      desactivarAlarma();
    } 
    ////else if (comandoSerial.startsWith("mov ")) {
    ////  int val = comandoSerial.substring(4).toInt();
    ////  if (val >= 0 && val <= 10) {
    ////    sensibilidadMovimiento = val;
    ////    Serial.printf("Sensibilidad Movimiento ajustada a %d\n", val);
    ////  }
    ////} 
    else if (comandoSerial.startsWith("rui ")) {
      int val = comandoSerial.substring(4).toInt();
      if (val >= 0 && val <= 10) {
        sensibilidadRuido = val;
        Serial.printf("Sensibilidad Ruido ajustada a %d\n", val);
      }
    }else if (comandoSerial.startsWith("temp ")) {
      float val = comandoSerial.substring(5).toFloat();
      if (val >= 0.0 && val <= 100.0) {
        sensibilidadTemperatura = val;
        Serial.printf("Sensibilidad Temperatura ajustada a %.2f\n", val);
      } 
    }
    else if (comandoSerial.startsWith("luz ")) {
      int val = comandoSerial.substring(4).toInt();
      if (val >= 0 && val <= 10) {
        brilloLuz = val;
        Serial.printf("Brillo Luz ajustado a %d\n", val);
      }
    } else if (comandoSerial.startsWith("vol ")) {
      int val = comandoSerial.substring(4).toInt();
      if (val >= 0 && val <= 10) {
        volumenBuzzer = val;
        Serial.printf("Volumen Buzzer ajustado a %d\n", val);
      }
    } else {
      Serial.println("Comando no reconocido");
    }
  }
}

/*
* Envia los datos a un servidor local mysql
*/
void mandarDatos() {
  Serial.println("MYSQL ESTADO: " + String(estado) + 
  " RUIDO: " + String(ruido) + 
  ////" MOVIMIENTO: " + String(movimiento) + 
  " MAGNETICO: " +  String(magnetico) +
  " TEMPERATURA: " + String(temperatura));
}