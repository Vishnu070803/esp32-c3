# Phase 4: Advanced Motion Tracking (IMU & Interrupts)

In Phase 3, you likely "polled" the temperature sensor—meaning your `loop {}` constantly asked the sensor "what is the temperature now?" over and over. In advanced systems, this wastes CPU power. 
Let's learn about **Interrupts** and complex sensors.

## 1. The ICM-42670-P IMU (Inertial Measurement Unit)
Your board has an IMU, which is actually two sensors in one package:
*   **Accelerometer (3-Axis):** Measures linear acceleration (g-force). It can tell which way gravity is pulling, letting you determine the tilt/orientation of the board.
*   **Gyroscope (3-Axis):** Measures rotational speed (degrees per second). It tells you how fast the board is spinning around the X, Y, and Z axes.

These sensors use **MEMS (Micro-Electromechanical Systems)** technology. There are literal microscopic silicon beams etched into the chip that bend when you move the board. That bending changes electrical capacitance, which is converted into digital I2C/SPI data!

## 2. Hardware Interrupts
Instead of your code constantly checking if the board has moved, you can configure the IMU to monitor motion on its own. 
When it detects a shake or tilt, it sends a physical voltage spike over a dedicated wire to one of the ESP32-C3's GPIO pins.

*   **The Interrupt Controller:** The ESP32-C3 has an internal block that monitors GPIO pins. When it sees that voltage spike, it instantly pauses your main `loop {}` program, saves the CPU state, and jumps to a special function you wrote called an **Interrupt Service Routine (ISR)**.
*   Inside the ISR, you handle the event (e.g., set a flag that motion occurred), and then the CPU returns right back to where it was in the main loop.

### Action Items for You:
1.  Initialize the IMU sensor in a new project.
2.  Read the X, Y, Z acceleration data in real-time and print it.
3.  *Advanced Challenge:* Try configuring a GPIO pin as an input with an **Interrupt**, and see if you can make the IMU trigger an interrupt when the board is tapped!
