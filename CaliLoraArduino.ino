// --- <Libraries> --------------------------------------------------
#include <Arduino.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include <SPI.h>
#include <SD.h>

#include <Wire.h>
#include <DS3231.h>
#include <Adafruit_VEML7700.h>

// --- <[Libraries]> ------------------------------------------------

// --- <Definitions> ------------------------------------------------
// --- LEDS ---------------------
const int yellowLed = 2;
const int redLed = 3;
const int blueLed = 4;
const int greenLed = 5;
// --- DHT11 --------------------
#define DHTPIN 6
#define DHTTYPE DHT11

// --- <[Definitions]> ----------------------------------------------

// --- <Set-up> -----------------------------------------------------
// --- DHT11 --------------------
DHT dht(DHTPIN, DHTTYPE);
// --- AM312 --------------------
// Set GPIOs for LED and PIR Motion Sensor
const int motionLed = LED_BUILTIN;
const int motionSensor = 7;
bool motion = false;
// --- I2C_TEMP -----------------
float current;
int termPin = 0;   // The analog pin to which the thermistor is connected is A0
int termNom = 100000;   // Thermistor reference resistance
int refTemp = 29;   // Temperature for reference resistance
int beta = 3950;   // Beta factor
int resistance = 100000; // value of resistance in series in the circuit
// --- General Times ------------
unsigned long lastTime = 0;
unsigned long interval = 16000;
// --- Join State ---------------
bool joined = false;
// --- <[Set-up]> ---------------------------------------------------

// Checks if motion was detected, sets LED HIGH and starts a timer
void detectsMovement() {
  motion = true;
  digitalWrite(motionLed, HIGH);
}

