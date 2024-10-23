// -------------------------- Libraries ----------------------------
#include <DHT.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include <SPI.h>
#include <SD.h>

#include <Wire.h>
#include <DS3231.h>
#include <Adafruit_VEML7700.h>
// -------------------------- Definitions --------------------------
// -------------------- DHT11 --------------------
#define PIN_DHT11 7
#define DHTTYPE DHT11
// -------------------- DS18B20 ------------------
// #define ONE_WIRE_BUS 6
// -------------------- AM312 --------------------
#define timeSeconds 2

// --------------------------------- Set-up -------------------------------
// -------------------- DHT11 --------------------
DHT dht(PIN_DHT11, DHTTYPE);
// -------------------- DS18B20 ------------------
// // Setup a oneWire instance to communicate with any OneWire devices
// OneWire oneWire(ONE_WIRE_BUS);
// // Pass our oneWire reference to Dallas Temperature sensor 
// DallasTemperature sensors(&oneWire);
// -------------------- AM312 --------------------
// Set GPIOs for LED and PIR Motion Sensor
const int led = LED_BUILTIN;
const int motionSensor = 5;
// -------------------- VEML7700 -----------------
Adafruit_VEML7700 veml = Adafruit_VEML7700();
// -------------------- SD -----------------------
const int chipSelect = SDCARD_SS_PIN;

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

//
RTClib myRTC;

void setup() {
  Serial.begin(9600);
  Serial.println(F("DHTxx test!"));

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  // -------------------- VEML7700 -----------------
  if (!veml.begin()) {
    Serial.println("Sensor not found");
    while (1);
  }

  veml.setGain(VEML7700_GAIN_1_8);
  veml.setIntegrationTime(VEML7700_IT_100MS);

  // -------------------- SD -----------------------
  // Serial.print("Initializing SD card...");

  // // see if the card is present and can be initialized:
  // if (!SD.begin(chipSelect)) {
  //   Serial.println("Card failed, or not present");
  //   // don't do anything more:
  //   while (1);
  // }
  // Serial.println("card initialized.");
  // -------------------- GENERAL -------------------

  dht.begin();
  // sensors.begin();

  // -------------------- AM312 --------------------
  pinMode(motionSensor, INPUT_PULLUP);
  // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);

  Wire.begin();

}

// --------------------------------- Loop ---------------------------------
void loop() {
  delay(2000);

  // -------------------- SD -----------------------
  // make a string for assembling the data to log:
  String dataString = "";
  // -------------------- SD -----------------------


  // Obtener el tiempo de captura en milisegundos
  unsigned long timeMillis = millis();

  // -------------------- DHT11 --------------------
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  float hic = dht.computeHeatIndex(t, h, false);

  // -------------------- DS18B20 ------------------
  // sensors.requestTemperatures();

  // -------------------- AM312 --------------------
  if((digitalRead(led) == HIGH) && (motion == false)) {
    motion = true;
  }

  DateTime now = myRTC.now();

  // -------- PRINT / SAVE --------
  Serial.println("-----------------------------------------------");
  Serial.print("Capture Time: ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
  Serial.println("-------------------- DHT11 --------------------");
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.println("%");
  Serial.print(F("Temperature: "));
  Serial.print(t);
  Serial.println(F("°C "));
  Serial.print(F("°Heat index: "));
  Serial.print(hic);
  Serial.println(F("°C"));

  // Serial.println("-------------------- DS18B20 ------------------");
  // Serial.print("Celsius temperature: ");
  // Serial.println(sensors.getTempCByIndex(0)); 

  Serial.println("-------------------- AM312 --------------------");
  Serial.print("Movement?: ");
  Serial.println(motion);
  
  Serial.println("-------------------- VEML7700 -----------------");
    Serial.print("lux: "); Serial.println(veml.readLux());


  // // -------- PRINT / SAVE --------
  dataString += String(now.year(), DEC);
  dataString += String(";");
  dataString += String(now.month(), DEC);
  dataString += String(";");
  dataString += String(now.day(), DEC);
  dataString += String(";");
  dataString += String(now.hour(), DEC);
  dataString += String(";");
  dataString += String(now.minute(), DEC);
  dataString += String(";");
  dataString += String(now.second(), DEC);
  dataString += String(";");
  dataString += String(h);
  dataString += String(";");
  dataString += String(t);
  dataString += String(";");
  // -------------------- DS18B20 ------------------
  // dataString += String(sensors.getTempCByIndex(0));
  // dataString += String(";"); 
  // -------------------- AM312 --------------------
  dataString += String(motion);
  dataString += String(";");
  dataString += String(veml.readLux());

  // open the file. note that only one file can be open at a time,
  // so you have to close this one befores opening another.
  File dataFile = SD.open("data.csv", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening data.csv");
    dataFile.close();
  }

  digitalWrite(led, LOW);
  startTimer = false;
  motion = false;


}
