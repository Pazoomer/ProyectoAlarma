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
#include "GestionDatos.h"
#include "GestionRed.h"
#include "GestionPeticiones.h"
#include "ESPAsyncWebServer.h"

const unsigned int TAM_COLA = 5;
int cola[TAM_COLA];
const unsigned int N_DATOS = 2000;

const long PAUSA = 1000;
const unsigned int BAUD_RATE = 115200;
noDelay pausa(PAUSA);

int dato = 1;
int nDatos;

const char* ssid = "MEGACABLE-2E9F";
const char* password = "Uu5raDYY";

void actualizaLectura();
int obtenLuz();
int luz;
int luzMaxima;
int luzMinima;
int luzPromedio;
const unsigned int fotoreceptor = 34;
AsyncWebServer server(80);

void setup() {
  luzMaxima = 0;
  luzMinima = 1023;
  Serial.println("Start");
  Serial.begin(BAUD_RATE);
  delay(100);
  Serial.println("Iniciado");

  conectaRedWiFi(ssid, password);
  inicializaLittleFS();
  configuraServidor();

  Serial.println("Servidor web inicializado");
}

void loop() {
  if (pausa.update()) {
    if (dato <= N_DATOS) {
      actualizaLectura();
      nDatos = insertaCola(cola, luz, TAM_COLA);
      despliegaCola(cola, nDatos);
      float promedioMov = obtenPromedioMovil(cola, nDatos);
      Serial.print("Promedio movil: ");
      Serial.println(promedioMov);
      dato++;
    }
  }
}

/*
* Esta funcion llama a las funciones de recolección de datos
*/
void actualizaLectura() {
  luz = obtenLuz();
  luzPromedio = obtenPromedioMovil(cola, TAM_COLA); 
  if (luz > luzMaxima) {
    luzMaxima = luz;
  }
  if (luz < luzMinima) {
    luzMinima = luz;
  }
}

/*
* Esta funcion obtiene la luz del fotoreceptor
*/
int obtenLuz() {
  int valorAnalogico = analogRead(fotoreceptor);

  // Imprimir los valores para depuración
  Serial.print("Valor Analogico: ");
  Serial.print(valorAnalogico);

  return valorAnalogico;
}
