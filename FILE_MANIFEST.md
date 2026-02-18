# File Manifest

Complete list of all files in the Patient Fall Detection System.

## Documentation (6 files)

| File | Purpose | Lines |
|------|---------|-------|
| README.md | Project overview, quick start, features | ~150 |
| USAGE.md | Detailed user guide with examples | ~200 |
| CONTRIBUTING.md | Developer guide, code standards | ~250 |
| ARCHITECTURE.md | Visual architecture diagrams | ~150 |
| IMPLEMENTATION_SUMMARY.md | Technical implementation details | ~250 |
| CHECKLIST.md | Development progress tracking | ~180 |

## Configuration (3 files)

| File | Purpose |
|------|---------|
| platformio.ini | Build configuration for simulation and production |
| diagram.json | Wokwi circuit diagram |
| wokwi.toml | Wokwi simulator configuration |

## Interfaces (3 files)

| File | Purpose | Methods |
|------|---------|---------|
| include/interfaces/IForceSensor.h | Force sensor abstraction | init(), getPressurePercentage(), isOccupied() |
| include/interfaces/INurseInput.h | Button input abstraction | init(), update(), wasShortPressed(), wasLongPressed() |
| include/interfaces/IAlertSystem.h | Alert system abstraction | init(), triggerFallAlarm(), clearAlarm() |

## Models (2 files)

| File | Purpose |
|------|---------|
| include/models/SystemState.h | State machine enum (6 states) |
| include/models/SensorPayload.h | Sensor data structure |

## Constants (2 files)

| File | Purpose |
|------|---------|
| include/constants/SystemConstants.h | Timing and thresholds |
| include/constants/NetworkConstants.h | WiFi and API configuration |

## Logic Layer (2 files)

| File | Purpose | Lines |
|------|---------|-------|
| src/logic/FallDetector.h | State machine interface | ~35 |
| src/logic/FallDetector.cpp | State machine implementation | ~190 |

## Simulation Drivers (3 files)

| File | Purpose | Simulates |
|------|---------|-----------|
| src/drivers/sim/WokwiPotentiometer.cpp | ADC-based force sensor | Tekscan A502 |
| src/drivers/sim/WokwiButton.cpp | GPIO button with debounce | Blue Charm BC B2 |
| src/drivers/sim/SerialConsoleAlert.cpp | Serial console logger | Connexxall alerts |

## Production Drivers (3 files - stubs)

| File | Purpose | Status |
|------|---------|--------|
| src/drivers/real/TekscanA502.cpp | Real force sensor driver | Stub with TODOs |
| src/drivers/real/BlueCharmBLE.cpp | Real BLE button driver | Stub with TODOs |
| src/drivers/real/ConnexxallWiFi.cpp | Real HTTP alert driver | Stub with TODOs |

## Main Application (1 file)

| File | Purpose | Lines |
|------|---------|-------|
| src/main.cpp | Dependency injection composition root | ~60 |

## Test Infrastructure (3 files)

| File | Purpose |
|------|---------|
| test/helpers/MockSensors.h | Mock implementations for unit testing |
| test/generators/SensorDataGenerator.h | Test data generation utilities |
| test/test_state_machine.cpp | Unit test template and examples |

## Build Artifacts (excluded via .gitignore)

- .pio/ - PlatformIO build output
- *.o, *.elf, *.bin - Compiled binaries
- .vscode/ - IDE configuration

## Total: 30 Files

- 21 Source/Header files
- 6 Documentation files  
- 3 Configuration files

## File Organization by Layer

```
Composition Root (1)
    └─ main.cpp

Logic Layer (2)
    ├─ FallDetector.h
    └─ FallDetector.cpp

Interface Layer (3)
    ├─ IForceSensor.h
    ├─ INurseInput.h
    └─ IAlertSystem.h

Driver Layer (6)
    ├─ Simulation (3)
    │   ├─ WokwiPotentiometer.cpp
    │   ├─ WokwiButton.cpp
    │   └─ SerialConsoleAlert.cpp
    └─ Production (3)
        ├─ TekscanA502.cpp
        ├─ BlueCharmBLE.cpp
        └─ ConnexxallWiFi.cpp

Models & Constants (4)
    ├─ SystemState.h
    ├─ SensorPayload.h
    ├─ SystemConstants.h
    └─ NetworkConstants.h

Test Infrastructure (3)
    ├─ MockSensors.h
    ├─ SensorDataGenerator.h
    └─ test_state_machine.cpp
```

## Key Metrics

- **Interfaces**: 3 (force, button, alert)
- **Implementations**: 6 (3 simulation + 3 production)
- **States**: 6 (SYSTEM_OFF, IDLE, POLLING, INPUT_PAUSED, CALIBRATION, ALARM)
- **State Transitions**: 8 (all from specification)
- **Documentation**: 6 comprehensive guides
- **Test Helpers**: 3 (mocks, generators, test template)

## Dependencies

### Simulation Environment
- No external libraries required
- Uses built-in Arduino framework

### Production Environment
- WiFi (built-in with ESP32)
- HTTPClient (built-in with ESP32)
- BLE library (NimBLE-Arduino - to be added)
- ArduinoJson (for API payloads - to be added)

## Notes

All production drivers contain TODO comments marking where real hardware implementation is needed. The simulation drivers are fully functional and ready for Wokwi testing.
