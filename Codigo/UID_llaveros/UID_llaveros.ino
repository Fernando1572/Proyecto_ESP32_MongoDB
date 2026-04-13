#include <SPI.h>          // Librería para comunicación SPI
#include <MFRC522.h>     // Librería para el lector RFID RC522

// ========================
// Definición de pines ESP32
// ========================
#define SS_PIN 5   // Pin SDA (SS) del RC522
#define RST_PIN 22 // Pin de reset del RC522

// Crear objeto del lector RFID
MFRC522 MyLectorRF(SS_PIN, RST_PIN);

// Variable para almacenar el UID de la tarjeta
String BufferID = "";

// ========================
// Función de inicialización
// ========================
void setup() {
  Serial.begin(115200); // Inicializa comunicación serial (rápida para ESP32)

  // Inicializa el bus SPI con pines específicos del ESP32:
  // SCK = 18, MISO = 19, MOSI = 23, SS = SS_PIN
  SPI.begin(18, 19, 23, SS_PIN);

  // Inicializa el módulo RC522
  MyLectorRF.PCD_Init();

  Serial.println("Control Inicializado ...");
}

// ========================
// Loop principal
// ========================
void loop() {

  // Verifica si hay una nueva tarjeta presente
  if (MyLectorRF.PICC_IsNewCardPresent()) {

    // Intenta leer la tarjeta
    if (MyLectorRF.PICC_ReadCardSerial()) {

      // Limpia el buffer donde se guardará el UID
      BufferID = "";

      Serial.print("Card UID:");

      // Recorre cada byte del UID de la tarjeta
      for (byte i = 0; i < MyLectorRF.uid.size; i++) {

        // Imprime un 0 si el valor es menor a 0x10 (formato bonito)
        Serial.print(MyLectorRF.uid.uidByte[i] < 0x10 ? " 0" : " ");

        // Imprime el byte en formato hexadecimal
        Serial.print(MyLectorRF.uid.uidByte[i], HEX);

        // Guarda el UID en una cadena (también en hexadecimal)
        BufferID += String(MyLectorRF.uid.uidByte[i], HEX);
      }

      Serial.println();

      // Muestra el UID completo en una sola línea
      Serial.println("UID completo: " + BufferID);

      // Detiene la lectura de la tarjeta actual (importante)
      MyLectorRF.PICC_HaltA();
    }
  }
}