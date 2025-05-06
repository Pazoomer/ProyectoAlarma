#pragma once

#include <ESPAsyncWebServer.h>  // Necesario para que reconozca AsyncWebServerRequest

extern AsyncWebServer server;  // Declaración externa del servidor

void conectaRedWiFi(const char* ssid, const char* password);
void inicializaLittleFS();
void configuraServidor();
void noHallada(AsyncWebServerRequest* request);
String processor(const String& var);
