# Deep Dive: Mapping ESP32-C3 Boot Logs to Linker Scripts, C Source, and Memory Addresses

Every single line printed in the serial terminal corresponds to a specific hardware state, a C source file in ESP-IDF, a section in the linker scripts (`memory.ld` / `sections.ld`), or an address in physical RAM/Flash.

Here is the complete line-by-line architectural breakdown.

---

## 1. Phase 1: Hardware ROM & Strapping Pins (0ms)

```text
ESP-ROM:esp32c3-api1-20210207
Build:Feb  7 2021
rst:0xc (RTC_SW_CPU_RST),boot:0xc (SPI_FAST_FLASH_BOOT)
```

| Log Element | Meaning | Source File / Hardware Component | Linker / Memory Mapping |
| :--- | :--- | :--- | :--- |
| `ESP-ROM` | 1st-stage ROM bootloader execution | Built-in Mask ROM on silicon | Executing at ROM address `0x40000000` |
| `rst:0xc` | Reset reason: Software CPU Reset | [components/esp_system/port/soc/esp32c3/system_internal.c](~/.espressif/v6.0.2/esp-idf/components/esp_system/port/soc/esp32c3/system_internal.c) | Reads RTC controller hardware register |
| `boot:0xc` | Strapping pins state (`GPIO9=HIGH`) | Hardware GPIO sampling circuit | Selects SPI Flash Boot mode (`SPI_FAST_FLASH_BOOT`) |

---

## 2. Phase 2: ROM Loading 2nd Stage Bootloader (0ms - 24ms)

```text
load:0x3fcd5830,len:0x155c
load:0x403cbf10,len:0xce0
load:0x403ce710,len:0x2fcc
entry 0x403cbf1a
```

| Log Element | Meaning | Source File / Component | Linker / Memory Mapping |
| :--- | :--- | :--- | :--- |
| `load:0x3fcd5830` | ROM copies bootloader `.data` & `.bss` to SRAM | ROM Bootloader | Target: `bootloader.memory.ld` -> `iram_loader_seg` (`0x3FCD5830`) |
| `load:0x403cbf10` | ROM copies bootloader code to SRAM IRAM | ROM Bootloader | Target: `bootloader.memory.ld` -> `iram_seg` (`0x403CBF10`) |
| `entry 0x403cbf1a` | ROM jumps to 2nd stage bootloader entry point | [components/bootloader/subproject/main/bootloader_start.c](~/.espressif/v6.0.2/esp-idf/components/bootloader/subproject/main/bootloader_start.c) | Function `call_start_cpu0()` at `0x403CBF1A` |

---

## 3. Phase 3: 2nd Stage Bootloader & Partition Table (24ms - 72ms)

```text
I (24) boot: ESP-IDF v6.0.2 2nd stage bootloader
I (43) boot: Partition Table:
I (52) boot:  0 nvs              WiFi data        01 02 00009000 00006000
I (59) boot:  1 phy_init         RF data          01 01 0000f000 00001000
I (65) boot:  2 factory          factory app      00 00 00010000 00100000
I (72) boot: End of partition table
```

| Log Element | Meaning | Source File / Component | Flash & Memory Mapping |
| :--- | :--- | :--- | :--- |
| `I (24) boot` | 2nd Stage Bootloader banner printed | `bootloader_start.c` -> `bootloader_main()` | Executing inside SRAM (`0x403C...`) |
| `Partition Table` | Reads flash memory map | [components/bootloader_support/src/bootloader_utility.c](~/.espressif/v6.0.2/esp-idf/components/bootloader_support/src/bootloader_utility.c) | Reads Flash address `0x8000` (`partition-table.bin`) |
| `factory app` | Identifies main app offset | `partitions.csv` / `sdkconfig` | Application binary located at Flash offset `0x10000` |

---

## 4. Phase 4: App Image Segment Mapping & Loading (75ms - 123ms)

