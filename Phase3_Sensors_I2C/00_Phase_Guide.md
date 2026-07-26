# Phase 3: Communicating with Sensors (I2C Protocol)

GPIOs are great for simple ON/OFF signals, but what if a sensor needs to send complex data, like a temperature reading of 24.5°C? We need a communication protocol. The most famous in embedded systems is **I2C** (Inter-Integrated Circuit).

## 1. The I2C Protocol Architecture
I2C allows a master (your ESP32-C3) to talk to multiple slave devices (sensors) using only **two wires**:
*   **SDA (Serial Data):** The wire that the data is sent across.
*   **SCL (Serial Clock):** The wire that synchronizes the timing. The master ticks this clock up and down.

**How it works (The Conversation):**
1.  **Address:** Every sensor on the bus has a unique 7-bit physical address. The ESP32-C3 shouts: *"Hey, device 0x70, are you there?"*
2.  **ACK:** The sensor responds by pulling the SDA line low (Acknowledge).
3.  **Command:** The ESP32-C3 says: *"Read your temperature register."*
4.  **Data:** The sensor sends the bytes representing the temperature back over SDA.

## 2. The SHTC3 Environment Sensor
Your `esp-rs` board has an SHTC3 sensor built-in.
*   It measures both Temperature and Relative Humidity.
*   It connects to the ESP32-C3 via the I2C bus.
*   Internal Architecture: The SHTC3 contains a tiny analog temperature probe, an ADC (Analog-to-Digital Converter) to turn the analog voltage into a digital number, and an I2C interface to send that number to the ESP32.

## 3. Pull-up Resistors
I2C requires the SDA and SCL lines to be held at 3.3V by default. This is done using physical "Pull-up Resistors" wired between the 3.3V power and the data lines. Your `esp-rs` board already has these soldered on for you!

### Action Items for You:
1.  Create a project in this folder to talk to the SHTC3.
2.  Find out which GPIO pins on the `esp-rs` board are routed to the I2C SDA and SCL lines.
3.  Use the `i2c` module in `esp-hal` to initialize the bus.
4.  You can write raw I2C commands reading the SHTC3 datasheet, OR you can use an existing Rust driver crate for the SHTC3 to read the temperature and print it to your terminal!
