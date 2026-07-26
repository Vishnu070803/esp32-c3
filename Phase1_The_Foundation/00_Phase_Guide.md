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

## 3. The Toolchain (From Linux to ESP32)
Your computer speaks x86_64. The ESP32-C3 speaks RISC-V. We need a "cross-compiler" to translate your code.
In the Rust ecosystem, this is beautiful and seamless compared to C/C++.

1.  **The Compiler Target:** We tell Rust to compile for `riscv32imc-unknown-none-elf` (bare-metal RISC-V).
2.  **The Flasher (`espflash`):** A Rust utility that talks over `/dev/ttyACM0` to the ESP32-C3's ROM bootloader and writes your compiled `.bin` file into the SPI Flash memory.

### Action Items for You:
1.  **Install the Rust Target:**
    ```bash
    rustup target add riscv32imc-unknown-none-elf
    ```
2.  **Install the Tools:**
    ```bash
    cargo install cargo-generate
    cargo install espflash
    ```
3.  **Create your first project:**
    Inside this `Phase1` directory, run:
    ```bash
    cargo generate esp-rs/esp-template
    ```
    *   **Project Name:** `hello_esp`
    *   **MCU:** `esp32c3`
    *   **Configure advanced:** `false`
4.  **Run it!**
    ```bash
    cd hello_esp
    cargo run
    ```
    This will compile the bare-metal code, flash it over USB, and open a serial monitor so you can see the `println!("Hello world!");` output.
