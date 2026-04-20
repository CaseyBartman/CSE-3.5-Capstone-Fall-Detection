# CSE-3.5-Capstone Fall Detection System

## Table of Contents
- [Overview](#overview)
- [Handoff Documentation Guide](#handoff-documentation-guide)
- [System States & Workflows](#system-states--workflows)
- [Architecture](#architecture)
- [Technical Setup Guide](#technical-setup-guide)
- [Tips for Future Development](#tips-for-future-development)
- [Engineering Standards & Testing Manifesto](#engineering-standards--testing-manifesto)
- [License](#license)

## Overview
This system is a smart pressure-sensing floor mat that continuously monitors patient movement to detect when a patient uses a toilet and stands up without supervision, and sends alerts via ntfy. It also listens for BLE button presses from nurses to pause, calibrate, or acknowledge alarms. This system is designed for engineers, clinical stakeholders, and future development teams.

## Handoff Documentation Guide
This repository contains several documents to support a smooth transition to future teams. Here’s how to use them:

-   **This `README.md` file** is the primary technical guide for engineers. It covers the system architecture, development setup, testing procedures, and coding standards. If you need to understand *how* the system works or *how* to modify it, start here.
-   **The `USER_MANUAL.md`** is a non-technical guide for clinical staff (nurses). It explains the system's purpose and day-to-day operation in simple terms, focusing on user workflows and troubleshooting. It can be edited here: https://docs.google.com/document/d/1zu8R34xOgtsTTHlpuhSVWi8CvuyDlSidG8npKi_QmLA/edit?tab=t.0
-   **The Definition of Done (DoD)** is the product requirements document. It defines the exact system behaviors, user stories, and edge cases. It should be used by engineers to verify the implementation and by stakeholders to understand the system's precise capabilities and limitations. This file is not saved in this repository, but was provided with the other handoff documentation. 

## System States & Workflows
The system operates based on a state machine that manages its behavior.

| Current State | Input/Trigger | New State | Logic/Output Action |
|---------------|----------------------------------|--------------|---------------------------|
| `SYSTEM_OFF` | System power on | `IDLE` | `bootUpSystem()` |
| `IDLE` | `isWifiConnected && isSensorReady` | `POLLING` | Log: "System Armed" |
| `POLLING` | Nurse presses button (short) | `PAUSED` | `pauseLogic()` (for 2 mins) |
| `POLLING` | `detectedForce > THRESHOLD` | `ALERT` | Send alarm via ntfy |
| `PAUSED` | Timer > `PAUSE_DURATION_MS` | `POLLING` | Resume monitoring |
| `PAUSED` | Nurse presses button (short) | `CALIBRATING` | Start zeroing sequence |
| `CALIBRATING` | Timer > `CALIB_DURATION_MS` | `POLLING` | Save new zero offsets |
| `ALERT` | Nurse presses button (short) | `POLLING` | `clearNtfyAlert()` |

## Architecture
This project uses **Dependency Injection** for simulation-first development. The core logic is completely independent of hardware, allowing for robust testing and development flexibility.

### Directory Structure
- **`sketch.ino`**: Standalone Arduino sketch for Wokwi simulation.
- **`include/`**: Contains interfaces (`IForceSensor`, `INurseInput`, `IAlertSystem`), data models (`SystemState`, `SensorPayload`), and constants.
- **`src/`**:
    - **`logic/`**: The core state machine (`FallDetector.cpp`)—the "brains" of the system.
    - **`drivers/`**: Real and simulated hardware drivers. Dependency injection allows swapping between `real/` and `sim/` implementations.
    - **`main.cpp`**: The composition root where dependencies are wired together.
- **`test/`**: Unit and integration tests using the Google Test framework.
- **`run_tests.sh`**: A script that builds and runs all tests via CMake.

## Technical Setup Guide

### 1. Wokwi Simulation Setup
Wokwi allows you to run the entire system in a browser without any physical hardware.

1.  **Generate the Sketch**: Run the Python script to combine all necessary source files into a single Arduino sketch.
    ```bash
    python combine_to_sketch.py
    ```
2.  **Open Wokwi**: Go to [Wokwi.com](https://wokwi.com/) and create a new Arduino project.
3.  **Load the Sketch**: Copy the entire content of the generated `sketch.ino` file and paste it into the `sketch.ino` tab in your Wokwi project.
4.  **Load the Diagram**: Copy the entire content of `diagram.json` from this repository and paste it into the `diagram.json` tab in Wokwi. This file defines the simulated hardware components (like the potentiometer for the force sensor and the button for nurse input).
5.  **Run Simulation**: Click the "Start Simulation" button. You can now interact with the simulated hardware and see log output in the Serial Monitor.

### 2. Running the Test Suite
The project uses the Google Test framework for C++ unit and integration testing.

**Dependencies:**
-   **CMake**: For building the test suite.
-   **C++ Compiler**: Configured for GNU Make 3.81 on Windows.
-   **Python**: For the test parsing script.

**Execution:**
Run the following command in a bash terminal to build the tests, execute them, and parse the results:
```bash
bash run_tests.sh --verbose --clean
```
This script automates the entire test process and provides a clean summary of the results.

### 3. Hardware Deployment

> **Setup order matters:** Deploy the ESP32 button controller **before** the Arduino. The Arduino begins listening for button signals as soon as it boots, so the ESP32 should already be connected to WiFi and ready to send signals when the Arduino starts up.

#### 3a. Install PlatformIO

PlatformIO is required to build and upload code to both the ESP32 and the Arduino.

1.  **Install PlatformIO IDE extension on VS Code**: Open VS Code → Extensions panel → search `PlatformIO IDE` → Install.

2.  **Open the project**: In VS Code, go to **File → Open Folder** and select the folder containing `platformio.ini`. PlatformIO should detect the config automatically.

    > **Note (Linux only):** To allow serial port access, add yourself to the `dialout` group, then log out and back in:
    > ```bash
    > sudo usermod -aG dialout $USER
    > ```

#### 3b. Configure Network Constants

Before flashing either device, update `include/constants/NetworkConstants.h` with your environment's values:

```cpp
const char* WIFI_SSID     = "your-network-name";
const char* WIFI_PASSWORD = "your-network-password";
const char* ARDUINO_HOST  = "192.168.x.x";           // Arduino's local IP address
const char* BUTTON_SIGNAL_ENDPOINT = "http://192.168.x.x/trigger";  // Full trigger URL
```

To find the Arduino's IP address, upload the `real` environment once and read it from the serial monitor output on boot.

> **Tip:** If the IP address shows as `0.0.0.0` or does not appear, press the Arduino's reset button and wait for it to reconnect. Repeat until a valid IP address is printed.

#### 3c. ESP32 Button Controller

1.  **Connect the ESP32 via USB**.

2.  **Build and upload**:
    ```bash
    pio run -e esp32 --target clean
    pio run -e esp32 --target upload
    ```

3.  **Monitor serial output** to confirm WiFi connection and signal transmission:
    ```bash
    pio device monitor -e esp32 --baud 115200
    ```

#### 3d. Arduino UNO R4 WiFi

1.  **Connect the UNO R4 WiFi via USB**: Use the board's USB-C port.

2.  **Select your build environment**: The project has two environments — pick the one matching your hardware setup:
    - `real` — Tekscan A502 + physical button + Ntfy alerts
    - `archive` — Tekscan A502 + BlueCharm BLE + LED *(no longer supported)*

3.  **Build and upload**:

    Build and upload (`real` environment):
    ```bash
    pio run -e real --target clean
    pio run -e real --target upload
    ```

    Build and upload (`archive` environment):
    ```bash
    pio run -e archive --target clean
    pio run -e archive --target upload
    ```

4.  **Monitor serial output**:
    ```bash
    pio device monitor -e real --baud 115200
    ```

## Tips for Future Development
- **Start with a Test**: Before adding or changing any logic in `FallDetector.cpp`, create a new test case in `test/test_state_machine.cpp` that reproduces the desired behavior or bug. This follows our Test-Driven Design (TDD) philosophy.
- **Add New Hardware**: To support a new type of sensor or alert system:
    1.  Define a new interface in `include/interfaces/`.
    2.  Create a simulated version in `src/drivers/sim/`.
    3.  Create the real hardware driver in `src/drivers/real/`.
    4.  Update the composition root (`main.cpp`) and the `combine_to_sketch.py` script to include the new driver.
- **Stay Abstract**: Always keep the core logic in `FallDetector.cpp` free of any hardware-specific code. It should only interact with interfaces, never concrete drivers.

## Engineering Standards & Testing Manifesto
To ensure our codebase remains scalable and our tests act as reliable documentation, we adhere to the following architectural principles:

### Logic & Abstraction
- **The Stepdown Rule**: Code must read as a top-down narrative. High-level logic stays at the top, descending one level of abstraction at a time.
- **Encapsulation Over Exposure**: Prioritize hiding storage or internal structural details. The top layer should never interact with the "how," only the "what."

### Naming & Documentation
- **The "Why/What/How" Rule**: Variable and function names must clearly communicate their purpose and intent.
- **Boolean Prefixing**: All flags must use `is`, `has`, or `should` prefixes (e.g., `isModalOpen`).

### Prohibited Practices
- **No Magic Values**: No literals in assertions or actions. Use named constants to explain the "why" behind a value.
- **DRY Strings**: Any string used in 2 or more places must be moved to a central constant file.
- **Comments**: Avoid these wherever possible. If you require a comment to explain the code, the code is not readable and should be refactored.

## License
This is a capstone project for The Ohio State University, CSE 5911.