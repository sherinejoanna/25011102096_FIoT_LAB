# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 10

**DATE:** 05/10/2026 (Tasks 10C/10d: 12/10/2026)

**NAME OF EXP:** STM32–Wemos Wi-Fi Modem Interface Using AT Commands + HTTP Server

**HARDWARE SETUP / PLATFORM:** Blue Pill STM32F103C8T6 (ARM Cortex-M3, 72 MHz) + Wemos D1 mini (ESP8266 AT firmware) + USB-to-TTL serial adapter — STM32CubeMX + Keil MDK-ARM (STM32CubeIDE project included), STM32 HAL, PuTTY

---

## TASK 10C / 10D : Configure ESP8266 as a TCP Server, HTTP Response

### 1. Circuit Diagram

| STM32 Pin | Target Connection | Role |
|---|---|---|
| `PA9` (`USART1_TX`) | Wemos `RX` | Transmit AT commands to ESP8266 |
| `PA10` (`USART1_RX`) | Wemos `TX` | Receive AT responses from ESP8266 |
| `PA2` (`USART2_TX`) / `PA3` (`USART2_RX`) | USB-to-TTL `RX` / `TX` | Debug console to PC (PuTTY) |
| `GND` | Shared ground rail | Common reference |

Set `USART1` and `USART2` to **115200-8-N-1**. The firmware reads the Wi-Fi settings from `Core/Inc/wifi_secrets.h`, which Git ignores. See `wifi_config.h` for the macro definitions.

![Circuit Diagram](circuit.png)


[View the data flow diagram](dataflow.svg)

### 2. Core Logic / Pseudocode / Code Base

**How it works**

Send `AT+CIPMUX=1` to enable multiple connections and `AT+CIPSERVER=1,80` to listen on port 80. The modem delivers a browser request as `+IPD,<id>,<len>:GET /path HTTP/1.1`. The `esp_at.c` interrupt handler buffers `USART1` input, and `on_esp_event()` records the URI for connection `<id>` when it receives a `+IPD` notification. The foreground loop assembles the HTTP/1.1 response as follows.

```
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: <strlen(body)>
Connection: close

<html><body><h1>STM32 IoT Node</h1>...</body></html>
```
To send the response, issue `AT+CIPSEND=<id>,<len>`, wait for `>`, write the raw HTTP bytes, and wait for `SEND OK`. Then close the connection with `AT+CIPCLOSE=<id>`. The `/led/on` and `/led/off` routes also control the `PC13` LED.

**Pseudocode:**

1. Initialise the modem, connect to Wi-Fi, enable multi-link server mode on port 80, and print the server URL.
2. In the main loop, pump incoming ESP lines and route unsolicited events to `on_esp_event()`.
3. For each connection link with a pending request, construct the HTML page (or `404 Not Found`) and send it over `CIPSEND`.
4. If a `WIFI DISCONNECT` event occurs, wait for the link to re-associate and re-enable the TCP server.

**Code Base:**

- Keil project resources include `CAT10D_HTTP_Server_Static_Page.ioc` (STM32CubeMX configuration), `MDK-ARM/CAT10D_HTTP_Server_Static_Page.uvprojx` (Keil MDK), `.project` / `.cproject` (STM32CubeIDE); firmware entry point in `Core/Src/main.c`.
- `esp_at.c/.h` — interrupt-driven RX ring buffer, timeout management, and `esp_at_send()`
- `USART1` NVIC interrupt enabled in `stm32f1xx_it.c`, `stm32f1xx_hal_msp.c`, and `.ioc`
- `handle_request()` and `http_send()`

**Expected result**

```
Server running: http://192.168.x.x/
[HTTP] link 0  GET /
[HTTP] link 1  GET /favicon.ico
```

### 3. Inference

The STM32 uses the connection ID from each `+IPD` frame to route the browser request and returns an HTML response with a matching `Content-Length`.


---
