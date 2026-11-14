# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 3

**DATE:** 27/07/2026 (Week 2)

**NAME OF EXP:** Timer-Based LED Pattern Generator and Button Interrupt Controller

**HARDWARE SETUP / PLATFORM:** AT89S52 development board (8051 architecture, 11.0592 MHz crystal) + 4 LEDs + push-button — Keil µVision 5 (C51), progisp + USBasp ISP programmer

---

## ACTIVITY : Four-State LED Control System

### 1. Circuit Diagram

| Signal | Pin Mapping |
|---|---|
| LED1 / LED2 / LED3 / LED4 | P1.1 / P1.3 / P1.5 / P1.7 (active-HIGH via 330 Ω resistors) |
| Push-button input | Matrix keypad switch: column P2.5 driven LOW, row P2.0 sampled |

![Circuit Diagram](circuit.png)


### 2. Core Logic / Pseudocode / Code Base

**How it works**

The `mode` variable selects the active state. `leds_write(pattern)` translates the four pattern bits to Port 1 outputs, from LED1 at bit 3 to LED4 at bit 0. The main loop checks `button_pressed_event()` every 10 ms. Two matching samples establish a state change over a 20 ms debounce window, and only the press edge changes mode. The animation advances every 30 ticks (300 ms), so the loop can keep checking the button while the pattern runs.

| State (`mode`) | LED1 LED2 LED3 LED4 Sequence |
|---|---|
| 0 (Idle) | `0000` (all LEDs off at reset) |
| 1 (Running) | `1000` → `0100` → `0010` → `0001` |
| 2 (Binary count) | `0000` → … → `1111` |
| 3 (Alternating) | `1010` ↔ `0101` |
| 4 (Flash all) | `1111` ↔ `0000` |

**Pseudocode:**

1. Drive the keypad column line LOW, turn all four LEDs OFF, and initialise `mode = 0`.
2. On every 10 ms tick, poll `button_pressed_event()`; when a new press edge is detected, advance `mode` (wrapping from 4 back to 1) and reset `step = 0`.
3. Every 30 ticks (300 ms), evaluate `pattern_for(mode, step)` and output the 4-bit pattern to Port 1.
4. Increment `step` (wrapping modulo 4 in Running mode and modulo 16 in Binary mode).

**Code Base:**

- Keil project resources include `TASK3A.uvproj` (Keil µVision project), `STARTUP.A51`, and the compiled image `Objects/TASK3A.hex`.
- `main.c` — implements `leds_write()`, `button_pressed_event()`, and `pattern_for()`
- `delay.c/.h` — reusable `delay_ms()` timing library from CAT 2
- `TASK3A.uvproj`

### 3. Inference

The 20 ms debounce filter recognizes a single press edge, and the state machine uses that event to advance one LED mode.
