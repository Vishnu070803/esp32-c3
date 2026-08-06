# ESP32-C3 Embedded Learning Journey

Welcome to the ESP32-C3 embedded learning repository! This project serves as a structured, phase-by-phase learning environment for transitioning from high-level software development (Linux/Web) into the world of bare-metal embedded systems.

## The Learning Methodology: Parallel C and Rust

This repository follows a unique **Parallel Implementation Strategy**:
1. **The C Baseline (Configure & Test):** For every hardware concept, we first write, configure, and test the code using standard **C** via the official ESP-IDF framework. This establishes a strong, industry-standard understanding of the hardware, FreeRTOS, and the build system.
2. **The Rust Port (Perform):** Once the C implementation is working, we port the exact same functionality to **Embedded Rust**. This allows us to directly compare the two paradigms and experience how Rust's compile-time safety and `no_std` (bare-metal) ecosystem handle the exact same hardware.

## The Hardware
This repository is tailored for the **ESP32-C3-DevKit-RUST** board (or similar ESP32-C3 boards). 
The ESP32-C3 is a single-core, 32-bit **RISC-V** microcontroller featuring integrated 2.4 GHz Wi-Fi and Bluetooth 5 (LE).

The `esp-rs` DevKit specifically includes:
*   **ICM-42670-P:** 6-Axis IMU (Accelerometer & Gyroscope)
*   **SHTC3:** Temperature and Humidity Sensor
*   **WS2812:** Addressable RGB "Smart" LED
*   **Li-Ion Charger:** Integrated battery charging circuit

## Project Structure
To prevent getting overwhelmed, the learning process is broken down into modular phases. Each phase has its own dedicated directory containing a `00_Phase_Guide.md` anchor document. You should read the anchor document to understand the underlying hardware architecture, and then use that directory as your workspace to write code and notes for the action items.

*   📁 **`Phase0_Power_and_Booting/`**
    *   Learn how the hardware physically turns on, the role of the LDO voltage regulator, Boot vs. EN buttons, and how the chip uses Strapping Pins to determine its boot mode. Verifying serial connections (`/dev/ttyACM0` on Linux, `COMx` on Windows).
*   📁 **`Phase0.5_Ecosystems_and_Tools/`**
    *   **"What, Where, and How?"** A complete breakdown of the programming environments: Arduino IDE, ESP-IDF (C/C++), MicroPython, and Rust. Includes complete guides on `idf.py` setup, ESP-IDF project structure, linker scripts (`memory.ld`/`sections.ld`), and an end-to-end mapping from live console boot logs to C source files and hardware memory addresses.
*   📁 **`Phase1_The_Foundation/`**
    *   Understand the RISC-V (RV32IMC) architecture, the Memory Map (Flash, SRAM, ROM), and how to set up the toolchain to cross-compile and flash your very first "Hello World" application.
*   📁 **`Phase2_Physical_World_GPIO/`**
    *   Dive into Memory Mapped Registers, GPIO digital signals, and driving the onboard WS2812 RGB LED using precise timing hardware (the RMT peripheral).
*   📁 **`Phase3_Sensors_I2C/`**
    *   Learn the I2C two-wire protocol (SDA/SCL) and how to communicate with the SHTC3 environment sensor to read real-world temperature and humidity.
*   📁 **`Phase4_Motion_Tracking/`**
    *   Explore MEMS technology with the ICM-42670-P IMU. Transition from "polling" sensors to using hardware Interrupts for efficient motion detection.
*   📁 **`Phase5_Wireless_Connectivity/`**
    *   Harness the ESP32-C3's Wi-Fi radio subsystem. Understand the difference between bare-metal networking and standard OS-level networking, and serve sensor data over the network.
*   📁 **`Phase6_FreeRTOS_and_Multitasking/`**
    *   **The OS Layer:** Transitioning from bare-metal "super loops" to a Real-Time Operating System (FreeRTOS). Learn about Tasks, Schedulers, Mutexes, and Inter-Process Communication (IPC).
*   📁 **`PhaseX_True_Bare_Metal/`** *(Not yet started — Future)*
    *   **🚧 Placeholder.** True bare metal programming: writing directly to hardware registers with no OS, no ESP-IDF, no FreeRTOS. Custom startup code, linker scripts, and Rust `no_std` via `esp-hal`. This phase will be written when we actually explore it hands-on.

## ⚠️ Scope and Coverage Note

> **True Bare Metal is NOT covered yet.**
>
> Throughout this repository, the term "bare metal" is used loosely to mean *"understanding the underlying hardware registers"*. In practice, **all current phases (0–6) use ESP-IDF, which always runs FreeRTOS underneath**. Even when we directly access GPIO registers in Phase 2, FreeRTOS is still running.
>
> **True bare metal** (zero OS, zero framework, custom startup vector, raw linker scripts, or Rust `no_std` `esp-hal`) is architecturally different and is reserved for a future phase (`PhaseX_True_Bare_Metal/`) when we have sufficient low-level understanding to do it meaningfully.

## Getting Started
1. Start at **Phase 0**. Open `Phase0_Power_and_Booting/00_Phase_Guide.md` and complete the hardware connection verification.
2. Move sequentially through the folders, completing the Action Items at the bottom of each guide.
3. Feel free to create new projects for both C (`idf.py create-project`) and Rust (`esp-generate`) directly inside each Phase directory as you practice!

## Prerequisites
### Linux
Ensure your user is part of the `dialout` group to allow flashing without `sudo`:
```bash
sudo usermod -a -G dialout $USER
```
*(Remember to log out and log back in for this to take effect!)*

### Windows
If you installed ESP-IDF using the Espressif Installer but want to use standard PowerShell instead of their shortcut, you can use the included activation script to bypass Windows Store Python aliases:
```powershell
. .\activate_idf.ps1
```
