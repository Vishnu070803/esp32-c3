# Implement I2C Communication and Sensor Reading

This phase focuses on interacting with external integrated circuits using a communication protocol. We will be communicating with the on-board SHTC3 Temperature and Humidity sensor via the I2C protocol.

Following our established methodology, we will progress through increasing levels of abstraction and different toolchains to deeply understand what is happening on the silicon.

## Proposed Changes

### Part 1: High-Level C Abstraction (ESP-IDF)
We will create a project (`i2c_sensor_c`) that initializes the I2C bus and reads the temperature from the SHTC3.
- **In C:** We will use the ESP-IDF driver APIs (`driver/i2c.h` or the newer `driver/i2c_master.h`) to configure the I2C peripheral.
- We will construct the specific byte sequence required by the SHTC3 datasheet (Wake up, Measure, Read) and send it over the I2C bus using the driver functions.

### Part 2: Direct Memory-Mapped Register Access (Raw C)
To truly understand how the I2C protocol is generated on the physical pins, we will write a new C project (`i2c_raw_c`) that bypasses the ESP-IDF drivers completely. 

> [!CAUTION]
> The I2C protocol requires precise timing, clock stretching, and state machine management. Controlling this via raw registers is significantly more complex than simple GPIO toggling.

- We will open the ESP32-C3 Technical Reference Manual (TRM) to the I2C Controller chapter.
- We will locate the I2C hardware peripheral base address.
- We will use raw C pointers to directly manipulate the 32-bit hardware registers:
    - Configure the SCL clock dividers and timing registers (`I2C_SCL_LOW_PERIOD_REG`, `I2C_SCL_HIGH_PERIOD_REG`).
    - Write our target sensor address and commands directly into the I2C hardware Command Registers (`I2C_COMD0_REG`).
    - Push data into the hardware FIFO buffer.
    - Manually trigger the hardware state machine to execute the transaction and read the resulting bytes from the RX FIFO.

### Part 3: Bare-Metal Rust (`esp-rs`)
Once we understand the hardware architecture and the C implementation, we will port our knowledge to pure bare-metal Rust (`i2c_sensor_rust`).
- We will initialize the I2C peripheral using `esp-hal`.
- We will utilize Rust's `embedded-hal` traits. We have two choices for reading the sensor:
    1. Manually construct the byte arrays and write/read using the `i2c.write_read()` method.
    2. Leverage the powerful Rust ecosystem by importing a pre-written driver crate specifically for the `shtcx` sensor, plugging our hardware I2C bus into it.

## Verification Plan

### Manual Verification
1. **ESP-IDF Validation (Part 1):** Flash the high-level C code. Verify that realistic room temperature and humidity values are printed to the serial monitor.
2. **Bare-Metal Register Validation (Part 2):** Flash the raw C code. Verify that we can successfully read the exact same temperature values by manually triggering the hardware state machine.
3. **Rust Validation (Part 3):** Flash the Rust code. Verify the serial monitor outputs the identical data, proving our bare-metal environment is successfully managing the hardware.
