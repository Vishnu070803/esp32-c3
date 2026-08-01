# Phase 1: The Foundation (Understanding the Brain)

In Phase 0, we learned how the chip turns on. Now, let's explore what is actually inside the chip executing our code, and how we get our code onto it.

## 1. The RISC-V Architecture
Unlike Intel/AMD processors (x86) or most smartphones (ARM), the ESP32-C3 uses an open-source Instruction Set Architecture (ISA) called **RISC-V** (pronounced "Risk-Five").
*   **32-bit Single Core:** It processes data in 32-bit chunks and has only one core, meaning it does one thing at a time (no true parallel processing).
*   **RV32IMC:** This is the specific flavor of RISC-V in the ESP32-C3.
    *   **I:** Base Integer instructions (basic math).
    *   **M:** Hardware Multiplication and Division (speeds up math).
    *   **C:** Compressed instructions (shrinks code size so more fits in memory).

## 2. The Memory Map
When you write a program, where does it live? The ESP32-C3 doesn't have a hard drive. It has a **Memory Map**—a continuous range of addresses (0x0000_0000 to 0xFFFF_FFFF) assigned to physical hardware.
*   **Internal ROM (Read-Only Memory):** Contains the permanent factory bootloader (what runs in Download Mode).
*   **SRAM (Static RAM - 400KB):** Extremely fast memory used for your program's variables, stack, and heap. Loses data when powered off.
*   **SPI Flash (usually 4MB external to the chip, but inside the metal shield):** Where your actual compiled code is permanently stored. The chip executes code directly from this flash memory using a technique called eXecute-In-Place (XIP).

## 3. The Toolchains (C/ESP-IDF vs Rust)
Your computer speaks x86_64. The ESP32-C3 speaks RISC-V. We need a "cross-compiler" to translate your code.
Because we are following the **Parallel C and Rust Methodology**, we will use two different toolchains:

1. **The C Toolchain (ESP-IDF):** Uses GCC (`riscv32-esp-elf-gcc`), CMake, and `idf.py`. It always runs on top of FreeRTOS.
2. **The Rust Toolchain (`esp-rs`):** Uses LLVM (`riscv32imc-unknown-none-elf`), Cargo, and `espflash`. We use the `no_std` bare-metal approach, meaning there is no operating system—just your code and the hardware.

### Action Items for You:

#### Part A: The C Baseline (ESP-IDF)
We already explored this in Phase 0.5, but let's officially run it here!
1.  **Activate the Environment:** 
    ```bash
    source ~/.espressif/tools/activate_idf_v6.0.2.sh
    ```
2.  **Navigate to the C Project:** 
    ```bash
    cd ~/esp32-c3/Phase1_The_Foundation/hello_world
    ```
3.  **Build and Flash:**
    ```bash
    idf.py set-target esp32c3
    idf.py build
    idf.py -p /dev/ttyACM0 flash monitor
    ```

#### Part B: Porting to Embedded Rust
Now let's build the exact same "Hello World" logic in pure, bare-metal Rust.
1.  **Install the Rust Target:**
    ```bash
    rustup target add riscv32imc-unknown-none-elf
    ```
2.  **Install the Tools (in a clean terminal, NOT the ESP-IDF terminal!):**
    ```bash
    cargo install esp-generate
    cargo install espflash
    ```
3.  **Create your Rust project:**
    Inside this `Phase1` directory, run the new generator:
    ```bash
    esp-generate --chip esp32c3 hello_esp
    ```
    *   **Module Selection:** When prompted by the interactive menu, select **`esp32c3-mini-1`** (since your DevKit uses the Mini module) and press `s` to save and generate.
    *   *(You can accept the defaults for the rest of the features).*

4.  **Add the Print Macro (The Rust Way!):**
    Because we are in `#![no_std]` (bare-metal), there is no standard library `println!`. We use Espressif's custom crate to route text over the USB/UART.
    ```bash
    cd hello_esp
    cargo add esp-println --features esp32c3
    ```
    Open `src/bin/main.rs`, add `use esp_println::println;` at the top, and insert `println!("Hello from baremetal Rust!");` inside the infinite `loop {}` at the bottom.

5.  **Run it!**
    ```bash
    cargo run
    ```
    *   **Magic Note:** You don't need to specify the USB port! `espflash` (which is configured as the default runner in `.cargo/config.toml`) automatically detects your board, cross-compiles for RISC-V, flashes it, and opens the serial monitor.
    *   **Bootloader Note:** You will initially see about 200ms of ESP-IDF boot logs! This is normal. The Rust ecosystem stitches a tiny, pre-compiled ESP-IDF 2nd-stage bootloader to your binary to map the SPI Flash memory. Once it says `Disabling RNG early entropy source...`, it hands 100% control over to your pure bare-metal Rust code!
