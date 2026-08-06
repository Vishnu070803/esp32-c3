param (
    # Default path to the ESP-IDF installation directory.
    # If your ESP-IDF is installed elsewhere, change this variable or pass it as an argument:
    # Example: . .\activate_idf.ps1 -IdfPath "C:\custom\path\esp-idf"
    [string]$IdfPath = "C:\esp\v6.0.2\esp-idf",
    
    # Default path to the embedded Python installed by Espressif IDE.
    [string]$EspressifPython = "C:\Espressif\tools\python"
)

Write-Host "=== ESP-IDF Environment Activator ===" -ForegroundColor Cyan

# 1. Prevent Windows Store Python Hijack
# We inject the Espressif embedded Python to the front of the PATH for this session.
if (Test-Path $EspressifPython) {
    $env:PATH = "$EspressifPython;" + $env:PATH
    Write-Host "[OK] Injected Espressif Python into PATH to prevent Windows Store prompt." -ForegroundColor Green
} else {
    Write-Host "[WARN] Espressif Python not found at $EspressifPython. Relying on system Python." -ForegroundColor Yellow
}

# 2. Locate and Dot-Source the official export script
$ExportScript = Join-Path $IdfPath "export.ps1"

if (Test-Path $ExportScript) {
    Write-Host "[OK] Found export.ps1 at $ExportScript. Activating..." -ForegroundColor Green
    
    # Dot-sourcing the script so the environment variables bleed into this active terminal
    . $ExportScript
    
    Write-Host "=== ESP-IDF is ready! ===" -ForegroundColor Green
    Write-Host "Useful commands:"
    Write-Host "  idf.py build              - Compile the project"
    Write-Host "  idf.py -p COMx flash      - Flash to device"
    Write-Host "  idf.py fullclean          - Clean the build folder (Crucial when switching from Linux!)"
} else {
    Write-Host "[ERROR] Could not find export script at: $ExportScript" -ForegroundColor Red
    Write-Host "Please edit 'activate_idf.ps1' and update the `$IdfPath variable to match your system." -ForegroundColor Yellow
}
