# ESP-IDF: Installation, Creating Projects, and Flashing

> **Official Reference:** [ESP-IDF Programming Guide for ESP32-C3](https://docs.espressif.com/projects/esp-idf/en/stable/esp32c3/get-started/index.html)

This document is your guide to the **standard, official way** to program the ESP32-C3 using Espressif's own framework — **ESP-IDF (Espressif IoT Development Framework)**. Everything we learn here is the industry standard for commercial ESP32 development.

---

## What is the ESP-IDF Toolchain?

When you write a program on your Linux machine and click "build", the ESP-IDF system orchestrates multiple tools:

```
Your C Code (.c files)
       │
       ▼
  GCC/LLVM Compiler  ← Cross-Compiler: translates your C to RISC-V binary
  (riscv32-esp-elf-gcc)
       │
       ▼
    CMake / Ninja     ← Build System: manages what to compile and link
       │
       ▼
  .bin + .elf Files   ← Your compiled firmware binary
       │
       ▼
  esptool.py          ← Flashing Tool: writes the binary to the board over USB
       │
       ▼
   ESP32-C3 Flash     ← Your code now lives on the chip!
```

All of this is installed and managed by the **EIM (ESP-IDF Installation Manager)**, a single tool Espressif created to eliminate the pain of manually installing toolchains.

---

## Part 1: Installation on Linux (Ubuntu/Debian)

> **Source:** [Official Linux Installation Guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32c3/get-started/linux-setup.html)

### Step 1: Install System Prerequisites

These are Linux libraries the ESP-IDF toolchain depends on:

```bash
sudo apt-get update
sudo apt-get install -y git wget flex bison gperf python3 python3-pip \
  python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util \
  libusb-1.0-0
```

> **Why these packages?**
> - `git` — EIM clones the ESP-IDF source code from GitHub.
> - `cmake` + `ninja-build` — The build system that compiles your C code.
> - `python3` + `pip` — ESP-IDF uses Python extensively for its build scripts and the `esptool.py` flasher.
> - `libusb-1.0-0` — The Linux USB library that `esptool.py` uses to talk to `/dev/ttyACM0`.

---

### Step 2: Install the EIM (ESP-IDF Installation Manager)

The EIM is the recommended way to install ESP-IDF v6.0 and newer. It manages all the toolchain versions for you, similar to how `rustup` manages Rust.

Add Espressif's APT repository and install:

```bash
echo "deb [trusted=yes] https://dl.espressif.com/dl/eim/apt/ stable main" | \
  sudo tee /etc/apt/sources.list.d/espressif.list

sudo apt update
sudo apt install eim
```

*(Alternatively, download the GUI installer directly from [dl.espressif.com/dl/eim/](https://dl.espressif.com/dl/eim/))*

---

### Step 3: Install ESP-IDF Using EIM

Run the EIM CLI to install the latest stable version of ESP-IDF for the ESP32-C3 target:

```bash
eim install
```

The installer will:
1. Download the ESP-IDF source code (from GitHub).
2. Download the pre-compiled RISC-V GCC toolchain.
3. Download supporting tools (`esptool.py`, `openocd`, etc.).
4. Create a Python virtual environment with all required Python packages.
5. Print you an **activation command** at the end. It will look like:

```
============================================
to activate the environment, run:
    source "/home/svishnu/.espressif/tools/activate_idf_v6.0.2.sh"
============================================
```

**Copy that command!** You will need it every time you open a new terminal to work with ESP-IDF.

---

### Step 4: Activate the Environment

Every time you open a new terminal to work on ESP32 projects, you **must activate** the ESP-IDF environment first. This adds the cross-compiler (`riscv32-esp-elf-gcc`) and `idf.py` to your PATH:

```bash
source "/home/svishnu/.espressif/tools/activate_idf_v6.0.2.sh"
```

> **Tip:** Add this line to your `~/.bashrc` file to auto-activate every time you open a terminal, but be aware it slightly slows down shell startup.

After sourcing, verify it's working:

```bash
idf.py --version
# Expected output: ESP-IDF v6.0.2
```

---

## Part 2: Understanding the `idf.py` CLI

`idf.py` is your one-stop command for everything in ESP-IDF. It is a Python wrapper around CMake, Ninja, and esptool.

| Command | What It Does |
| :--- | :--- |
| `idf.py set-target esp32c3` | Tells the build system your target chip is the ESP32-C3 (RISC-V) |
| `idf.py menuconfig` | Opens an interactive text-based menu to configure your project |
| `idf.py build` | Compiles all source code and generates `.bin` files |
| `idf.py -p /dev/ttyACM0 flash` | Flashes the compiled `.bin` onto the chip via USB |
| `idf.py -p /dev/ttyACM0 monitor` | Opens a serial monitor to read `printf()` output from the chip |
| `idf.py -p /dev/ttyACM0 flash monitor` | Builds, flashes, AND opens the monitor in one command |
| `idf.py fullclean` | Deletes the `build/` directory and all compiled output (fresh rebuild) |

---

## Part 3: Creating Your First Hello World Project

> **Source:** [Start a Project on Linux from Command Line](https://docs.espressif.com/projects/esp-idf/en/stable/esp32c3/get-started/linux-macos-start-project.html)

### Step 1: Copy the Hello World Example

ESP-IDF ships with dozens of example projects. Let's copy the hello_world example to our Phase 1 working directory:

```bash
# Make sure the environment is activated first!
cp -r $IDF_PATH/examples/get-started/hello_world ~/esp32-c3/Phase1_The_Foundation/hello_world
```

> **What is `$IDF_PATH`?** This is an environment variable set by the activation script. It points to the directory where ESP-IDF was installed (e.g., `~/.espressif/frameworks/esp-idf-v6.0.2/`).

---

### Step 2: Look Inside the ESP-IDF Project Structure

An ESP-IDF application is structured as a collection of modular **components**. Here is the standard ESP-IDF project layout and the purpose of every file:

```
my_esp32_project/
├── CMakeLists.txt              ← [REQUIRED] Root CMake file (registers ESP-IDF build system)
├── main/                       ← [REQUIRED] Main application component directory
│   ├── CMakeLists.txt          ← [REQUIRED] Component CMake file (registers source files for 'main')
│   └── hello_world_main.c      ← [REQUIRED] C source code containing app_main() entry point
├── components/                 ← [OPTIONAL] Custom or 3rd-party reusable components
│   └── my_sensor_driver/
│       ├── CMakeLists.txt
│       ├── include/
│       │   └── sensor.h
│       └── sensor.c
├── sdkconfig                   ← [AUTO-GENERATED] Global Kconfig configuration file
├── sdkconfig.old               ← [AUTO-GENERATED] Backup of previous configuration
├── build/                      ← [AUTO-GENERATED] Output artifacts (binaries, object files, elf)
│   ├── bootloader/             ← Compiled 2nd-stage bootloader binary
│   ├── partition_table/        ← Compiled partition table binary
│   └── hello_world.bin         ← Final application binary flashed to chip
├── partitions.csv              ← [OPTIONAL] Custom flash memory layout definition
└── README.md
```

#### Detailed Breakdown of Every Key File:

1. **Root `CMakeLists.txt`**
   * **Role:** Tells CMake that this directory is an ESP-IDF project.
   * **Minimal Content:**
     ```cmake
     cmake_minimum_required(VERSION 3.16)
     include($ENV{IDF_PATH}/tools/cmake/project.cmake)
     project(hello_world)
     ```
   * **Rule:** Must be run at the root of the project directory where `idf.py` is invoked.

2. **`main/` Component Directory**
   * **Role:** Every ESP-IDF project has a default primary component called `main`.
   * **`main/CMakeLists.txt`:** Registers the source files and header include paths for the `main` component.
     ```cmake
     idf_component_register(SRCS "hello_world_main.c"
                            INCLUDE_DIRS ".")
     ```
   * **`hello_world_main.c`:** Contains `void app_main(void)`—the entry point function called by ESP-IDF's FreeRTOS initialization task after boot.

3. **`components/` Directory (Optional)**
   * **Role:** Contains custom modular libraries (drivers, utilities). ESP-IDF treats each subfolder with a `CMakeLists.txt` as a decoupled component that can be reused across multiple projects.

4. **`sdkconfig` File**
   * **Role:** Generated automatically by `idf.py set-target` or `idf.py menuconfig`. It stores hundreds of system settings (CPU clock frequency, watchdog timeouts, Wi-Fi configuration, FreeRTOS tick rate, log levels).
   * **Rule:** Never manually edit `sdkconfig` while learning; use `idf.py menuconfig` to edit it visually.

5. **`build/` Directory**
   * **Role:** Contains all compiled output. Key outputs generated after `idf.py build`:
     * `build/bootloader/bootloader.bin` → 2nd stage bootloader flashed at `0x0`
     * `build/partition_table/partition-table.bin` → Flash layout map flashed at `0x8000`
     * `build/hello_world.bin` → Application binary flashed at `0x10000`
     * `build/hello_world.elf` → Symbol file used by GDB debugger and `idf.py monitor` for stack traces.
   * **Rule:** This directory can be safely deleted at any time with `idf.py fullclean`.

6. **`partitions.csv` (Optional)**
   * **Role:** Defines how the physical SPI Flash memory (e.g., 4MB) is divided into partitions (Bootloader, Partition Table, NVS storage, PHY calibration data, Factory App, OTA update slots).

7. **Linker Scripts (`.ld` Files)**
   * **Role:** Tells the GCC linker (`riscv32-esp-elf-ld`) exact memory addresses for code sections (`.text` in Flash, `.data`/`.bss` in SRAM, vector tables, ROM symbols).
   * **Why are there multiple `.ld` files instead of just one?**
     * **`memory.ld` (Physical Memory Map):** Defines *where* physical SRAM and Flash memory start (`ORIGIN`) and how large they are (`LENGTH`). This changes depending on board variant/Flash size.
     * **`sections.ld` (Code Placement Rules):** Defines *where* C code sections (`.text` -> Flash, `.data`/`.bss` -> SRAM) are mapped. This stays identical across chip variants. `sections.ld` uses `INCLUDE memory.ld` to combine them.
     * **Modularity Industry Standard:** Decoupling hardware memory sizes from software code rules is standard practice across ARM Cortex-M, RISC-V, and embedded systems to avoid duplicating section rules for every board.
   * **Why are there two pairs (`app` vs `bootloader`)?**
     * ESP-IDF builds two standalone binaries per project:
       1. **Bootloader (`bootloader.memory.ld` + `bootloader.sections.ld`):** Runs first out of ROM using a tiny slice of SRAM.
       2. **App (`memory.ld` + `sections.ld`):** Runs second after handoff, utilizing the full SRAM and Flash.
   * **Where they live:**
     * **Generated App Linker Scripts:**
       * [build/esp-idf/esp_system/ld/memory.ld](file:///home/svishnu/esp32-c3/Phase1_The_Foundation/hello_world/build/esp-idf/esp_system/ld/memory.ld)
       * [build/esp-idf/esp_system/ld/sections.ld](file:///home/svishnu/esp32-c3/Phase1_The_Foundation/hello_world/build/esp-idf/esp_system/ld/sections.ld)
     * **Generated Bootloader Linker Scripts:**
       * `build/bootloader/ld/bootloader.memory.ld`
       * `build/bootloader/ld/bootloader.sections.ld`
     * **Framework Source Templates:**
       * `~/.espressif/v6.0.2/esp-idf/components/esp_system/ld/esp32c3/memory.ld.in`
       * `~/.espressif/v6.0.2/esp-idf/components/esp_system/ld/esp32c3/sections.ld.in`

Open `main/hello_world_main.c`:

```c
#include <stdio.h>
#include "freertos/FreeRTOS.h"    // ← FreeRTOS OS is always included!
#include "freertos/task.h"         // ← Task functions (vTaskDelay)
#include "esp_chip_info.h"
#include "esp_log.h"

static const char *TAG = "hello_world";

void app_main(void)               // ← This is your "main()" entry point
{
    ESP_LOGI(TAG, "Hello world!");

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    ESP_LOGI(TAG, "This is %s chip with %d CPU core(s)",
             CONFIG_IDF_TARGET,
             chip_info.cores);

    for (int i = 10; i >= 0; i--) {
        ESP_LOGI(TAG, "Restarting in %d seconds...", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // ← Delay 1 second (non-blocking)
    }
    esp_restart();
}
```

> **Key Insight:** Notice that even in a simple Hello World, `FreeRTOS` is already included and running! `vTaskDelay()` is a FreeRTOS function. ESP-IDF always runs on top of FreeRTOS — there is no "bare metal" mode in ESP-IDF. The `app_main()` function itself runs inside a FreeRTOS task.

---

### Step 3: Set the Target Chip

Navigate to the project and tell ESP-IDF we're building for the ESP32-C3:

```bash
cd ~/esp32-c3/Phase1_The_Foundation/hello_world
idf.py set-target esp32c3
```

This generates a `build/` directory and a `sdkconfig` file configured for the RISC-V ESP32-C3 toolchain.

---

### Step 4: (Optional) Configure with menuconfig

```bash
idf.py menuconfig
```

This opens a terminal UI. For Hello World, you don't need to change anything. Press `Q` to quit, then `N` when asked to save changes.

> **When do you use menuconfig?** When configuring Wi-Fi credentials, enabling/disabling drivers (like Bluetooth), setting the CPU clock speed, configuring flash size, or enabling logging verbosity.

---

### Step 5: Build the Project

```bash
idf.py build
```

This will take **several minutes the first time** because it compiles all of ESP-IDF's components (Wi-Fi driver, FreeRTOS, networking stacks, etc.) along with your code. Subsequent builds only recompile files you changed.

Expected output at the end:
```
[527/527] Generating hello_world.bin
Project build complete. To flash, run:
  idf.py -p PORT flash
```

This produces three binary files in the `build/` directory:
- `bootloader/bootloader.bin` → Written to Flash at address `0x0`
- `partition_table/partition-table.bin` → Written to Flash at address `0x8000`
- `hello_world.bin` → Your application, written at address `0x10000`

---

## Part 4: Flashing the Board

### Step 1: Verify the Serial Port

Your board is at `/dev/ttyACM0` (confirmed in Phase 0).

### Step 2: Flash!

```bash
idf.py -p /dev/ttyACM0 flash
```

The `idf.py flash` command internally calls `esptool.py`, which:
1. Sends a special DTR/RTS signal over the USB serial connection to force the board into **Download Mode** automatically.
2. Negotiates the baud rate (up to 460800 bps for fast flashing).
3. Erases the relevant Flash sectors.
4. Writes the three binary files to the correct Flash addresses.
5. Sends a reset command to restart the board into **Normal Boot Mode**, which runs your new code.

> **If flashing fails with `Path '/dev/ttyACM0' is not readable`:**
> This happens because Linux group permission changes require a new shell session. Fix it immediately in your terminal:
> ```bash
> newgrp dialout
> source "/home/svishnu/.espressif/tools/activate_idf_v6.0.2.sh"
> # OR temporarily grant permission directly:
> sudo chmod 666 /dev/ttyACM0
> ```

---

## Part 5: Monitor the Output

```bash
idf.py -p /dev/ttyACM0 monitor
```

This opens a serial terminal at 115200 baud. To exit IDF monitor, press **`Ctrl + ]`**.

> **One-shot command:** Combine build, flash, and monitor into one command:
> ```bash
> idf.py -p /dev/ttyACM0 flash monitor
> ```

---

## Part 6: Reading Real ESP32-C3 Boot Logs Line-by-Line

When you connect the serial monitor, here is what actual live ESP32-C3 boot logs look like, annotated with what each component is doing under the hood:

```text
ESP-ROM:esp32c3-api1-20210207
Build:Feb  7 2021
rst:0x15 (USB_UART_CHIP_RESET),boot:0xc (SPI_FAST_FLASH_BOOT)
```
* **`ESP-ROM`**: The permanent factory silicon ROM executes at hardware address `0x0`.
* **`rst:0x15`**: Cause of reset = `USB_UART_CHIP_RESET` (the flashing tool automatically toggled the DTR/RTS reset pin after flashing).
* **`boot:0xc`**: Strapping pins (GPIO9 HIGH) instructed the ROM to boot normally from SPI Flash.

```text
I (24) boot: ESP-IDF v6.0.2 2nd stage bootloader
I (44) boot: Partition Table:
I (53) boot:  0 nvs              WiFi data        01 02 00009000 00006000
I (59) boot:  1 phy_init         RF data          01 01 0000f000 00001000
I (66) boot:  2 factory          factory app      00 00 00010000 00100000
I (123) boot: Loaded app from partition at offset 0x10000
```
* **`I (24)`**: Log entry at 24 milliseconds after power-on. The 2nd stage bootloader started.
* Reads the Partition Table at `0x8000` and locates your application at `0x10000`.

```text
I (150) cpu_start: cpu freq: 160000000 Hz
I (188) heap_init: Initializing. RAM available for dynamic allocation:
I (194) heap_init: At 3FC8B640 len 000349C0 (210 KiB): RAM
```
* CPU clock speed configured to 160 MHz.
* FreeRTOS heap memory manager initialized 210 KB of SRAM for dynamic `malloc()` allocations.

```text
I (249) main_task: Started on CPU0
I (249) main_task: Calling app_main()
Hello world!
This is esp32c3 chip with 1 CPU core(s), WiFi/BLE, silicon revision v0.4, 2MB external flash
Restarting in 10 seconds...
```
* At 249 ms, FreeRTOS launches `main_task` on CPU0.
* FreeRTOS calls `app_main()` in your C code.
* Your C code prints `Hello world!`, counts down from 10, calls `esp_restart()`, and restarts the chip!

---

## Official References

| Resource | Link |
| :--- | :--- |
| ESP-IDF Get Started | [docs.espressif.com](https://docs.espressif.com/projects/esp-idf/en/stable/esp32c3/get-started/index.html) |
| ESP-IDF Linux Installation | [linux-setup.html](https://docs.espressif.com/projects/esp-idf/en/stable/esp32c3/get-started/linux-setup.html) |
| Linux Project Start Guide | [linux-macos-start-project.html](https://docs.espressif.com/projects/esp-idf/en/stable/esp32c3/get-started/linux-macos-start-project.html) |
| idf.py Reference | [build-system.html](https://docs.espressif.com/projects/esp-idf/en/stable/esp32c3/api-guides/build-system.html) |
| esptool.py Reference | [esptool.io](https://docs.espressif.com/projects/esptool/en/latest/) |
| ESP-IDF Examples on GitHub | [github.com/espressif/esp-idf/tree/master/examples](https://github.com/espressif/esp-idf/tree/master/examples) |
