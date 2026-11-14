# CAT15 – Smart Indoor Environment Monitoring Using BME280, STM32 and MQTT

## Experiment Details

- **Platform:** STM32F401RE (NUCLEO-F401RE configuration in the supplied project)
- **Controller:** STM32F401RE
- **Sensor:** BME280
- **Wireless interface:** ESP8266 using the supplied communication library
- **MQTT broker configured in firmware:** `broker.emqx.io:1883`
- **MQTT topic configured in firmware:** `stm32/subi/data`

## Objective

Use the STM32F401RE to collect temperature, pressure, and relative humidity from a BME280. Encode the readings as JSON and publish them to an MQTT broker via the ESP8266.

## Hardware and Software

- STM32F401RE development board
- BME280 sensor module
- ESP8266 module with a suitable 3.3 V supply
- Jumper wires and USB connection for programming/debugging
- STM32CubeMX and the supplied Keil MDK project

## GPIO and Peripheral Connections

| STM32F401RE signal | Connected device | Function |
|---|---|---|
| PB6 / I2C1_SCL | BME280 SCL | I2C clock |
| PB7 / I2C1_SDA | BME280 SDA | I2C data |
| PA9 / USART1_TX | ESP8266 RX | STM32-to-ESP8266 serial data |
| PA10 / USART1_RX | ESP8266 TX | ESP8266-to-STM32 serial data |
| GND | BME280 and ESP8266 GND | Common reference |

The pin assignments for PB6/PB7 and PA9/PA10 come from the included CubeMX and HAL files. Check the power pins and voltage requirements for the breakout boards in use.

## Working Principle

1. The firmware initializes the HAL, system clock, GPIO, I2C1 and USART1 peripherals.
2. The BME280 driver is initialized over I2C and configured to measure temperature, pressure and humidity.
3. The ESP8266 library is initialized and attempts to connect to the Wi-Fi network configured in the source code.
4. The firmware opens a TCP connection to `broker.emqx.io` on port `1883` and initializes the MQTT client.
5. After a successful sensor read, the firmware formats temperature, pressure and humidity as a JSON payload.
6. The payload is published to `stm32/subi/data` approximately every two seconds.

## MQTT Payload Format

```json
{
  "temperature": 26.40,
  "pressure_hpa": 1008.20,
  "humidity": 58.00
}
```

The sample values illustrate the JSON format.

## Current Implementation Status

This folder includes the STM32 application, BME280 driver, ESP8266/MQTT library, CubeMX configuration, and Keil project. It covers the firmware and MQTT publishing setup. A Node-RED dashboard is outside the included deliverables. An independent build and a hardware test of sensor readout and MQTT delivery are still pending.

## Files

- `bme_project.ioc` – STM32CubeMX configuration
- `Core/` – application source and HAL configuration files
- `Drivers/` – CMSIS and STM32 HAL dependencies
- `bme280.c`, `Core/Inc/bme280.h`, `Core/Inc/bme280_defs.h` – BME280 driver
- `ESPLib.c`, `ESPLib.h` and related headers – ESP8266/MQTT communication support
- `MDK-ARM/` – Keil project and startup assembly
- `circuit.png` – signal-level connection diagram

## Inference

The firmware reads the BME280 over I²C and uses UART to send the readings through the ESP8266. Separate JSON fields let subscribers handle temperature, pressure, and humidity independently. Successful delivery depends on correct wiring, valid Wi-Fi credentials, and a working path from the ESP8266 to the configured broker.
