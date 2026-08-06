# Windows ESP-IDF Daily Workflow

When switching back and forth between Linux and Windows on this codebase, you need to be mindful of two things: **Environment Activation** and **Build Artifacts**.

This guide explains the daily routine for compiling and flashing code on Windows.

## 1. Environment Activation

Every time you open a new PowerShell terminal to work on this project, your terminal needs to know where the ESP-IDF toolchains (`riscv32-esp-elf-gcc`, `idf.py`, `cmake`) are located. 

Instead of doing this manually, use the team activation script included in the root of the repository.

### How to use it:
Open your PowerShell or VS Code terminal in the repository root and run:
```powershell
. .\activate_idf.ps1
```

> **Note the syntax:** You must include the leading dot and space (`. .\`). This is called **dot-sourcing**. It tells PowerShell to execute the script in the *current* context so the environment variables bleed into your active terminal.

### Customizing paths for other developers
If another developer clones this repo and their ESP-IDF is installed in a different location, they can simply edit the variables at the top of `activate_idf.ps1`:
```powershell
param (
    [string]$IdfPath = "C:\esp\v6.0.2\esp-idf",
    [string]$EspressifPython = "C:\Espressif\tools\python"
)
```
Or they can pass it dynamically without modifying the file:
```powershell
. .\activate_idf.ps1 -IdfPath "C:\Users\John\esp\esp-idf"
```

---

## 2. Dealing with Cross-Platform Build Artifacts

If you previously built a project (like `c_RMT_RGB_toggle`) on **Linux**, the generated `build/` folder contains Linux-specific paths (e.g., `/home/...`) and Ninja configurations.

If you try to run `idf.py build` on Windows with that Linux `build/` folder still present, CMake will crash with path syntax errors.

### The Rule of Thumb
Whenever you switch operating systems (or pull down changes from a team member on a different OS), **always clean the build directory**.

1. Navigate to your specific project folder:
   ```powershell
   cd Phase2_Physical_World_GPIO\c_RMT_RGB_toggle
   ```
2. Destroy the old OS build artifacts:
   ```powershell
   idf.py fullclean
   ```
3. Set the target architecture (just in case):
   ```powershell
   idf.py set-target esp32c3
   ```
4. Build the firmware for Windows:
   ```powershell
   idf.py build
   ```

---

## 3. Flashing and Monitoring

On Windows, your ESP32-C3 will mount to a COM port (e.g., `COM3`, `COM4`) instead of a Linux `ttyUSB` port. You can find this by opening **Device Manager -> Ports (COM & LPT)**.

To flash and see the serial monitor in one step:
```powershell
idf.py -p COM3 flash monitor
```
*(Press `Ctrl + ]` to exit the monitor).*
