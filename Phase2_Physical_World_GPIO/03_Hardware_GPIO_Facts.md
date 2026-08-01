# Hardware GPIO Facts & Discoveries

This document logs the language-agnostic hardware facts and architectural concepts we discover about the ESP32-C3 silicon, separate from C or Rust implementations.

## 1. Memory-Mapped Register Layout
The CPU controls the GPIO hardware by reading and writing to specific 32-bit physical RAM addresses.
*   **GPIO Peripheral Base Address:** `0x6000_4000`
*   **`GPIO_ENABLE_REG`** (Offset `0x0020`): Each bit configures whether a specific pin is an output driver (1) or not (0).
*   **`GPIO_OUT_REG`** (Offset `0x0004`): Each bit sets the physical voltage of the pin (1 = 3.3V, 0 = 0V).

## 2. The Read-Modify-Write Race Condition
When you use a standard bitwise operation in software (e.g., `*GPIO_OUT_REG |= (1 << 7)`), the CPU executes three separate instructions:
1.  **READ:** Fetches the 32-bit register from hardware.
2.  **MODIFY:** Performs the OR math in the CPU.
3.  **WRITE:** Pushes the new 32-bit value back to hardware.

**The Danger:** If a hardware interrupt (like Wi-Fi) pauses the CPU between Step 1 and Step 3, modifies a *different* GPIO pin, and returns control, your software will overwrite and undo the interrupt's change in Step 3 because it is writing back stale data.

## 3. Atomic Hardware Writes (`W1TS` and `W1TC`)
To solve the race condition, hardware designers built dedicated "Write 1 To Set" and "Write 1 To Clear" registers.
*   **`GPIO_OUT_W1TS_REG`** (Offset `0x0008`): Write `1` to Set.
*   **`GPIO_OUT_W1TC_REG`** (Offset `0x000C`): Write `1` to Clear.

**How they work:**
These registers execute in a **single atomic CPU cycle**. 
When you write a 32-bit number to these registers:
*   Any bit that is `1` instantly forces the corresponding pin HIGH (in `W1TS`) or LOW (in `W1TC`).
*   Any bit that is `0` is **completely ignored** by the hardware, leaving those pins untouched. 
This guarantees thread-safety and interrupt-safety without needing software locks!

## 4. The IO MUX Switchboard (`IO_MUX_GPIOn_REG`)
Before a signal reaches a pin, it must pass through the IO MUX. The IO MUX decides *which* internal hardware block owns the pin (e.g., JTAG, UART, or GPIO).
*   **Base Address:** `0x6000_9000`
*   **`MCU_SEL` (Bits 12-14):** The router switch. Set to `1` to connect the pin to the GPIO subsystem.
*   **`FUN_DRV` (Bits 10-11):** Drive strength. Controls how much current the pin can source (~5mA to ~40mA).
*   **`FUN_IE` (Bit 9):** Input Enable. Must be `1` to allow the CPU to read voltage from the pin. For a pure output, set to `0` to blind the CPU reading circuitry.
*   **`FUN_WPU` / `FUN_WPD` (Bits 8, 7):** Internal Weak Pull-Up / Pull-Down resistors.
*   **`FILTER_EN` (Bit 15):** Activates a hardware glitch filter (debouncer) for physical buttons.

## 5. The GPIO Matrix Router (`GPIO_FUNCn_OUT_SEL_CFG_REG`)
Once the IO MUX connects a pin to the GPIO subsystem, the GPIO Matrix decides *what* data is sent out.
*   **Index 0-127:** Forces the pin to output a signal directly from an internal hardware peripheral (like a UART TX line), completely ignoring software commands.
*   **Index 128 (`0x80`):** "Simple Software Control". Forces the pin to listen to our `GPIO_OUT_REG` and `GPIO_ENABLE_REG` software bits.
*   **`OEN_SEL` (Bit 9):** Output Enable Select. If `1`, it forces the pin's input/output direction to be controlled by our software `GPIO_ENABLE_REG`. If `0`, internal hardware rapidly flips the direction.

## 6. The Output Gate vs. The Voltage Generator
To successfully output a voltage, you need *both* of these registers to work together:
*   **`GPIO_ENABLE_REG` (The Door):** Configures the physical direction. If `1`, the door is open and voltage can flow out. If `0`, the pin is strictly an input.
*   **`GPIO_OUT_REG` (The Generator):** Holds the actual voltage state (1 = 3.3V).
*   **Why both?** Even if `GPIO_OUT_REG` is generating 3.3V, if the `GPIO_ENABLE_REG` door is closed, the outside world will see absolutely nothing.
