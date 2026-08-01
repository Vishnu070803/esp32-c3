# C Experiments and Logs

## Experiment 1: The `gpio_toggle_c` Project (ESP-IDF Driver)

### Objective
Toggle GPIO7 high (3.3V) and low (0V) using the standard `driver/gpio.h` API to verify basic hardware interaction.

### The Discovery
During manual multimeter testing, we discovered that **GPIO7** is physically hardwired to a Red status LED on the ESP32-C3-DevKit-RUST board. Our code successfully blinked this LED!

### Key Facts Learned
1. **The `app_main` function:** It is a FreeRTOS task. It returns `void`. Using `return -1;` or `exit();` will cause compilation errors or undefined behavior. If a setup step fails, you simply use `return;` to exit the task, leaving the rest of the OS running.
2. **Configuration vs Execution:** Hardware registers should be configured (`gpio_reset_pin`, `gpio_set_direction`) **once** outside the main loop. Only the voltage toggling (`gpio_set_level`) belongs inside the `while(1)` loop.
3. **FreeRTOS Delays:** Standard C `sleep()` cannot be used. We must use `vTaskDelay(pdMS_TO_TICKS(milliseconds))` to yield CPU time back to the RTOS.

### Build Logs
```text
Executing action: all (aliases: build)
Running ninja in directory /home/svishnu/esp32-c3/Phase2_Physical_World_GPIO/c_gpio_toggle/build
Executing "ninja all"...
...
Generated /home/svishnu/esp32-c3/Phase2_Physical_World_GPIO/c_gpio_toggle/build/bootloader/bootloader.bin
...
Generated /home/svishnu/esp32-c3/Phase2_Physical_World_GPIO/c_gpio_toggle/build/c_gpio_toggle.bin
c_gpio_toggle.bin binary size 0x23380 bytes. Smallest app partition is 0x100000 bytes. 0xdcc80 bytes (86%) free.

Project build complete. To flash, run:
 idf.py flash
```

## Experiment 2: Bare-Metal GPIO Toggle

### Objective
Toggle GPIO7 high and low by writing directly to the physical silicon registers, entirely bypassing the ESP-IDF `driver/gpio.h` black box.

### The Discovery
We successfully reconfigured the IO MUX switchboard, the GPIO Matrix router, and the GPIO driver gates using direct pointer dereferencing in C (`*(volatile uint32_t *)`). The LED blinked exactly as it did with the ESP-IDF drivers, but we had 100% control over the hardware logic.

### Key Facts Learned
1. **The C Pointer Arithmetic Trap:** You cannot add a raw byte offset (like `0x570`) directly to a `uint32_t *` base address. In C, pointer arithmetic multiplies the added number by the size of the type (4 bytes). Base addresses and offsets must be kept as raw integers and only cast to a pointer at the final step: `((volatile uint32_t *)(BASE + OFFSET))`.
2. **Bitwise NOT (`~`) Pitfall:** Defining masks using `~` (e.g. `~ (1 << 9)`) flips *all* other 31 bits to `1`. Using the bitwise OR (`|`) with this mask will corrupt the entire register. When setting exact bits, a direct assignment (`=`) is much safer than Read-Modify-Write because it naturally forces all unmentioned bits to `0`.
3. **Commenting Registers:** When writing bare-metal code, every bitfield operation must be heavily commented so future developers understand *why* a specific bit is being flipped.

### Build Logs
```text
Executing "ninja flash"...
Writing 'c_gpio_toggle.bin' at 0x00010000...
Wrote 141680 bytes (75941 compressed) at 0x00010000 in 0.9 seconds (1242.5 kbit/s).
Hard resetting via RTS pin...
...
GPIO 7 is high 
GPIO 7 is low 
```

## Experiment 3: The `c_RMT_RGB_toggle` Project (RMT Peripheral)

### Objective
Control the WS2812 "Smart" RGB LED attached to GPIO 2. The WS2812 requires strict microsecond electrical pulses to transmit 24-bit color data. Because FreeRTOS software toggling is not fast or precise enough, we must use the ESP32's **Remote Control (RMT) Peripheral** to generate the hardware signals.

### The Procedure
1. **The Target:** We ran `idf.py set-target esp32c3` to ensure ESP-IDF compiles for the RISC-V architecture.
2. **The Blueprint:** We initialized `rmt_tx_channel_config_t` with 10MHz resolution (1 tick = 100ns) and 48 memory symbols.
3. **The Data Packing:** We packed our colors into `0xGGRRBB` format. Since the WS2812 expects the Green byte first, and demands the Most Significant Bit first, extracting the bits iteratively using `(color >> (23 - i)) & 1` perfectly satisfied the hardware protocol without needing complex `if/else` logic.
4. **The Transmission:** We instantiated a Copy Encoder to move our standard C array directly into the hardware's tiny DMA memory, then fired `rmt_transmit()`.

### Common Errors Encountered
*   `missing braces around initializer`: ESP-IDF v5/v6 wraps `rmt_symbol_word_t` in a union. Using brace initialization (e.g. `{{{8, 1, 4, 0}}}`) is dangerous and confusing. **Solution:** Use explicit Designated Initializers (`.duration0 = 8`).
*   `DMA not supported`: The ESP32-C3 does not support RMT DMA. We encountered this because we did not zero-initialize our configuration struct (`rmt_tx_channel_config_t rmt_tx;`), leaving garbage data on the stack that accidentally enabled the `flags.with_dma` boolean. **Solution:** Always zero-initialize structs (`= {}`).
*   `Wrong chip argument`: Flashing failed because the project was defaulting to the original ESP32 target instead of `esp32c3`.

### Key Code Concepts
*   `rmt_new_tx_channel`: Claims physical hardware.
*   `rmt_new_copy_encoder`: Creates an engine to translate standard arrays into hardware registers.
*   `rmt_transmit`: Pulls the trigger to dump the memory into the wire.
