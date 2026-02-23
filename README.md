# CSE-3.5-Capstone Fall Detection System
## Overview

This system monitors a force sensor (Tekscan A502) to detect when a patient stands up from a bed or chair, and sends alerts via Connexxall. It also listens for BLE button presses from nurses to pause, calibrate, or acknowledge alarms.

## System States

| Current State |            Input/Trigger         |   New State  |    Logic/Output Action    |
|---------------|----------------------------------|--------------|---------------------------|
| SYSTEM_OFF    | System power on                  | IDLE         | bootUpSystem()            |
| IDLE          | isWifiConnected && isSensorReady | POLLING      | Log: "System Armed"       |
| POLLING       | Nurse presses hold button        | INPUT_PAUSED | pauseLogic() (for 2 mins) |
| INPUT_PAUSED  | Timer > PAUSE_DURATION_MS        | POLLING      | Resume monitoring         |
| INPUT_PAUSED  | Button pressed once (short)      | CALIBRATION  | Start zeroing sequence    |
| CALIBRATION   | timer > CALIB_DURATION_MS        | POLLING      | Save new zero offsets     |
| POLLING       | detectedForce > THRESHOLD        | ALARM        | Send alarm to Connexxall  |
| ALARM         | Nurse Press Button               | POLLING      | clearConnexxallAlert()    |

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
│   └── main.cpp                    // Composition root
├── test/                           // Unit tests (future)
├── diagram.json                    // Wokwi circuit diagram
└── wokwi.toml                      // Wokwi configuration
```

### Quick Start - Wokwi Simulation

Run the ``combine_to_sketch.py`` script with the flag to decide whether we want to build for production or simulation with dependency injection.

Example: 
```$ python combine_to_sketch.py``` then, copy the sketch.ino file and place it into the web browser for Wokwi, and copy the ```diagram.json``` file and place it into that browser project as well. You can then build the project and run it there to see all the logs and test behavior. 


## License

This is a capstone project for Ohio State University, CSE 5911