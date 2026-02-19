# CSE-3.5-Capstone Fall Detection System

A patient fall detection system using dependency injection for simulation-first development on Arduino/ESP32.

## Overview

This system monitors a force sensor (Tekscan A502) to detect when a patient stands up from a bed or chair, and sends alerts via Connexxall. It also listens for BLE button presses from nurses to pause, calibrate, or acknowledge alarms.

## System States

| Current State | Input/Trigger | New State | Logic/Output Action |
|--------------|---------------|-----------|-------------------|
| SYSTEM_OFF | System power on | IDLE | bootUpSystem() |
| IDLE | isWifiConnected && isSensorReady | POLLING | Log: "System Armed" |
| POLLING | Nurse presses hold button | INPUT_PAUSED | pauseLogic() (for 2 mins) |
| INPUT_PAUSED | Timer > PAUSE_DURATION_MS | POLLING | Resume monitoring |
| INPUT_PAUSED | Button pressed once (short) | CALIBRATION | Start zeroing sequence |
| CALIBRATION | timer > CALIB_DURATION_MS | POLLING | Save new zero offsets |
| POLLING | detectedForce > THRESHOLD | ALARM | Send alarm to Connexxall |
| ALARM | Nurse Press Button | POLLING | clearConnexxallAlert() |

## Architecture

This project uses **Dependency Injection** for simulation-first development. The core logic is completely independent of hardware, allowing you to:
- Develop and test in Wokwi with simulated components
- Deploy to real hardware without changing business logic

### Directory Structure

```
patient-fall-alert-system/
├── sketch.ino                      // Standalone Arduino sketch for Wokwi (recommended)
├── include/
│   ├── constants/
│   │   ├── SystemConstants.h       // Timers, thresholds
│   │   └── NetworkConstants.h      // API endpoints, WiFi credentials
│   ├── interfaces/
│   │   ├── IForceSensor.h          // Abstract interface for sensors
│   │   ├── INurseInput.h           // Abstract interface for button input
│   │   └── IAlertSystem.h          // Abstract interface for alerts
│   └── models/
│       ├── SystemState.h           // Enum: IDLE, POLLING, ALARM...
│       └── SensorPayload.h         // Data transfer object
├── src/
│   ├── logic/
│   │   ├── FallDetector.cpp        // State machine (The Brain)
│   │   └── FallDetector.h
│   ├── drivers/
│   │   ├── real/
│   │   │   ├── TekscanA502.cpp     // Real force sensor
│   │   │   ├── BlueCharmBLE.cpp    // Real BLE button
│   │   │   └── ConnexxallWiFi.cpp  // Real HTTP alerts
│   │   └── sim/
│   │       ├── WokwiPotentiometer.cpp  // Simulates force sensor
│   │       ├── WokwiButton.cpp         // Simulates nurse button
│   │       └── SerialConsoleAlert.cpp  // Simulates alerts
│   └── main.cpp                    // Composition root (PlatformIO)
├── test/                           // Unit tests (future)
├── platformio.ini                  // Build configurations (optional)
├── diagram.json                    // Wokwi circuit diagram
└── wokwi.toml                      // Wokwi configuration
```

**Note:** For Wokwi simulation, use `sketch.ino` (self-contained). For production builds with advanced features, use the PlatformIO structure.

## Getting Started

### Prerequisites

- For simulation: [Wokwi for VS Code](https://wokwi.com/vscode) extension
- For production: [PlatformIO](https://platformio.org/) (optional, for advanced builds)

### Quick Start - Wokwi Simulation (Recommended)

The easiest way to test the system is using the standalone Arduino sketch:

1. **Open in VS Code with Wokwi extension installed**
2. **Press F1** and select "Wokwi: Start Simulator"
3. The simulator will load `sketch.ino` and run automatically
4. **View Serial Monitor** to see logs (click the terminal icon in Wokwi)

**No build required!** Wokwi compiles the sketch automatically.

### Alternative: Building with PlatformIO (Optional)

If you prefer PlatformIO for advanced features:

```bash
# Build for Wokwi simulation
pio run -e wokwi-sim

# Upload and monitor (in Wokwi)
pio run -e wokwi-sim -t upload
pio device monitor
```

### Building for Production Hardware

```bash
# Build for ESP32 hardware
pio run -e production-esp32

# Upload to device
pio run -e production-esp32 -t upload

# Monitor serial output
pio device monitor
```

## Wokwi Simulation

The system includes a standalone Arduino sketch (`sketch.ino`) that works directly with Wokwi without requiring PlatformIO builds.

The Wokwi simulation uses:
- **Slide Potentiometer** (Pin 34) - Simulates the force sensor
- **Push Button** (Pin 15) - Simulates the nurse button
  - Short press: Acknowledge alarm or start calibration
  - Long press (2s): Pause monitoring

### Testing in Wokwi

1. Open the project in VS Code with Wokwi extension
2. Press F1 → "Wokwi: Start Simulator"
3. **Open Serial Monitor** in Wokwi to see logs
4. Interact with the components:
   - Move the potentiometer slider up (high pressure)
   - Move it down quickly (simulates fall)
   - Press the button briefly (short press)
   - Hold the button for 2 seconds (long press)

## Hardware Pins

| Component | Pin | Description |
|-----------|-----|-------------|
| Force Sensor | 34 | ADC1_CH6 (0-3.3V analog input) |
| Nurse Button | 15 | Digital input with internal pullup |

## Configuration

Edit constants in:
- `include/constants/SystemConstants.h` - Timing and thresholds
- `include/constants/NetworkConstants.h` - WiFi and API credentials

## Key Features

✅ **Dependency Injection** - Logic decoupled from hardware  
✅ **Simulation First** - Develop in Wokwi before hardware testing  
✅ **State Machine** - Clear state transitions with logging  
✅ **Clean Architecture** - Interfaces, models, and business logic separated  
✅ **Scalable** - Easy to add new sensors or alert systems  

## Future Enhancements

- [ ] Add unit tests using native testing framework
- [ ] Implement real Tekscan A502 driver
- [ ] Implement real BLE connection to Blue Charm button
- [ ] Implement Connexxall API integration
- [ ] Add data logging and analytics
- [ ] Implement WiFi connection management with reconnection logic

## License

This is a capstone project for CSE 3.5.