```text
I (75) esp_image: segment 0: paddr=00010020 vaddr=3c010020 size=05c88h ( 23688) map
I (86) esp_image: segment 1: paddr=00015cb0 vaddr=3fc89000 size=0172ch (  5932) load
I (91) esp_image: segment 2: paddr=000173e4 vaddr=40380000 size=08c34h ( 35892) load
I (104) esp_image: segment 3: paddr=00020020 vaddr=42000020 size=0d794h ( 55188) map
I (114) esp_image: segment 4: paddr=0002d7bc vaddr=40388c34 size=001c0h (   448) load
I (114) esp_image: segment 5: paddr=0002d984 vaddr=50000000 size=00024h (    36) load
I (123) boot: Loaded app from partition at offset 0x10000
```

This is where your compiled binary (`hello_world.bin`) is loaded into memory according to your **Linker Scripts** (`memory.ld` and `sections.ld`):

| Segment | Flash Address (`paddr`) | RAM Address (`vaddr`) | Section Name | Linker Script Mapping (`sections.ld` & `memory.ld`) |
| :---: | :---: | :---: | :---: | :--- |
| **0 (map)** | `0x00010020` | `0x3C010020` | `.flash.rodata` | Read-only constants & string literals mapped to Flash DROM (`drom0_0_seg`). Read via MMU cache! |
| **1 (load)**| `0x00015CB0` | `0x3FC89000` | `.dram0.data` | Initialized global/static variables copied from Flash into SRAM (`dram0_0_seg`). |
| **2 (load)**| `0x000173E4` | `0x40380000` | `.iram0.text` | Critical fast functions & ISRs loaded directly into Instruction RAM (`iram0_0_seg`). |
| **3 (map)** | `0x00020020` | `0x42000020` | `.flash.text` | Your main application C code instructions mapped to Flash IROM (`irom0_0_seg`). Executed directly from Flash via XIP cache! |
| **4 (load)**| `0x0002D7BC` | `0x40388C34` | `.iram0.data` | Instruction RAM literal data tables. |
| **5 (load)**| `0x0002D984` | `0x50000000` | `.rtc.text` | Ultra-low-power RTC RAM memory segment for sleep mode retention. |

> **Source File:** [components/bootloader_support/src/esp_image_format.c](~/.espressif/v6.0.2/esp-idf/components/bootloader_support/src/esp_image_format.c) -> `esp_image_load()`

---

## 5. Phase 5: CPU & FreeRTOS Initialization (141ms - 242ms)

```text
I (141) cpu_start: Unicore app
I (150) cpu_start: cpu freq: 160000000 Hz
I (188) heap_init: Initializing. RAM available for dynamic allocation:
I (194) heap_init: At 3FC8B640 len 000349C0 (210 KiB): RAM
I (242) sleep_gpio: Enable automatic switching of GPIO sleep configuration
```

| Log Element | Meaning | Source File / Component | Linker / Memory Mapping |
| :--- | :--- | :--- | :--- |
| `cpu_start` | CPU initialization & clock scaling | [components/esp_system/port/cpu_start.c](~/.espressif/v6.0.2/esp-idf/components/esp_system/port/cpu_start.c) | Sets CPU clock to 160MHz via system registers |
| `heap_init` | Dynamic Memory Allocator setup | [components/heap/heap_caps_init.c](~/.espressif/v6.0.2/esp-idf/components/heap/heap_caps_init.c) | Scans remaining unused SRAM (between `0x3FC8B640` and `0x3FCC0000`) defined in `memory.ld` for `malloc()` |
| `sleep_gpio` | Low-power GPIO state configuration | [components/esp_system/sleep_gpio.c](~/.espressif/v6.0.2/esp-idf/components/esp_system/sleep_gpio.c) | Configures GPIO isolation modes |

---

## 6. Phase 6: FreeRTOS Task Launch & `app_main()` Execution (249ms+)

```text
I (249) main_task: Started on CPU0
I (249) main_task: Calling app_main()
Hello world!
This is esp32c3 chip with 1 CPU core(s), WiFi/BLE, silicon revision v0.4, 2MB external flash
```

