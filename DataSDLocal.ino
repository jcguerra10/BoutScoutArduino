// -------------------------- Libraries ----------------------------
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <SdFat.h>
#include <Wire.h>
#include <DS3231.h>
#include <Adafruit_VEML7700.h>

// -------------------------- Definitions --------------------------
// -------------------- DHT11 --------------------
#define PIN_DHT11 4
#define DHTTYPE DHT11
// -------------------- AM312 --------------------
#define timeSeconds 2

// -------------------- SdFat -----------------------
SdFat SD;       // Usa SdFat en lugar de SD
File dataFile;  // Define el archivo para almacenar datos

const int chipSelect = SS1;  // Define el pin de selección del chip (ajústalo según tu placa)

// -------------------- VEML7700 -----------------
Adafruit_VEML7700 veml = Adafruit_VEML7700();

// -------------------- DHT11 --------------------
DHT dht(PIN_DHT11, DHTTYPE);

// -------------------- Thermistor ---------------
float current;
int termPin = 0;          // The analog pin to which the thermistor is connected is A0
int termNom = 100000;     // Thermistor reference resistance
int refTemp = 28;         // Temperature for reference resistance
int beta = 3950;          // Beta factor
int resistance = 100000;  // value of resistance in series in the circuit

// -------------------- AM312 --------------------
const int led = LED_BUILTIN;
const int motionSensor = 5;

// Timer: Auxiliary variables
unsigned long now = millis();
unsigned long lastTrigger = 0;
boolean startTimer = false;
boolean motion = false;

// Checks if motion was detected, sets LED HIGH and starts a timer
void detectsMovement() {
  digitalWrite(led, HIGH);
  startTimer = true;
  lastTrigger = millis();
}

// RTC
RTClib myRTC;

void setup() {
  Serial.begin(9600);
  Serial.println(F("DHTxx test!"));

  // -------------------- VEML7700 -----------------
  if (!veml.begin()) {
    Serial.println("Sensor not found");
    while (1)
      ;
  }

  veml.setGain(VEML7700_GAIN_1_8);
  veml.setIntegrationTime(VEML7700_IT_100MS);

  // -------------------- SdFat -----------------------
  Serial.print("Initializing SD card with SdFat...");

  // Intenta inicializar la tarjeta SD
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1)
      ;  // Si falla, detén la ejecución
  }
  Serial.println("Card initialized.");

  dht.begin();

  // -------------------- AM312 --------------------
  pinMode(motionSensor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
}

void loop() {
  delay(2000);

  // -------------------- DHT11 --------------------
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  float hic = dht.computeHeatIndex(t, h, false);

  DateTime now = myRTC.now();

  // ---
  current = analogRead(termPin);
  current = 1023 / current - 1;
  current = resistance / current;

  float temperature = current / termNom;    // (R/Ro)
  temperature = log(temperature);           // ln(R/Ro)
  temperature /= beta;                      // 1/B * ln(R/Ro)
  temperature += 1.0 / (refTemp + 273.15);  // + (1/To)
  temperature = 1.0 / temperature;          // The inverted value
  temperature -= 273.15;                    // Convert from Kelvin to Celsius

  unsigned int tempRealHex = (unsigned int)(temperature * 100);  // Multiplicar por 100 para enviar un valor entero
  // ---

  // -------------------- AM312 --------------------
  if ((digitalRead(led) == HIGH) && (motion == false)) {
    motion = true;
  }

  // -------------------- VEML7700 -----------------
  float lux = veml.readLux();

  // -------- PRINT / SAVE --------
  String dataString = "";
  // Agregamos los datos del RTC (fecha y hora)
  dataString += String(now.year(), DEC);
  dataString += String("/");
  dataString += String(now.month(), DEC);
  dataString += String("/");
  dataString += String(now.day(), DEC);
  dataString += String(" ");
  dataString += String(now.hour(), DEC);
  dataString += String(":");
  dataString += String(now.minute(), DEC);
  dataString += String(":");
  dataString += String(now.second(), DEC);

  // Agregamos los datos de los sensores al dataString
  dataString += String("; DHT Humedad: ");
  dataString += String(h);  // Humedad del DHT11
  dataString += String(" %");

  dataString += String("; DHT Temperatura: ");
  dataString += String(t);  // Temperatura del DHT11
  dataString += String(" °C");

  dataString += String("; Thermistor: ");
  dataString += String(temperature);  // Índice de calor calculado
  dataString += String(" °C");

  dataString += String("; Movimiento detectado: ");
  dataString += String(motion ? "Sí" : "No");  // Estado del sensor de movimiento

  dataString += String("; Luz (lux): ");
  dataString += String(lux);  // Lectura del sensor VEML7700

  // -------------------- SdFat -----------------------
  // Abre el archivo para escribir
  dataFile = SD.open("data.csv", FILE_WRITE);

  if (dataFile) {
    dataFile.println(dataString);  // Escribe en el archivo
    dataFile.close();              // Cierra el archivo
    Serial.println(dataString);    // Imprime también en el monitor serial
  } else {
    Serial.println("Error opening data.csv");
  }

  digitalWrite(led, LOW);
  startTimer = false;
  motion = false;
}
