# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 13

**DATE:** 27/10/2026

**NAME OF EXP:** BME280 Sensor Interfacing Using I²C and MQTT using Custom Built ESP8266 API

**HARDWARE SETUP / PLATFORM:** NUCLEO-F401RE (STM32F401RET6, ARM Cortex-M4F, 84 MHz) + Bosch BME280 + Wemos D1 mini — STM32CubeMX + Keil MDK-ARM (STM32CubeIDE project included), STM32 HAL, ESPLib API (clean-room implementation), Node-RED, `broker.emqx.io`

---

## TASK 13B : MQTT Subscribe LED Control

### 1. Circuit Diagram

Reuse the CAT 12 wiring. CAT 13B also drives `LD2` on `PA5`. The MQTT endpoint is **`broker.emqx.io:1883`**.

![Circuit Diagram](circuit.png)


![Data flow](dataflow.svg)

### 2. Core Logic / Pseudocode / Code Base

**How it works**

After a valid `CONNACK`, the Nucleo subscribes to `subithrasri/stm32/led` and checks `SUBACK`. The foreground loop calls `MQTTDataHandler()`, which returns `1` when it has a complete, valid `PUBLISH`. The exact payloads `"ON"`, `"OFF"`, and `"TOGGLE"` control `LD2` (`PA5`). Other payloads leave the LED as it is. The client sends `PINGREQ` every 60 s and reconnects and resubscribes if `PINGRESP` times out. `SysTick_Handler` still calls `MQTTTimer()` for lab handout compatibility. The parser frames packets independently of idle time.

> **Security note:** `broker.emqx.io` is a public test broker. Any client that knows the topic can send commands. For use beyond the lab, configure an authenticated broker and set `MQTT_USERNAME` and `MQTT_PASSWORD` in `wifi_secrets.h` so the `CONNECT` packet carries credentials.

**Pseudocode:**

1. Initialise `LD2`, run `mqtt_start()`, subscribe via `MQTT_subscribeTopic()`, and ensure `HAL_UART_Receive_IT()` is active.
2. In the main loop: call `MQTTDataHandler()`; when a message arrives, log the topic/payload and update `LD2`.
3. Every 60 s, transmit a keep-alive ping via `MQTT_pingReq()`.

**Code Base:**

- Keil project resources include `CAT13B_MQTT_Subscribe_LED_Control.ioc` (STM32CubeMX configuration), `MDK-ARM/CAT13B_MQTT_Subscribe_LED_Control.uvprojx` (Keil MDK), `.project` / `.cproject` (STM32CubeIDE); firmware entry point in `Core/Src/main.c`.
- `subscribe_commands()` — sends `SUBSCRIBE`, validates `SUBACK`, and handles automatic reconnection/resubscription
- `handle_message()`

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

Node-RED and other MQTT clients can operate the Nucleo LED by publishing to its command topic. The firmware reads the action from the incoming `PUBLISH` payload.
