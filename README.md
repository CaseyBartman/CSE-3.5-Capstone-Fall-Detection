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


### Development Standards
## Engineering Standards & Testing Manifesto
To ensure our codebase remains scalable and our tests act as reliable documentation, we adhere to the following architectural principles:

## Logic & Abstraction
The Stepdown Rule: Code must read as a top-down narrative. High-level logic stays at the top, descending one level of abstraction at a time.

Encapsulation Over Exposure: Prioritize hiding storage or internal structural details. The top layer should never interact with the "how," only the "what."

Helper Logic: If any conditional (if) path exceeds 2 lines, it must be extracted into a descriptive helper function.

Function Size: Keep functions concise. Target 15 lines, with a hard limit of 20 lines.

## Naming & Documentation
The "Why/What/How" Rule: Variable and function names must clearly communicate their purpose and intent.

Domain Context: Variable naming should reflect domain context (e.g., xaLienPayload) rather than generic terms.

Boolean Prefixing: All flags must use is, has, or should prefixes (e.g., isModalOpen, hasRequirement).

## Prohibited Practices
No Magic Values: No literals in assertions or actions. Use named constants to explain the "why" behind a value.

DRY Strings: Any string used in 2 or more places must be moved to a central constant file.

Comments: Avoid these wherever possible. If you require a comment to explain the code written, the code is not readable and should be clearly refactored to follow our naming standards.

## Running Tests
Dependencies: 
- CMake (For your dev environment, install cmake via web download, brew, winget, etc.)
- C++ Compiler of your choosing (initially configured for GNU Make 3.81 on Windows environment. Use this link if windows: winget install GnuWin32.Make and make sure to set your PATH variable).
- Python 
run ```$ bash run_tests.sh --verbose --clean``` and view the results of the tests.

This runs a script that builds all the mock and source code, runs the tests, and prints the resujlts
-See test_state_machine.cpp for the tests themselves. The script references CMakeLists.txt to build the project with the items in the /test folder, runs those tests, and prints the results using parse_tests.py
The tests are done using the Google Test C++ Framework

## License
This is a capstone project for Ohio State University, CSE 5911