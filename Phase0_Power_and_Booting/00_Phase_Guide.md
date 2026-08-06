# Phase 0: Power, Booting, and First Connection

Welcome to Phase 0! Before writing a single line of code, we must understand how this piece of hardware operates at a physical level. Coming from a Linux environment, a microcontroller feels very different. There is no operating system (unless we add an RTOS), no terminal by default, and no bash shell. It's just bare silicon executing instructions from memory the moment it receives power.

## 1. Powering the Board
When you connect the ESP32-C3-DevKit-RUST board via USB to your computer, several things happen on the hardware level:
1. **5V USB Power:** The USB cable provides 5 Volts of power.
2. **Voltage Regulator (LDO):** The ESP32-C3 chip cannot handle 5V. It requires exactly 3.3V. The board has a small component called an LDO (Low Dropout) voltage regulator that steps the 5V down to a stable 3.3V.
3. **Power-On Reset (POR):** As soon as the chip detects stable 3.3V power, a hardware circuit triggers a "Power-On Reset", telling the internal processor to wake up and begin executing instructions from address `0x0`.

## 2. The Two Physical Buttons
Look closely at your board. You will see two tiny push buttons. These are your primary way of controlling the hardware state.

*   **EN / RESET Button (Usually marked `RST` or `EN`):** 
    *   This is physically wired to the `CHIP_PU` (Chip Power Up / Enable) pin of the ESP32-C3.
    *   When you press it, you pull the pin to `GND` (0 Volts), forcing the chip to turn off.
    *   When you release it, the pin springs back up to 3.3V, causing a "Hard Reset", exactly the same as unplugging and plugging the USB back in.
*   **BOOT Button (Usually marked `BOOT` or `IO9`):**
    *   This button is wired to **GPIO9** (General Purpose Input/Output Pin #9).
    *   This specific pin is a **Strapping Pin**. It has a special job during the split-second the chip turns on.

## 3. Boot Modes (How the chip decides what to do)
When the ESP32-C3 wakes up (either from being plugged in, or you releasing the RESET button), it looks at the voltage level on the **Strapping Pins** (primarily GPIO9, GPIO8, and GPIO2) to decide its **Boot Mode**. 

There are two main modes you care about:

| Boot Mode | Condition at Startup | What Happens? |
| :--- | :--- | :--- |
| **Normal Mode (SPI Boot)** | `BOOT` button is **NOT PRESSED** (GPIO9 is HIGH) | The chip looks at its internal/external Flash Memory, loads the application you previously flashed, and runs it. |
| **Download Mode (Flash Mode)** | `BOOT` button is **PRESSED** (GPIO9 is LOW) | The chip ignores the Flash memory. Instead, it starts an internal factory ROM bootloader. It waits patiently for your computer to send new firmware over the USB connection. |

**The Manual Flashing Dance:**
Usually, flashing tools (like `espflash` in Rust) are smart enough to send a special signal over USB to automatically trigger the Download Mode. But sometimes, especially on Linux or custom boards, this automatic reset fails. 
If a flashing tool says "Waiting for download mode...", you must do this manually:
1. Press and **HOLD** the `BOOT` button.
2. Press and **RELEASE** the `RESET` button.
3. **RELEASE** the `BOOT` button.
*(You just forced the chip to restart while GPIO9 was held low!)*

## 4. The Serial Connection (Linux Side)
The ESP32-C3 has a built-in **USB Serial/JTAG Controller**. This means when you plug it in, it talks directly to the USB driver on your Linux machine.

When plugged in, Linux will typically assign it a device file:
*   `/dev/ttyACM0` or `/dev/ttyACM1`
*   Sometimes `/dev/ttyUSB0` (if a secondary UART bridge is used).

On Windows, it is assigned a COM port:
*   `COMx` (e.g., `COM3`, `COM4`) which can be checked in Device Manager.

### Action Item for You:
Let's verify your Linux machine can see the board and that you have permission to talk to it.

1.  Open your terminal.
2.  Unplug your ESP32-C3 board, wait a second, and plug it back in.
3.  Run this command to see the latest kernel logs:
    ```bash
    dmesg | tail -n 20
    ```
    Look for lines mentioning `usb`, `Espressif`, or a `tty` device attached.
4.  Run this command to list serial devices:
    ```bash
    ls -l /dev/ttyACM* /dev/ttyUSB*   # Linux
    # [System.IO.Ports.SerialPort]::GetPortNames()   # Windows PowerShell
    ```
    On Linux, you should see an output like: `crw-rw---- 1 root dialout ... /dev/ttyACM0`. Note the group is usually `dialout`.
5.  **Crucial Step:** To flash the board without using `sudo` every time, your user must be in the `dialout` group. Check your groups by typing `groups`. If `dialout` is not there, run:
    ```bash
    sudo usermod -a -G dialout $USER
    ```
    *(Note: You usually have to log out and log back into Linux for this group change to take effect).*
