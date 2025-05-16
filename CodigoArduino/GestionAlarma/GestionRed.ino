/*
 * GestionRed.ino
 *
 * Este modulo permite la conexión a intenet
 * y la configuración asociada a la pagina web
 */
#include "ESPAsyncWebServer.h"
extern AsyncWebServer server;
void noHallada(AsyncWebServerRequest* request);

/*
* Conecta el microcontrolador ESP32 a una red WiFi
*/
void conectaRedWiFi(const char* ssid, const char* password) {
  // Conexion a la red
  WiFi.begin(ssid, password);
  Serial.print("Conectandose a la red ");
  Serial.print(ssid);
  Serial.println(" ...");

  unsigned long tiempoInicio = millis();  // Tiempo de inicio para evitar bucles infinitos
  unsigned long tiempoLimite = 30000;  // 30 segundos para esperar la conexión
  
  // Mientras no se ha conectado a la red WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    
    if (millis() - tiempoInicio > tiempoLimite) {
      Serial.println("\nTiempo de espera excedido");
      return;
    }
  }
  
  Serial.println("\nConexion establecida");
  Serial.print("Direccion IP del servidor web: ");
  Serial.println(WiFi.localIP());
}

/**
* Esta funcion monta el sistema de archivos LittleFS
*/
void inicializaLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("Ocurrió un error al montar LittleFS");
  } else {
    Serial.println("Se monto LittleFS con exito");
  }
}

/*
* Mapea las diferentes funcionalidades del servidor a los URL
* con las que seran invocadas
*/
void configuraServidor() {
  // Carga los archivos estaticos desde la raiz del sistema de archivos LittleFS
  server.serveStatic("/", LittleFS, "/");
  // Si se invoca al servidor con la URL: "direccionIP_ServidorWeb/"
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    // Despliega las URLs/paginas
    request->send(LittleFS, "/index.html", "text/html");
  });
  // Si se invoca al servidor con la URL: "direccionIP_ServidorWeb/e"
  server.on("/e", HTTP_GET, [](AsyncWebServerRequest* request) {
    // Despliega una página con la última lectura de la luminosidad y
    // las luminosidades máxima, mínima y promedio de las lecturas del último minuto.
    request->send(LittleFS, "/promedio.html", "text/html",
                  false, processor);
  });
  // Si se invoca al servidor con la URL: "direccionIP_ServidorWeb/u"
  server.on("/u", HTTP_GET, [](AsyncWebServerRequest* request) {
    // Despliega una página con el estado de la alarma y el causante
    request->send(LittleFS, "/estado.html", "text/html",
                  false, processor);
  });
  // Invoca a la funcion noHallada() si se invoca al servidor con una URL inexistente
  server.onNotFound(noHallada);
  server.begin();
}

/*
* Esta funcion le envia al cliente una pagina
* con el mensaje de que la URL solicitada no se encontro
*/
void noHallada(AsyncWebServerRequest* request) {
  // Le envia al cliente el mensaje de respuesta. Recibe como
  // argumentos:
  // - 404: El codigo de estado HTTP (indica que no se pudo
  // atender la solicitud).
  // - "text/plain": El tipo del contenido del mensaje (texto
  // plano.
  // - "URL no encontrada": El cuerpo del mensaje de respuesta,
  // una cadena con el mensaje "URL no encontrada".
  request->send(404, "text/plain", "URL no encontrada");
}

/*
* Coloca los valores clave en la pagina web
*/
String processor(const String& var) {
  if (var == "ESTADO") {
    if(estado==0){
      return "Desactivada";
    }else if (estado==1){
      return "Activada";
    }else{
      return "Desconocido";
    }
  }

  if (var == "RUIDO") {
    return String(ruido);
  }
  if (var == "RUIDOMAX") {
    return String(ruidoMaximo);
  }
  if (var == "RUIDOMIN") {
    return String(ruidoMinimo);
  }
  if (var == "RUIDOPROM") {
    return String(ruidoPromedio);
  }

  //if (var == "MOVIMIENTO") {
   // return String(movimiento);
  //}
  //if (var == "MOVIMIENTOMAX") {
   // return String(movimientoMaximo);
  //}
  //if (var == "MOVIMIENTOMIN") {
    //return String(movimientoMinimo);
  //}
  //if (var == "MOVIMIENTOPROM") {
  //  return String(movimientoPromedio);
  //}

  if (var == "MAGNETICO") {
    return String(magnetico);
  }
  if (var == "MAGNETICOMAX") {
    return String(magneticoMaximo);
  }
  if (var == "MAGNETICOMIN") {
    return String(magneticoMinimo);
  }
  if (var == "MAGNETICOPROM") {
    return String(magneticoPromedio);
  }

  if (var == "TEMPERATURA") {
    return String(temperatura);
  }
  if (var == "TEMPERATURAMAX") {
    return String(temperaturaMaximo);
  }
  if (var == "TEMPERATURAMIN") {
    return String(temperaturaMinimo);
  }
  if (var == "TEMPERATURAPROM") {
    return String(temperaturaPromedio);
  }
  if (var == "RESPONSABLE") {
    return String(responsableAlarma);
  }
  if (var == "HORAACTIVACION") {
    return String(horaActivacion);
  }
  if (var == "ARMADA") {
    if(armada==0){
      return "Desarmada";
    }else if (armada==1){
      return "Armada";
    }else{
      return "Desconocido";
    }
  }
  return String();
}