# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 13

**DATE:** 27/10/2026

**NAME OF EXP:** BME280 Sensor Interfacing Using I²C and MQTT using Custom Built ESP8266 API

**HARDWARE SETUP / PLATFORM:** NUCLEO-F401RE (STM32F401RET6, ARM Cortex-M4F, 84 MHz) + Bosch BME280 + Wemos D1 mini — STM32CubeMX + Keil MDK-ARM (STM32CubeIDE project included), STM32 HAL, ESPLib API (clean-room implementation), Node-RED, `broker.emqx.io`

---

## TASK 13C : BME280 JSON Telemetry over MQTT & Node-RED Dashboard

### 1. Circuit Diagram

Reuse the CAT 12 wiring. CAT 13B also drives `LD2` on `PA5`. The MQTT endpoint is **`broker.emqx.io:1883`**.

![Circuit Diagram](circuit.png)


![Data flow](dataflow.svg)

### 2. Core Logic / Pseudocode / Code Base

**How it works**

As in CAT 12, initialize the Bosch BME280 over I²C1. Every two seconds, put the compensated temperature, humidity, and pressure readings into the following JSON payload.
`{"status":"success","temperature":28.45,"humidity":62.30,"pressure":1008.72}` and published to topic `subithrasri/stm32/bme280` at QoS 0.

**Pseudocode:**

1. Initialise the BME280 (`2×` oversampling, IIR filter off, Normal mode).
2. Connect to Wi-Fi and the MQTT broker via `mqtt_start()`.
3. In the main loop: read the sensor, format the payload with `build_json()`, publish to the broker, log to `USART6`, and wait 2 s.

**Code Base:**

- Keil project resources include `CAT13C_BME280_MQTT_NodeRED.ioc` (STM32CubeMX configuration), `MDK-ARM/CAT13C_BME280_MQTT_NodeRED.uvprojx` (Keil MDK), `.project` / `.cproject` (STM32CubeIDE); firmware entry point in `Core/Src/main.c`.
- `build_json()`, `bme_setup()`, `bme_read()`, and `fmt_2dp()`

**Expected result**

```
PUB subithrasri/stm32/bme280 {"status":"success","temperature":28.45,"humidity":62.30,"pressure":1008.72}
```

**ESPLib API Reference:**

| ESPLib Call | Underlying AT / MQTT Traffic |
|---|---|
| `ESP8266_Init(StationMode, huart1)` | Sends `AT` and `AT+CWMODE=1` |
| `ESP8266_wifiConnect()` | Sends `AT+CWQAP` and `AT+CWJAP` |
| `ESP8266_portConnect("TCP", broker, "1883")` | Opens TCP socket via `AT+CIPSTART` |
| `MQTT_connectBroker()` | Sends MQTT `CONNECT` (`0x10`, protocol `"MQTT"` level 4, clean session, keep-alive, client ID) |
| `MQTT_publishTopic()` | Sends MQTT `PUBLISH` (`0x30`, topic length + string, payload) |
| `MQTT_subscribeTopic()` | Sends MQTT `SUBSCRIBE` (`0x82`) |

**Clean-Room Library Improvements:** `ESPLib.h` preserves the exact function names from the lab handout so student application code compiles without changes, while `ESPLib.c` and `mqtt_packet.c` provide a new implementation that resolves the flaws of the original course driver:

| Limitation in Original Driver | Behaviour in Reworked Implementation |
|---|---|
| Aborted the UART RX interrupt on every transmit, dropping any broker packets that arrived mid-send | Arms the UART RX interrupt once at startup into a 1 KB ring buffer and never aborts it |
| Inferred packet boundaries from 100 ms of UART silence; left `colonIndex` uninitialised on `SUBACK` / `PINGRESP` frames | Strips `+IPD,<len>:` framing first and feeds payload bytes into a stateful MQTT parser driven by packet type and Remaining Length |
| Handled only 1-byte Remaining Length without bounds checks or guaranteed NUL termination | Decodes variable-length Remaining Length (up to 4 bytes), validates buffer capacities prior to copying, guarantees NUL-terminated strings, and skips oversized packets |
| Ignored `CONNACK` / `SUBACK` responses and used unbounded `do {} while` loops that could hang indefinitely | Verifies `CONNACK` acceptance and `SUBACK ≠ 0x80`; enforces timeouts on every wait and returns `funcOk` / `funcErr` |

The client expects `PINGRESP` after sending `PINGREQ` and reconnects on a timeout. It counts and discards oversized topics and logs receive overflows or packet drops as their counters increase.

Publish and subscribe calls support **QoS 0**. Requests for QoS 1 or 2 return `funcErr`, and the broker must grant QoS 0 in `SUBACK`. The client discards and counts incoming QoS 1 or 2 publishes. It checks that `UNSUBACK` matches the `UNSUBSCRIBE` request.

A UART receive overflow or invalid `+IPD` header causes the driver to flush buffered bytes, reset the parser, and invalidate both the TCP socket and MQTT session so it can reconnect. The foreground loop retries failed `HAL_UART_Receive_IT` re-arming. The client validates the `CONNACK` Session Present flag and rejects publish topics containing wildcards.

