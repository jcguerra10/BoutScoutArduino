
# Sensor Data Logger with SD Card and Error Handling

## Description

This project is a sensor data logging system designed to read data from multiple sensors, log it to an SD card, and display it on the serial monitor. It includes robust error handling and is optimized for battery-powered systems. Additionally, it disables the Ethernet controller to prevent SPI conflicts with the SD card.

### Features:
- **DHT11 Sensor**: Measures temperature and humidity.
- **VEML7700 Sensor**: Measures ambient light intensity in lux.
- **Thermistor**: Measures temperature using a resistor and thermistor in a voltage divider configuration.
- **RTC DS3231**: Provides real-time date and time for data logging.
- **SD Card Logging**: Stores sensor data in a CSV file on an SD card.
- **Error Handling**: Detects and signals errors for each sensor and the SD card.
- **LED Indicator**: Flashes the LED in case of an error.
- **Battery Optimization**: Minimizes power usage by disabling unused components.

---

## Components

1. **DHT11 Sensor**:
   - Measures temperature and humidity.
   - Connected to digital pin `6`.

2. **VEML7700 Sensor**:
   - Measures ambient light intensity.
   - Communicates via I2C.

3. **Thermistor**:
   - Measures temperature using a resistor-thermistor voltage divider.
   - Connected to analog pin `A0`.

4. **RTC DS3231**:
   - Provides real-time clock functionality.
   - Communicates via I2C.

5. **SD Card**:
   - Logs data in a CSV file.
   - Connected via SPI with Chip Select (CS) on `SS`.

6. **LED**:
   - Provides status indication.
   - Connected to digital pin `2`.

---

## Circuit Diagram

### Connections:

| Component   | Pin on Arduino  | Notes                        |
|-------------|-----------------|------------------------------|
| **DHT11**   | `6`             | Signal pin for DHT11.        |
| **Thermistor** | `A0`          | Analog input for voltage divider. |
| **VEML7700**| `SDA`, `SCL`    | I2C pins for communication.  |
| **RTC DS3231** | `SDA`, `SCL`  | Shared I2C bus.             |
| **SD Card** | `SS` (SPI CS)   | Uses SPI for communication.  |
| **LED**     | `2`             | Status LED.                 |

---

## How It Works

1. **Initialization**:
   - Initializes the DHT11, VEML7700, RTC DS3231, and SD card.
   - Disables the Ethernet controller to prevent conflicts with the SD card.

2. **Sensor Readings**:
   - Reads temperature and humidity from the DHT11.
   - Reads ambient light from the VEML7700.
   - Reads temperature from the thermistor using the Steinhart-Hart equation.

3. **Error Handling**:
   - Detects errors for each sensor and the SD card.
   - Flashes the LED 3 times to signal errors without stopping the program.

4. **Data Logging**:
   - Logs sensor data to a CSV file (`sensor_log.csv`) on the SD card in the format:
     ```
     Date,Time,DHT_Temp,DHT_Hum,VEML_Lux,Thermistor_Temp
     01/01/2024,12:00:00,25.0,60.0,300.5,23.5
     ```

5. **Battery Optimization**:
   - Minimizes power usage by turning off the LED when idle.
   - SD card operations are performed only when necessary.

---

## Error Signaling

- **LED Flashing**: The LED blinks 3 times to indicate an error with any sensor or the SD card.
- **Serial Monitor Logs**: Errors are also displayed on the serial monitor for debugging.

---

## Dependencies

- **Libraries Used**:
  - `SdFat` for SD card operations.
  - `RTClib` for RTC DS3231.
  - `Adafruit_VEML7700` for light sensor.
  - `DHT` for temperature and humidity.

---

## Installation

1. Install the required libraries via the Arduino Library Manager:
   - `SdFat`
   - `RTClib`
   - `Adafruit_VEML7700`
   - `DHT`

2. Upload the code to an Arduino-compatible board.

3. Connect the components as per the circuit diagram.

4. Insert a formatted SD card (FAT16/FAT32).

5. Open the Serial Monitor to view logs.

---

## Example Output

### Serial Monitor:

```
Date: 01/01/2024 Time: 12:00:00 DHT11 Temp: 25.0°C Humidity: 60.0% Light: 300.5 lux Thermistor Temp: 23.5°C
```

### SD Card File (`sensor_log.csv`):
```
Date,Time,DHT_Temp,DHT_Hum,VEML_Lux,Thermistor_Temp
01/01/2024,12:00:00,25.0,60.0,300.5,23.5
```

---

## Troubleshooting

- **SD Card Initialization Error**:
  - Ensure the SD card is formatted as FAT16/FAT32.
  - Check the CS pin connection.

- **Sensor Errors**:
  - Verify sensor wiring and power connections.

- **No Data Logged**:
  - Check the `sensor_log.csv` file on the SD card.

---

## Future Improvements

- Implement low-power modes (e.g., deep sleep) for longer battery life.
- Add a reset mechanism for recovering from persistent errors.

---

