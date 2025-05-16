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
const unsigned int N_DATOS = 10000;
int dato = 1;
int nDatos;

//Configuración
const long PAUSA = 250;
const unsigned int BAUD_RATE = 115200;
noDelay pausa(PAUSA);

//Configuración de conexión a internet
const char* ssid = "IoT_ITSON";      //MEGACABLE-2E9F   IoT_ITSON    MEGACABLE-319E
const char* password = "lv323-iot";  //Uu5raDYY           lv323-iot   pGbKefc9
AsyncWebServer server(80);

//Configuración de tiempo
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -21600, 60000);  // -21600 = UTC-6 para Sonora

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
int armada = 0;  // 0=Desarmada, 1=Armada

//Parametros ajustables (1-10)
//Sensores
//int sensibilidadRuido = 10;
////int sensibilidadMovimiento = 5;
float sensibilidadTemperatura = 5;

//Actuadores
int volumenBuzzer = 15;
//int brilloLuz = 5;

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
String responsableAlarma = "";
String horaActivacion = "";

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
  if (pausa.update() && armada == 1) {
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
    Serial.println("Ruido activado");
    responsableAlarma = "Ruido";
    activarAlarma();
  } else if (temperatura >= 27 * (sensibilidadTemperatura * .2)) {
    Serial.println("Temperatura activado");
    responsableAlarma = "Temperatura";
    activarAlarma();
  }
  //else
  //if ((sensibilidadMovimiento/5) * movimiento < 2000) {
  //  activarAlarma();
  //} else
  else if (magnetico < 1) {
    Serial.println("Magentico activado");
    responsableAlarma = "Magnetico";
    activarAlarma();
  }
}

/*
* Activa la alarma
*/
void activarAlarma() {
  if (estado == 1) {
    return;
  }
  // Actualiza la hora NTP
  timeClient.update();
  String hora = timeClient.getFormattedTime();
  time_t epochTime = timeClient.getEpochTime();
  struct tm* ptm = gmtime((time_t*)&epochTime);
  int dia = ptm->tm_mday;
  int mes = ptm->tm_mon + 1;
  int año = ptm->tm_year + 1900;
  estado = 1;
  //Activar el buzzer y luz
  analogWrite(buzzer, volumenBuzzer * 25);
  analogWrite(luz, 125);
  Serial.println("Alarma activada");
  horaActivacion = hora.c_str();
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
  horaActivacion = "";
  responsableAlarma = "";
}

/*
* Leer comandos por Serial
*/
void leerComando() {
  if (Serial.available()) {
    comandoSerial = Serial.readStringUntil('\n');
    comandoSerial.trim();

    if (comandoSerial.equalsIgnoreCase("on")) {
      armada=1;
      activarAlarma();
    } else if (comandoSerial.equalsIgnoreCase("off")) {
      desactivarAlarma();
    } else if (comandoSerial.startsWith("temp ")) {
      float val = comandoSerial.substring(5).toFloat();
      if (val >= 0.0 && val <= 100.0) {
        sensibilidadTemperatura = val;
        Serial.printf("Sensibilidad Temperatura ajustada a %.2f\n", val);
      }
    } else if (comandoSerial.startsWith("vol ")) {
      int val = comandoSerial.substring(4).toInt();
      if (val >= 0 && val <= 10) {
        volumenBuzzer = val;
        Serial.printf("Volumen Buzzer ajustado a %d\n", val);
      }
    } else if(comandoSerial.equalsIgnoreCase("arm")) {
      if (armada==0){
        armada=1;
        Serial.println("Alarma armada");
      }else{
        armada=0;
        Serial.println("Alarma desarmada");
      }
      desactivarAlarma();
    } 
    else {
      Serial.println("Comando no reconocido");
    }
  }
}

/*
* Envia los datos a un servidor local mysql
*/
void mandarDatos() {
  Serial.println("MYSQL ESTADO: " + String(estado) + " RUIDO: " + String(ruido) + " TEMPERATURA: " + String(temperatura) +
                 ////" MOVIMIENTO: " + String(movimiento) +
                 " MAGNETICO: " + String(magnetico));
}