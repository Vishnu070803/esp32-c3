# Phase 2: Interacting with the Physical World (GPIO)

Now that we can run code on the RISC-V core, it's time to interact with the outside world. The most fundamental way a microcontroller does this is through **GPIO** (General Purpose Input/Output) pins.

## 1. What is a GPIO?
A GPIO is a physical pin on the edge of the chip. Your software can configure these pins as:
*   **Digital Output:** Act like a switch. You write a `1` (HIGH) in code, and the physical pin outputs 3.3 Volts. You write a `0` (LOW), and it outputs 0 Volts (GND). This can turn on LEDs, relays, or motors.
*   **Digital Input:** Act like a sensor. You read the pin in code. If there is 3.3V on the pin, your code reads `1`. If it's connected to GND, your code reads `0`. Used for buttons and switches.

At the hardware architecture level, GPIOs are controlled by **Memory Mapped Registers**. To turn a pin HIGH, the CPU literally writes a 32-bit number to a specific RAM address (e.g., `0x6000_4000`) that is physically wired to the GPIO hardware block.

## 2. The WS2812 "Smart" RGB LED
Your `esp-rs` board has a built-in WS2812 LED connected to **GPIO8**. 
Unlike a standard LED that just needs constant 3.3V to turn on, the WS2812 is a "Smart" LED. It requires a specific digital data signal to tell it what color to be.

*   **The Protocol (NZR):** To send a color, the ESP32-C3 must rapidly toggle GPIO8 HIGH and LOW at very precise microsecond timings to send 24 bits of data (8 bits Red, 8 bits Green, 8 bits Blue).
*   **RMT (Remote Control Peripheral):** Toggling a pin that fast using pure software `loop {}` is unreliable. The ESP32-C3 has a dedicated hardware block called the **RMT**. You give the RMT a block of memory representing the timings, and the hardware handles the ultra-fast pin toggling for you, freeing up the CPU!

## 3. The `esp-hal` (Hardware Abstraction Layer)
In Rust, we don't manually write memory addresses like `0x6000_4000` (which is tedious and error-prone). We use the `esp-hal` crate. It provides safe, easy-to-use Rust functions like `.set_high()` and `.set_low()` that handle the underlying register writes.

### Action Items for You:
1.  Inside this directory, generate a new project to control the LED. (You can copy the `hello_esp` project from Phase 1 to start).
2.  Your task in this phase will be to look into the `esp-hal` documentation and figure out how to configure **GPIO8** and use the **RMT** peripheral to send a color (like Red or Green) to the WS2812 LED on your board.
3.  *Hint: You may want to use a community crate like `smart-leds` alongside the `esp-hal` RMT driver to make this easy.*
