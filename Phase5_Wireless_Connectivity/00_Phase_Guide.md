# Phase 5: Wireless Connectivity (Wi-Fi)

The ESP32-C3's superpower is its integrated 2.4 GHz radio. Up until now, we've only been talking to the internal processor and local sensors. Now we connect to the world.

## 1. The Radio Subsystem Architecture
The Wi-Fi block inside the ESP32-C3 is incredibly complex. It includes:
*   **RF Transceiver:** The analog circuitry that actually generates and receives 2.4GHz radio waves via the antenna.
*   **Baseband/MAC:** The digital hardware that formats data into 802.11 b/g/n Wi-Fi frames and handles the strict timing required to exist on a wireless network without colliding with other devices.

Because this hardware requires massive, complex, and proprietary timing code to operate, Espressif provides pre-compiled "blobs" (binary libraries) that interface with the radio.

## 2. Networking Stacks
Having a Wi-Fi connection just means you can send radio waves to your router. To load a webpage, you need a Network Stack.
*   **TCP/IP Stack (e.g., lwIP or smoltcp):** This software layer sits on top of the Wi-Fi driver. It takes your raw data, wraps it in TCP segments, wraps those in IP packets, and hands them to the Wi-Fi hardware.

## 3. `esp-wifi` vs `esp-idf`
In Rust, you have two paths for wireless:
1.  **Bare-metal (`esp-wifi` + `smoltcp`):** You use the `esp-wifi` crate which wraps the proprietary radio blobs, and `smoltcp` as a pure-Rust network stack. This gives you maximum control and zero OS overhead.
2.  **Standard Library (`esp-idf-svc`):** You compile your Rust code against Espressif's official C framework (ESP-IDF) which includes FreeRTOS and lwIP. This gives you the standard Rust `std::net::TcpStream` API, making it feel just like writing a network app on Linux.

### Action Items for You:
1.  Decide whether you want to try bare-metal Wi-Fi or standard-library Wi-Fi. (Standard library is highly recommended for networking ease!).
2.  Connect to your home Wi-Fi network.
3.  Ping an external IP address, or write a tiny HTTP server that serves the temperature from Phase 3 to a web browser on your phone!
