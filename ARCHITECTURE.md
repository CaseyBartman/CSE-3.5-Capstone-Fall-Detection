# Architecture Diagram

```
┌──────────────────────────────────────────────────────────────────────┐
│                      PATIENT FALL DETECTION SYSTEM                    │
│                    (Dependency Injection Architecture)                │
└──────────────────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────────────────┐
│                          COMPOSITION LAYER                            │
├──────────────────────────────────────────────────────────────────────┤
│  main.cpp                                                             │
│  - Conditional compilation (IS_SIMULATION vs IS_REAL_HARDWARE)       │
│  - Creates concrete implementations                                   │
│  - Injects dependencies into FallDetector                             │
│  - Manages main loop                                                  │
└────────────────────────┬─────────────────────────────────────────────┘
                         │
                         │ injects
                         ▼
┌──────────────────────────────────────────────────────────────────────┐
│                          LOGIC LAYER (BRAIN)                          │
├──────────────────────────────────────────────────────────────────────┤
│  FallDetector (State Machine)                                         │
│  ├─ States: SYSTEM_OFF, IDLE, POLLING, INPUT_PAUSED, CALIBRATION,   │
│  │          ALARM                                                     │
│  ├─ Transitions: All 8 transitions from spec implemented             │
│  ├─ Methods:                                                          │
│  │   - handleIdleState()                                             │
│  │   - handlePollingState()      ← Fall detection logic             │
│  │   - handleAlarmState()        ← Alarm clearing                   │
│  │   - handlePauseState()        ← 2-min pause management           │
│  │   - handleCalibrationState()  ← 5-sec calibration                │
│  └─ Helper: transitionToState() - Entry actions & logging            │
└────────────────────────┬─────────────────────────────────────────────┘
                         │
                         │ depends on
                         ▼
┌──────────────────────────────────────────────────────────────────────┐
│                        INTERFACE LAYER (CONTRACTS)                    │
├──────────────────────────────────────────────────────────────────────┤
│  IForceSensor                 INurseInput               IAlertSystem  │
│  - init()                     - init()                  - init()      │
│  - getPressurePercentage()    - update()                - trigger()   │
│  - isOccupied()               - wasShortPressed()       - clear()     │
│                               - wasLongPressed()                      │
└───────────┬──────────────────────────┬─────────────────────┬─────────┘
            │                          │                     │
            │ implemented by           │                     │
            ▼                          ▼                     ▼
┌──────────────────────────────────────────────────────────────────────┐
│                      DRIVER LAYER (HARDWARE)                          │
├─────────────────────────┬────────────────────────────────────────────┤
│  SIMULATION (Wokwi)     │  PRODUCTION (Real Hardware)                │
├─────────────────────────┼────────────────────────────────────────────┤
│  WokwiPotentiometer     │  TekscanA502                               │
│  - Simulates force      │  - Real ADC reading                        │
│    sensor with ADC      │  - TODO: Implement protocol                │
│                         │                                            │
│  WokwiButton            │  BlueCharmBLE                              │
│  - GPIO with debounce   │  - BLE connection                          │
│  - Short/long press     │  - TODO: Implement BLE                     │
│                         │                                            │
│  SerialConsoleAlert     │  ConnexxallWiFi                            │
│  - Serial print logs    │  - HTTP POST/DELETE                        │
│                         │  - TODO: Implement API                     │
└─────────────────────────┴────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────────────────┐
│                      MODELS & CONSTANTS                               │
├──────────────────────────────────────────────────────────────────────┤
│  SystemState.h          SensorPayload.h                               │
│  - State enum           - Data structure                              │
│                                                                       │
│  SystemConstants.h      NetworkConstants.h                            │
│  - Timing (2-min, 5s)   - WiFi, API keys                             │
│  - Thresholds (15%, 5%) - Endpoints                                   │
└──────────────────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────────────────┐
│                      TEST INFRASTRUCTURE                              │
├──────────────────────────────────────────────────────────────────────┤
│  MockSensors.h                SensorDataGenerator.h                   │
│  - MockForceSensor            - Fall scenarios                        │
│  - MockNurseInput             - Stable patterns                       │
│  - MockAlertSystem            - Test sequences                        │
│                                                                       │
│  test_state_machine.cpp                                               │
│  - Template with examples                                             │
│  - Unity framework integration                                        │
└──────────────────────────────────────────────────────────────────────┘

STATE TRANSITIONS:
══════════════════

SYSTEM_OFF ──[power on]──────────────────────────────────► IDLE
                                                             │
                                                [ready]      │
                                                             ▼
                                                          POLLING ◄─────┐
                                                             │          │
                        ┌──────────[long press]────────────┘          │
                        ▼                                              │
                  INPUT_PAUSED                                         │
                    │       │                                          │
          [short]   │       │  [2-min timeout]                        │
                    ▼       └─────────────────────────────────────────┘
                CALIBRATION                                            │
                    │                                                  │
                    │  [5-sec timeout]                                │
                    └──────────────────────────────────────────────────┘
                                                                       │
POLLING ──[fall detected]───► ALARM ──[button press]──────────────────┘

KEY FEATURES:
═════════════

✓ Dependency Injection        ✓ Simulation-First Development
✓ Clean Architecture          ✓ State Machine Pattern
✓ SOLID Principles            ✓ Interface Segregation
✓ Testability                 ✓ Hardware Abstraction
✓ Wokwi Ready                 ✓ Production Ready (stubs)
✓ Comprehensive Docs          ✓ Mock Framework