void setup() {
  // Inicializar el monitor serial a 9600 baudios
  Serial.begin(9600);
  
  // Esperar hasta que el monitor serial esté listo
  // while (!Serial) {
  //   ; // Esperar
  // }
  Serial1.begin(115200);
  // while (!Serial1) {
  //   delay(2000);
  //   Serial.println("Not Available");
  // }
  Serial.println("Serial Ready");

  dht.begin();

  // --- <Leds> -----------------------------------------------------
  pinMode(yellowLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(greenLed, OUTPUT);

  digitalWrite(yellowLed, HIGH);
  digitalWrite(redLed, LOW);
  digitalWrite(blueLed, LOW);
  digitalWrite(greenLed, LOW);
  // --- <[Leds]> ---------------------------------------------------

  // --- <SENSORS> --------------------------------------------------
  // --- AM312 ------------------
  pinMode(motionSensor, INPUT_PULLUP);
  // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  pinMode(motionLed, OUTPUT);
  digitalWrite(motionLed, LOW);

  Wire.begin();
  // --- <[SENSORS]> ------------------------------------------------

  // --- <NOT JOIN> -------------------------------
  Serial1.println("AT+JOIN=0");
  delay(2000);
  // --- <[NOT JOIN]> -----------------------------

  // --- <OTTA PARAMETERS> ------------------------
  // Configurando DEVEUI
  Serial1.println("AT+DEVEUI=2953a317f5f1a0fd");
  delay(1000); // Pequeño delay para esperar respuesta
  // Configurando APPEUI
  Serial1.println("AT+APPEUI=2953a317f5f1a0fd");
  delay(1000);
  // Configurando APPKEY ,m
  Serial1.println("AT+APPKEY=764397e38b99e595aef95cc113debb48");
  delay(1000);

  Serial1.println("AT+BAND=5");
  delay(1000);
  Serial1.println("AT+MASK=0002");
  delay(1000);
  Serial1.println("AT+ADR=1");
  delay(1000);
  // --- <[OTTA PARAMETERS]> ----------------------

  // --- <JOIN> -----------------------------------
  Serial1.println("AT+JOIN=1:0:10:5");
  // --- <[JOIN]> ---------------------------------
}

void loop() {
  // Verificar si hay datos disponibles en el monitor serial (enviados desde la PC)
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n'); // Leer comando AT ingresado en el monitor serial
    Serial1.println(command); // Enviar el comando al módulo (por ejemplo, LoRa o GSM)
    Serial.print("Enviando comando: ");
    Serial.println(command); // Mostrar el comando enviado en el monitor serial
  }

  // Verificar si hay respuesta del módulo
  if (Serial1.available()) {
    String line = Serial1.readStringUntil('\n'); // Leer la respuesta del módulo
    if (line.indexOf("+EVT:JOINED") >= 0 || line.indexOf("+EVT:LINKCHECK:0:") >= 0) {
      joined = true;
      digitalWrite(greenLed, HIGH);
    } else if (line.indexOf("+EVT:SEND_CONFIRMED_OK") >= 0 || line.indexOf("+EVT:TX_DONE") >= 0) {
      digitalWrite(blueLed, HIGH);
      delay(1000);
      digitalWrite(blueLed, LOW);
    } else if (line.indexOf("+EVT:SEND_CONFIRMED_FAILED(4)") >= 0 || line.indexOf("+EVT:JOIN_FAILED_RX_TIMEOUT") >= 0) {
      digitalWrite(redLed, HIGH);
      delay(500);
      digitalWrite(redLed, LOW);
    } else if (line.indexOf("OK") >= 0) {
      digitalWrite(yellowLed, HIGH);
      delay(500);
      digitalWrite(yellowLed, LOW);
    }
    if (line.indexOf("+EVT:LINKCHECK:1:0:0:0:0") >= 0) {
      digitalWrite(greenLed, LOW);
    }
    
    Serial.print("RAK3272: ");
    Serial.println(line);
  }

  // IF CONNECTED
  // AND IF FULLFILL THE INTERVAL
  // --- <SEND MESSAGE> ---------------------------------------------
  if (joined && millis() - lastTime > interval) {
    // --- <DHT11> ------------------------------
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    unsigned int humHex = h * 100;
    unsigned int tempHex = t * 100;
    // --- <[DHT11]> ----------------------------

    // --- VEML7700 -----------------------------
    // float lux = veml.readLux();
    // unsigned int luxHex = lux * 100;
    // --- <[VEML7700]> -------------------------

    // ---
    current = analogRead(termPin);
    current = 1023 / current - 1;
    current = resistance / current;

    float temperature = current / termNom;  // (R/Ro)
    temperature = log(temperature);         // ln(R/Ro)
    temperature /= beta;                    // 1/B * ln(R/Ro)
    temperature += 1.0 / (refTemp + 273.15); // + (1/To)
    temperature = 1.0 / temperature;        // The inverted value
    temperature -= 273.15;                  // Convert from Kelvin to Celsius

    unsigned int tempRealHex = (unsigned int)(temperature * 100);  // Multiplicar por 100 para enviar un valor entero
    // ---

    // --- AM312 --------------------------------
    unsigned int motionHex = motion ? 1 : 0;
    // --- <[AM312]> ----------------------------
    // --- RTC ----------------------------------
    // DateTime now = myRTC.now();
    //String formatNow = String(now.year()+"/"+now.month()+"/"+now.day());                       // converting a constant char into a String
    // --- <SOLVE A BUG> ------------------------
    Serial1.println("AT+MASK=0002");
    delay(1000);
    // --- <[SOLVE A BUG]> ----------------------

    Serial.println(h);
    Serial.println(t);
    Serial.println(motion);
    Serial.println(temperature);
    Serial.println(tempRealHex);

    // --- <PAYLOAD> ----------------------------
    char payload[17];
    sprintf(payload, "%04X%04X%02X%04X", tempHex, humHex, motionHex, tempRealHex);
    // --- <[PAYLOAD]> --------------------------

    // --- <SEND PAYLOAD> -----------------------
    String command = "AT+SEND=1:" + String(payload);
    Serial1.println(command);
    // --- <[SEND PAYLOAD]> ---------------------
    
    // --- <AFTER SEND> -------------------------
    // --- AM312 ------------
    digitalWrite(motionLed, LOW);
    motion = false;

    // --- INTERVAL ---------
    lastTime = millis();
    // --- <[AFTER SEND]> -----------------------
  }
  // --- <[SEND MESSAGE]> -------------------------------------------

  // --- <SEND MESSAGE FROM CONSOLE> --------------
  while (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    Serial1.println(command);
  }
  // --- <[SEND MESSAGE FROM CONSOLE]> ------------
}
