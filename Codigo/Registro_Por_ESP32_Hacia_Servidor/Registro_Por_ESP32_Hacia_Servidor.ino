#include <WiFi.h>        // Librería para conexión WiFi en ESP32
#include <HTTPClient.h> // Librería para realizar peticiones HTTP
#include <SPI.h>        // Comunicación SPI (usada por el RFID)
#include <MFRC522.h>    // Librería para el lector RFID RC522

// ========================
// CONFIGURACIÓN WIFI
// ========================

// Nombre de la red WiFi
const char* ssid = "";

// Contraseña de la red WiFi
const char* password = "";

// URL del servidor Node.js (endpoint donde se envía el UID)
const char* serverName = "http://********:3000/api/rfid";


// ========================
// CONFIGURACIÓN RFID
// ========================

// Pines del módulo RFID RC522
#define SS_PIN 5    // SDA (SS)
#define RST_PIN 22  // Reset

// Crear objeto del lector RFID
MFRC522 MyLectorRF(SS_PIN, RST_PIN);

// Variable donde se almacenará el UID leído
String BufferID = "";


// ========================
// CONFIGURACIÓN DE LEDs
// ========================

#define LED_AZUL 2  // LED que indica acceso permitido
#define LED_ROJO 4  // LED que indica acceso denegado


// ========================
// FUNCIÓN SETUP
// Se ejecuta una sola vez al iniciar el ESP32
// ========================
void setup() {
  Serial.begin(115200); // Inicializar comunicación serial

  // Configurar pines de LEDs como salida
  pinMode(LED_AZUL, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);

  // Inicializar comunicación SPI para el RFID
  SPI.begin(18, 19, 23, SS_PIN);
  MyLectorRF.PCD_Init(); // Inicializar lector RFID

  // Conectar a la red WiFi
  WiFi.begin(ssid, password);
  Serial.println("Conectando a WiFi...");

  // Esperar hasta que se conecte
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado");

  // Mostrar IP asignada al ESP32
  Serial.print("IP ESP32: ");
  Serial.println(WiFi.localIP());
}


// ========================
// FUNCIÓN LOOP
// Se ejecuta continuamente
// ========================
void loop() {

  // Verificar si hay una nueva tarjeta RFID presente
  if (MyLectorRF.PICC_IsNewCardPresent()) {

    // Leer los datos de la tarjeta
    if (MyLectorRF.PICC_ReadCardSerial()) {

      BufferID = ""; // Limpiar variable

      // Convertir UID a string en formato hexadecimal
      for (byte i = 0; i < MyLectorRF.uid.size; i++) {
        BufferID += String(MyLectorRF.uid.uidByte[i], HEX);
      }

      BufferID.toLowerCase(); // Convertir a minúsculas

      // Mostrar UID en consola
      Serial.println("UID: " + BufferID);

      // Verificar conexión WiFi
      if (WiFi.status() == WL_CONNECTED) {

        HTTPClient http;

        // Iniciar conexión HTTP con el servidor
        http.begin(serverName);

        // Especificar que se enviará JSON
        http.addHeader("Content-Type", "application/json");

        // Crear JSON con el UID
        String json = "{\"uid\":\"" + BufferID + "\"}";

        // Enviar petición POST
        int httpResponseCode = http.POST(json);

        // Mostrar código de respuesta HTTP
        Serial.print("Codigo HTTP: ");
        Serial.println(httpResponseCode);

        // Si la respuesta es válida
        if (httpResponseCode > 0) {

          // Obtener respuesta del servidor
          String response = http.getString();
          Serial.println("Respuesta: " + response);

          // ========================
          // VALIDAR RESPUESTA
          // ========================
          // Se busca la palabra "true" en la respuesta
          // Si está presente -> acceso permitido
          // Si no -> acceso denegado

          if (response.indexOf("true") > 0) {
            Serial.println("ACCESO PERMITIDO");

            digitalWrite(LED_AZUL, HIGH); // Encender LED azul
            digitalWrite(LED_ROJO, LOW);  // Apagar LED rojo

          } else {
            Serial.println("ACCESO DENEGADO");

            digitalWrite(LED_AZUL, LOW);  // Apagar LED azul
            digitalWrite(LED_ROJO, HIGH); // Encender LED rojo
          }

          // Mantener LEDs encendidos por 2 segundos
          delay(2000);

          // Apagar ambos LEDs
          digitalWrite(LED_AZUL, LOW);
          digitalWrite(LED_ROJO, LOW);
        }

        // Finalizar conexión HTTP
        http.end();
      }

      // Detener lectura de la tarjeta actual
      MyLectorRF.PICC_HaltA();
    }
  }
}