| Log Element | Meaning | Source File | Function Name |
| :--- | :--- | :--- | :--- |
| `main_task: Started` | FreeRTOS scheduler creates initial task | [components/esp_system/port/cpu_start.c](~/.espressif/v6.0.2/esp-idf/components/esp_system/port/cpu_start.c) | `main_task()` |
| `Calling app_main()` | FreeRTOS hands control over to your application code | [components/esp_system/port/cpu_start.c](~/.espressif/v6.0.2/esp-idf/components/esp_system/port/cpu_start.c) | Calls `app_main()` |
| `Hello world!` | **Your application executes!** | [main/hello_world_main.c](../Phase1_The_Foundation/hello_world/main/hello_world_main.c) | `ESP_LOGI(TAG, "Hello world!");` / `printf()` |

---

## Complete Execution Flow Diagram

```
[ Power On ]
    │
    ▼
1st Stage ROM (0x40000000)
    │  Reads Strapping Pins (GPIO9=HIGH)
    ▼
Loads 2nd Stage Bootloader into SRAM (0x3FCD5830 / 0x403CBF10)
    │  Executes bootloader_start.c
    ▼
Reads Partition Table at Flash 0x8000
    │  Locates factory app at Flash 0x10000
    ▼
Maps & Loads App Segments (esp_image_format.c)
    ├── .flash.rodata ➔ Mapped to DROM (0x3C010020) via MMU
    ├── .dram0.data   ➔ Loaded to SRAM (0x3FC89000)
    ├── .iram0.text   ➔ Loaded to IRAM (0x40380000)
    └── .flash.text   ➔ Mapped to IROM (0x42000020) via MMU XIP
    │
    ▼
CPU Start & Heap Init (cpu_start.c & heap_caps_init.c)
    │  Configures 160MHz Clock & 210KB Dynamic RAM Heap
    ▼
FreeRTOS Kernel Starts ➔ Launches main_task()
    │
    ▼
Calls app_main() in [hello_world_main.c](../Phase1_The_Foundation/hello_world/main/hello_world_main.c)
    │
    ▼
"Hello world!" printed to Console (UART 0 / USB CDC)
```

---

## Summary Reference Table

| Log Phase | Memory Type | Address Range | Linked Section in `sections.ld` | Relevant C File |
| :--- | :--- | :--- | :--- | :--- |
| **ROM Boot** | Mask ROM | `0x40000000` | N/A (Hardware ROM) | Silicon Factory ROM |
| **2nd Stage Boot** | SRAM (IRAM) | `0x403CBF10` | `bootloader.memory.ld` | [bootloader_start.c](~/.espressif/v6.0.2/esp-idf/components/bootloader/subproject/main/bootloader_start.c) |
| **Partition Table** | SPI Flash | `0x00008000` | `partition-table.bin` | [bootloader_utility.c](~/.espressif/v6.0.2/esp-idf/components/bootloader_support/src/bootloader_utility.c) |
| **App Read-Only Data**| Flash DROM | `0x3C010020` | `.flash.rodata` | [esp_image_format.c](~/.espressif/v6.0.2/esp-idf/components/bootloader_support/src/esp_image_format.c) |
| **App RAM Variables** | Internal SRAM | `0x3FC89000` | `.dram0.data` | [esp_image_format.c](~/.espressif/v6.0.2/esp-idf/components/bootloader_support/src/esp_image_format.c) |
| **App Code Instructions**| Flash IROM | `0x42000020` | `.flash.text` | [esp_image_format.c](~/.espressif/v6.0.2/esp-idf/components/bootloader_support/src/esp_image_format.c) |
| **FreeRTOS Heap** | Internal SRAM | `0x3FC8B640` | `dram0_0_seg` | [heap_caps_init.c](~/.espressif/v6.0.2/esp-idf/components/heap/heap_caps_init.c) |
| **User Application** | Flash/SRAM | `app_main` | `.flash.text` | [main/hello_world_main.c](../Phase1_The_Foundation/hello_world/main/hello_world_main.c) |
