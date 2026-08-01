# Implement GPIO Toggle and WS2812 RGB LED Control

This phase focuses on interacting with the physical hardware. Based on your excellent suggestion, we will break this into two parts:
1. **Simple GPIO Toggle:** We will take a standard pin (e.g., GPIO7) and toggle it high (3.3V) and low (0V) slowly so you can verify the voltage changes using a multimeter.
2. **WS2812 RGB LED:** We will use the dedicated RMT (Remote Control) peripheral to send the complex high-speed protocol required to light up the smart LED on GPIO8.

Following our "Parallel C and Rust" methodology, we will implement both of these concepts in C (ESP-IDF) first, and then port them to pure bare-metal Rust.

## Proposed Changes

### Part 1: Simple GPIO Toggle (Multimeter Test)
We will create a project that toggles GPIO7 HIGH for 2 seconds, then LOW for 2 seconds.
- **In C (`gpio_toggle_c`):** We will use `gpio_config()` and `gpio_set_level()` from the ESP-IDF driver.
- **In Rust (`gpio_toggle_rust`):** We will configure `peripherals.GPIO7` as an `Output` using `esp-hal` and use `.set_high()` / `.set_low()`.

### Part 1.5: Direct Memory-Mapped Register Access (Raw C)
To truly understand what the ESP-IDF driver is doing under the hood, we will write a new C project (`gpio_raw_c`). We will bypass `driver/gpio.h` completely. We will use the ESP32-C3 Technical Reference Manual to find the exact memory addresses for the IO_MUX and GPIO peripheral blocks. We will use raw C pointers to directly manipulate the 32-bit hardware registers (`GPIO_ENABLE_REG`, `GPIO_OUT_REG`) to configure and toggle the Red LED.

### Part 2: WS2812 RGB LED via RMT (Remote Control) Peripheral
We will create a project to drive the smart LED on GPIO8.

> [!CAUTION]
> The WS2812 uses a strict microsecond pulse-width protocol. Software bit-banging is too slow and inaccurate. We must use the hardware RMT peripheral to generate the precise waveforms.

Following your established "No Spoon Feeding, True Architect" methodology, we will tackle this in two steps:

**Phase 2.1: The ESP-IDF Architecture (C)**
Before we touch the silicon, we will use the ESP-IDF `driver/rmt_tx.h` API. This will teach us the high-level architecture of the RMT block (Channels, Encoders, Transmit Symbols) and verify our LED is working.

**Phase 2.2: Bare-Metal RMT (Raw C)**
Once we understand the architecture, we will open the Technical Reference Manual (TRM). We will locate the RMT base address, configure the clock dividers (`RMT_SYS_CONF_REG`), set up the transmitter (`RMT_CHnCONF0_REG`), write our raw pulse-width timings directly into the RMT RAM block, and manually trigger the transmission.

## Verification Plan

### Manual Verification
1. **ESP-IDF Validation:** Flash the Phase 2.1 code to verify the LED cycles Red, Green, and Blue. This proves our hardware is healthy.
2. **Bare-Metal Validation:** Flash the Phase 2.2 raw C code. If the LED lights up with the exact same colors, we have successfully conquered one of the most complex hardware peripherals on the ESP32-C3 from scratch.
