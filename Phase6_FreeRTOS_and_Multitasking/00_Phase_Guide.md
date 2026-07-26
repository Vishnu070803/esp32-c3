# Phase 6: Real-Time Operating Systems (FreeRTOS)

Until now, we have talked about "bare-metal" programming. This means your `main()` function contains a giant `while(true)` loop that does everything: reads the sensor, updates the LED, checks the Wi-Fi, and repeats. This is called a "Super Loop."

If the Wi-Fi connection hangs for 3 seconds, your LED freezes for 3 seconds. To fix this, we need an OS.

## 1. What is FreeRTOS?
FreeRTOS (Free Real-Time Operating System) is the industry standard for microcontrollers. It is built directly into the ESP-IDF framework. It doesn't look like Linux—there is no GUI, no filesystem by default, and no shell. It is purely a **Task Scheduler**.

## 2. Tasks and The Scheduler
Instead of one giant loop, you write multiple small loops (Tasks).
*   **Task 1:** Blink the LED.
*   **Task 2:** Read the I2C IMU sensor.
*   **Task 3:** Send data over Wi-Fi.

The **Scheduler** is a piece of code that runs in the background. It uses a hardware timer to interrupt the CPU 1000 times a second (1000 Hz Tick). 
Every tick, the Scheduler looks at all your tasks, checks their priorities, and decides which one gets to run on the ESP32-C3's single core for the next millisecond. This gives the *illusion* of parallel processing.

## 3. Real-Time vs. General Purpose
Why use FreeRTOS instead of Linux? 
*   Linux is a "General Purpose OS". If you ask Linux to toggle a GPIO pin every exactly 10 microseconds, it might say "Hold on, I'm updating a background service right now." It is not deterministic.
*   FreeRTOS is "Real-Time". If you assign a high priority to a task, the scheduler guarantees it will interrupt anything else to run that task at the exact microsecond it is needed.

## 4. Inter-Process Communication (IPC)
If Task 1 (Sensor) needs to send data to Task 3 (Wi-Fi), they can't just share a global variable (that causes race conditions when the scheduler interrupts them mid-write).
FreeRTOS provides:
*   **Queues:** A safe pipeline to send data between tasks.
*   **Mutexes:** A "key" that tasks must hold to access shared hardware (like the I2C bus) so two tasks don't talk to different sensors at the exact same millisecond.

### Action Items for You:
1.  *(This action item depends on your choice in Phase 0.5).*
2.  If using ESP-IDF (C/C++) or Rust (`std`), we will write a multi-tasking program where one task blinks the LED at exactly 500ms, while another task purposely blocks the CPU doing heavy math, proving that the FreeRTOS scheduler keeps the LED blinking on time regardless of the heavy load!
