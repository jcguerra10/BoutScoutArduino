#include <SPI.h>
#include <SdFat.h>
#include <Wire.h>
#include <Adafruit_VEML7700.h>
#include <RTClib.h>
#include <DHT.h>

// Pins and configuration
#define DHTPIN 6              // Pin connected to DHT11
#define DHTTYPE DHT11         // DHT sensor type
#define LEDPIN 2              // Pin connected to the LED
#define THERMISTORPIN A0      // Pin connected to the thermistor
#define CSPIN SDCARD_SS_PIN   // Chip Select (CS) pin for SD card

// Thermistor configuration
#define SERIESRESISTOR 10000     // Resistor value in ohms
#define THERMISTORNOMINAL 10000  // Thermistor resistance at 25°C (10k)
#define TEMPERATURENOMINAL 25    // Nominal temperature (°C)
#define BCOEFFICIENT 3950        // Thermistor beta coefficient

// SdFat and file objects
SdFat sd;
SdFile dataFile;

// Library initializations
DHT dht(DHTPIN, DHTTYPE);
RTC_DS3231 rtc;
Adafruit_VEML7700 veml;

// Error tracking
bool dhtError = false;
bool vemlError = false;
bool rtcError = false;
bool sdError = false;

void setup() {
  Serial.begin(9600);

  // Set up the LED pin as output
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, LOW);

  // Disable the Ethernet controller to avoid interference with SPI
  disableEthernetController();

  // Initialize the DHT sensor
  dht.begin();
  Serial.println("DHT11 initialized.");

  // Initialize the RTC DS3231
  if (!rtc.begin()) {
    Serial.println("Failed to initialize the RTC DS3231.");
    rtcError = true;
  }
  if (!rtcError && rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time.");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Set with compilation date/time
  }

  // Initialize the VEML7700 sensor
  if (!veml.begin()) {
    Serial.println("Failed to initialize the VEML7700 sensor.");
    vemlError = true;
  }
  if (!vemlError) {
    veml.setGain(VEML7700_GAIN_1);       // Gain of 1x
    veml.setIntegrationTime(VEML7700_IT_100MS); // Integration time of 100 ms
  }

  // Initialize the SD card
  if (!sd.begin(CSPIN, SD_SCK_MHZ(50))) {
    Serial.println("Failed to initialize the SD card.");
    sdError = true;
  } else {
    Serial.println("SD card initialized successfully.");
    if (dataFile.open("sensor_log.csv", O_RDWR | O_CREAT | O_APPEND)) {
      dataFile.println("Date,Time,DHT_Temp,DHT_Hum,VEML_Lux,Thermistor_Temp");
      dataFile.close();
    } else {
      Serial.println("Failed to create the log file.");
      sdError = true;
    }
  }
}

void loop() {
  // Signal errors via LED
  if (rtcError || dhtError || vemlError || sdError) {
    signalError();
  }

  // Turn on the LED to indicate sensor readings
  digitalWrite(LEDPIN, HIGH);

  // Read data from the DHT sensor
  float tempDHT = dht.readTemperature();
  float hum = dht.readHumidity();
  dhtError = isnan(tempDHT) || isnan(hum);

  // Read data from the VEML7700 sensor
  float lux = 0;
  if (!vemlError) {
    lux = veml.readLux();
    vemlError = (lux < 0);
  }

  // Read data from the thermistor
  int adcValue = analogRead(THERMISTORPIN);
  float resistance = SERIESRESISTOR / (1023.0 / adcValue - 1);
  float steinhart;
  steinhart = resistance / THERMISTORNOMINAL;   // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // Convert to °C

  // Turn off the LED
  digitalWrite(LEDPIN, LOW);

  // Get the current date and time from the RTC
  char date[11], time[9];
  if (!rtcError) {
    DateTime now = rtc.now();
    sprintf(date, "%02d/%02d/%04d", now.day(), now.month(), now.year());
    sprintf(time, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  } else {
    sprintf(date, "N/A");
    sprintf(time, "N/A");
  }

  // Log data to SD card
  if (!sdError) {
    if (dataFile.open("sensor_log.csv", O_RDWR | O_APPEND)) {
      dataFile.print(date); dataFile.print(",");
      dataFile.print(time); dataFile.print(",");
      dataFile.print(dhtError ? "Error" : String(tempDHT)); dataFile.print(",");
      dataFile.print(dhtError ? "Error" : String(hum)); dataFile.print(",");
      dataFile.print(vemlError ? "Error" : String(lux)); dataFile.print(",");
      dataFile.println(steinhart);
      dataFile.close();
    } else {
      Serial.println("Failed to write to the SD card.");
      sdError = true;
    }
  }

  // Display the data on the serial monitor
  Serial.print("Date: "); Serial.print(date);
  Serial.print(" Time: "); Serial.print(time);
  Serial.print(" DHT11 Temp: "); Serial.print(dhtError ? "Error" : String(tempDHT));
  Serial.print("°C Humidity: "); Serial.print(dhtError ? "Error" : String(hum));
  Serial.print("% Light: "); Serial.print(vemlError ? "Error" : String(lux));
  Serial.print(" lux Thermistor Temp: "); Serial.print(steinhart);
  Serial.println("°C");

  // Sleep for 2 seconds (simulate low power mode)
  delay(2000);
}

void signalError() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LEDPIN, HIGH);
    delay(100);
    digitalWrite(LEDPIN, LOW);
    delay(100);
  }
}

void disableEthernetController() {
  pinMode(10, OUTPUT); // Set the Ethernet CS pin as output
  digitalWrite(10, HIGH); // Disable Ethernet controller
}