Run `make -C tests` for 54 packet parser checks and 56 ESPLib checks. The simulated ESP8266 exercises both `AT+CIPSEND` reply sequences, split and combined TCP segments, unmatched ACK IDs, broker timeouts, and receive overflows in the middle of a packet.

### 3. Inference

Publishing the BME280 data as JSON lets multiple MQTT subscribers receive the live readings from the same topic.

---

## NEXT STEP : Node-RED

### 1. Circuit Diagram

Reuse the CAT 12 wiring. CAT 13B also drives `LD2` on `PA5`. The MQTT endpoint is **`broker.emqx.io:1883`**.

![Circuit Diagram](circuit.png)


![Data flow](dataflow.svg)

### 2. Core Logic / Pseudocode / Code Base

**How it works**

- `NodeRED_CAT13_MQTT_flow.json`: subscribes via an **mqtt in** node (configured to output a parsed JSON object), routes the payload through a `function` node that splits temperature, humidity, and pressure onto separate debug outputs, computes rolling min/max/average statistics, and provides `ON` / `OFF` / `TOGGLE` inject buttons wired to an **mqtt out** node for controlling CAT 13B.
- `NodeRED_CAT13_Dashboard_flow.json`: feeds live temperature, humidity, and pressure streams into three dashboard gauges and a historical line chart (requires `node-red-dashboard`, accessible at `/ui`).

**Pseudocode:**

1. Import either flow JSON into Node-RED and click **Deploy**.
2. Observe the live sensor telemetry updating every 2 s in the Debug sidebar or `/ui` dashboard.

**Code Base:**

- `NodeRED_CAT13_MQTT_flow.json`
- `NodeRED_CAT13_Dashboard_flow.json`

**ESPLib API Reference:**

| ESPLib Call | Underlying AT / MQTT Traffic |
|---|---|
| `ESP8266_Init(StationMode, huart1)` | Sends `AT` and `AT+CWMODE=1` |
| `ESP8266_wifiConnect()` | Sends `AT+CWQAP` and `AT+CWJAP` |
| `ESP8266_portConnect("TCP", broker, "1883")` | Opens TCP socket via `AT+CIPSTART` |
| `MQTT_connectBroker()` | Sends MQTT `CONNECT` (`0x10`, protocol `"MQTT"` level 4, clean session, keep-alive, client ID) |
| `MQTT_publishTopic()` | Sends MQTT `PUBLISH` (`0x30`, topic length + string, payload) |
| `MQTT_subscribeTopic()` | Sends MQTT `SUBSCRIBE` (`0x82`) |

**Clean-Room Library Improvements:** `ESPLib.h` preserves the exact function names from the lab handout so student application code compiles without changes, while `ESPLib.c` and `mqtt_packet.c` provide a new implementation that resolves the flaws of the original course driver:

| Limitation in Original Driver | Behaviour in Reworked Implementation |
|---|---|
| Aborted the UART RX interrupt on every transmit, dropping any broker packets that arrived mid-send | Arms the UART RX interrupt once at startup into a 1 KB ring buffer and never aborts it |
| Inferred packet boundaries from 100 ms of UART silence; left `colonIndex` uninitialised on `SUBACK` / `PINGRESP` frames | Strips `+IPD,<len>:` framing first and feeds payload bytes into a stateful MQTT parser driven by packet type and Remaining Length |
| Handled only 1-byte Remaining Length without bounds checks or guaranteed NUL termination | Decodes variable-length Remaining Length (up to 4 bytes), validates buffer capacities prior to copying, guarantees NUL-terminated strings, and skips oversized packets |
| Ignored `CONNACK` / `SUBACK` responses and used unbounded `do {} while` loops that could hang indefinitely | Verifies `CONNACK` acceptance and `SUBACK ≠ 0x80`; enforces timeouts on every wait and returns `funcOk` / `funcErr` |

The client expects `PINGRESP` after sending `PINGREQ` and reconnects on a timeout. It counts and discards oversized topics and logs receive overflows or packet drops as their counters increase.

Publish and subscribe calls support **QoS 0**. Requests for QoS 1 or 2 return `funcErr`, and the broker must grant QoS 0 in `SUBACK`. The client discards and counts incoming QoS 1 or 2 publishes. It checks that `UNSUBACK` matches the `UNSUBSCRIBE` request.

A UART receive overflow or invalid `+IPD` header causes the driver to flush buffered bytes, reset the parser, and invalidate both the TCP socket and MQTT session so it can reconnect. The foreground loop retries failed `HAL_UART_Receive_IT` re-arming. The client validates the `CONNACK` Session Present flag and rejects publish topics containing wildcards.

Run `make -C tests` for 54 packet parser checks and 56 ESPLib checks. The simulated ESP8266 exercises both `AT+CIPSEND` reply sequences, split and combined TCP segments, unmatched ACK IDs, broker timeouts, and receive overflows in the middle of a packet.

### 3. Inference

Use an `mqtt in` node to receive the topic in Node-RED. The dashboard shows the three readings on gauges and trend charts and calculates summary statistics.
