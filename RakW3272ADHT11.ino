#include <Arduino.h>
#include <DHT.h>

// Definición de los pines para los LEDs
const int ledAmarillo = 2; // Cambia según los pines disponibles en tu MKR Zero
const int ledRojo = 3;
const int ledAzul = 4;
const int ledVerde = 5;

#define DHTPIN 6
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

unsigned long lastTime = 0;
unsigned long interval = 35000;

bool joined = false; // Estado de unión a la red LoRaWAN

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  dht.begin();

  pinMode(ledAmarillo, OUTPUT);
  pinMode(ledRojo, OUTPUT);
  pinMode(ledAzul, OUTPUT);
  pinMode(ledVerde, OUTPUT);

  digitalWrite(ledAmarillo, LOW);
  digitalWrite(ledRojo, LOW);
  digitalWrite(ledAzul, LOW);
  digitalWrite(ledVerde, LOW);

  Serial.println("Configurando parametros OTAA...");

  // Configurando DEVEUI
  Serial1.println("AT+JOIN=0");
  delay(2000); // Pequeño delay para esperar respuesta

  // Configurando DEVEUI
  Serial1.println("AT+DEVEUI=8ca643ed245347a6");
  delay(1000); // Pequeño delay para esperar respuesta
  // Configurando APPEUI
  Serial1.println("AT+APPEUI=8ca643ed245347a6");
  delay(1000);
  // Configurando APPKEY
  Serial1.println("AT+APPKEY=764397e38b99e595aef95cc113debb48");
  delay(1000);

  Serial1.println("AT+BAND=5");
  delay(1000);
  Serial1.println("AT+MASK=0002");
  delay(1000);
  Serial1.println("AT+ADR=1");
  delay(1000);

  Serial1.println("AT+JOIN=1:0:10:5");

}

void loop() {

  if (Serial1.available()) {
    String line = Serial1.readStringUntil('\n');
    if (line.indexOf("+EVT:JOINED") >= 0) {
      joined = true;
      digitalWrite(ledVerde, HIGH);
      Serial.println("Módulo unido a la red LoRaWAN.");
    } else if (line.indexOf("+EVT:SEND_CONFIRMED_OK") >= 0 || line.indexOf("+EVT:TX_DONE") >= 0) {
      digitalWrite(ledAzul, HIGH);
      delay(1000);
      digitalWrite(ledAzul, LOW);
    } else if (line.indexOf("+EVT:SEND_CONFIRMED_FAILED(4)") >= 0 || line.indexOf("+EVT:JOIN_FAILED_RX_TIMEOUT") >= 0) {
      digitalWrite(ledRojo, HIGH);
      delay(500);
      digitalWrite(ledRojo, LOW);
    } else if (line.indexOf("OK") >= 0) {
      digitalWrite(ledAmarillo, HIGH);
      delay(500);
      digitalWrite(ledAmarillo, LOW);
    }
    
    Serial.print("RAK3272: "); // Imprime cualquier otra salida del módulo
    Serial.println(line);
  }

  // Proceder solo si el módulo está unido a la red LoRaWAN
  if (joined && millis() - lastTime > interval) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (!isnan(h) && !isnan(t)) {
      // Convertimos los valores de humedad y temperatura a un formato adecuado para el payload
      unsigned int humHex = h * 100;   // Multiplicamos por 100 para obtener más precisión
      unsigned int tempHex = t * 100;  // Multiplicamos por 100 para obtener más precisión

      char payload[9];  // 4 dígitos para cada valor y un carácter nulo '\0' al final
      sprintf(payload, "%04X%04X", tempHex, humHex);  // Formateamos en hexadecimal
      
      // Enviamos el comando AT+SEND por el puerto 1 con el payload de humedad y temperatura
      String command = "AT+SEND=1:" + String(payload);
      Serial1.println(command);
      
      lastTime = millis();
    } else {
      Serial.println("Failed to read from DHT sensor!");
    }
  }

  // Permite enviar comandos manualmente desde el monitor serial al RAK3272
  while (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    Serial1.println(command);
  }
}
