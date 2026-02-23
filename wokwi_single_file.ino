// Patient Fall Detection System - Wokwi Single File Sketch
// This file merges all logic, drivers, models, and interfaces for Wokwi simulation
// Place this file in the Wokwi web editor for simulation

// ============================================================================
// SYSTEM CONSTANTS
// ============================================================================
// Timing Constants (in milliseconds)
const unsigned long PAUSE_DURATION_MS = 120000;  // 2 minutes pause duration
const unsigned long CALIB_DURATION_MS = 5000;    // 5 seconds calibration duration
const unsigned long DEBOUNCE_DELAY_MS = 50;      // Button debounce delay
const unsigned long LONG_PRESS_MS = 2000;        // Long press threshold

// Sensor Thresholds
const float DEFAULT_PRESSURE_THRESHOLD = 15.0;   // Default 15% threshold for occupied state
const float FALL_DETECTION_THRESHOLD = 5.0;      // Threshold for detecting stand-up (fall below this %)

// System Constants
const int SENSOR_SAMPLE_RATE_MS = 50;            // Sensor sampling rate
const int SERIAL_BAUD_RATE = 115200;             // Serial communication baud rate

// ============================================================================
// STATE MACHINE
// ============================================================================

enum class SystemState {
  SYSTEM_OFF,
  IDLE,
  POLLING,
  INPUT_PAUSED,
  CALIBRATION,
  ALARM
};

// ============================================================================
// MODELS
// ============================================================================

struct SensorPayload {
  float pressurePercentage;
  bool isOccupied;
  unsigned long timestamp;

  SensorPayload() : pressurePercentage(0.0f), isOccupied(false), timestamp(0) {}
  SensorPayload(float pressure, bool occupied, unsigned long time)
    : pressurePercentage(pressure), isOccupied(occupied), timestamp(time) {}
};

// ============================================================================
// INTERFACES (Abstraction Layer)
// ============================================================================

class IForceSensor {
  public:
    virtual void init() = 0;
    virtual float getPressurePercentage() = 0;
    virtual bool isOccupied() = 0;
    virtual ~IForceSensor() {}
};

class INurseInput {
  public:
    virtual void init() = 0;
    virtual bool isPressed() = 0;
    virtual bool wasShortPressed() = 0;
    virtual bool wasLongPressed() = 0;
    virtual void update() = 0;
    virtual ~INurseInput() {}
};

class IAlertSystem {
  public:
    virtual void init() = 0;
    virtual void triggerFallAlarm() = 0;
    virtual void clearAlarm() = 0;
    virtual ~IAlertSystem() {}
};

// ============================================================================
// SIMULATION DRIVERS (Wokwi)
// ============================================================================

#include <Arduino.h>

class WokwiPotentiometer : public IForceSensor {
  private:
    const int _pin;
    const float _threshold;
  public:
    WokwiPotentiometer(int pin, float threshold) : _pin(pin), _threshold(threshold) {}
    void init() override {
      pinMode(_pin, INPUT);
      Serial.print("WokwiPotentiometer initialized on pin ");
      Serial.println(_pin);
    }
    float getPressurePercentage() override {
      int raw = analogRead(_pin);
      // Map 12-bit ADC (0-4095) to 0-100%
      return (float)raw / 40.95f;
    }
    bool isOccupied() override {
      return getPressurePercentage() > _threshold;
    }
};

class WokwiButton : public INurseInput {
  private:
    const int _pin;
    bool _lastState;
    bool _currentState;
    unsigned long _pressStartTime;
    bool _shortPressFlag;
    bool _longPressFlag;
    bool _longPressTriggered;
  public:
    WokwiButton(int pin)
      : _pin(pin), _lastState(HIGH), _currentState(HIGH),
        _pressStartTime(0), _shortPressFlag(false),
        _longPressFlag(false), _longPressTriggered(false) {}
    void init() override {
      pinMode(_pin, INPUT_PULLUP);
      Serial.print("WokwiButton initialized on pin ");
      Serial.println(_pin);
    }
    void update() override {
      _shortPressFlag = false;
      _longPressFlag = false;
      _currentState = digitalRead(_pin);
      if (_currentState == LOW && _lastState == HIGH) {
        _pressStartTime = millis();
        _longPressTriggered = false;
      }
      if (_currentState == LOW && _lastState == LOW) {
        unsigned long pressDuration = millis() - _pressStartTime;
        if (pressDuration >= LONG_PRESS_MS && !_longPressTriggered) {
          _longPressFlag = true;
          _longPressTriggered = true;
          Serial.println("Long press detected");
        }
      }
      if (_currentState == HIGH && _lastState == LOW) {
        unsigned long pressDuration = millis() - _pressStartTime;
        if (pressDuration < LONG_PRESS_MS) {
          _shortPressFlag = true;
          Serial.println("Short press detected");
        }
      }
      _lastState = _currentState;
    }
    bool isPressed() override {
      return _currentState == LOW;
    }
    bool wasShortPressed() override {
      return _shortPressFlag;
    }
    bool wasLongPressed() override {
      return _longPressFlag;
    }
};

class SerialConsoleAlert : public IAlertSystem {
  private:
    bool _alarmActive;
  public:
    SerialConsoleAlert() : _alarmActive(false) {}
    void init() override {
      Serial.println("SerialConsoleAlert initialized");
    }
    void triggerFallAlarm() override {
      _alarmActive = true;
      Serial.println("========================================");
      Serial.println("!!!!! FALL ALARM TRIGGERED !!!!!");
      Serial.println("Alert sent to Connexxall (Simulated)");
      Serial.println("========================================");
    }
    void clearAlarm() override {
      if (_alarmActive) {
        _alarmActive = false;
        Serial.println("========================================");
        Serial.println("Alarm cleared by nurse");
        Serial.println("Connexxall alert cleared (Simulated)");
        Serial.println("========================================");
      }
    }
};

// ============================================================================
// FALL DETECTOR LOGIC
// ============================================================================

class FallDetector {
  private:
    IForceSensor* _sensor;
    INurseInput* _button;
    IAlertSystem* _alert;
    SystemState _currentState;
    unsigned long _stateStartTime;
    bool _wasOccupied;
    float _calibrationThreshold;
    void handleIdleState() {
      bool isReady = true;
      if (isReady) {
        transitionToState(SystemState::POLLING);
      }
    }
    void handlePollingState() {
      float pressure = _sensor->getPressurePercentage();
      bool occupied = _sensor->isOccupied();
      if (!_wasOccupied && occupied) {
        _wasOccupied = true;
      }
      if (_wasOccupied && !occupied && pressure < FALL_DETECTION_THRESHOLD) {
        Serial.println("Fall detected! Occupant left bed.");
        _alert->triggerFallAlarm();
        transitionToState(SystemState::ALARM);
        return;
      }
      if (_button->wasShortPressed()) {
        Serial.println("Pause requested by nurse (short press)");
        transitionToState(SystemState::INPUT_PAUSED);
        return;
      }
      if (_button->wasLongPressed()) {
        Serial.println("Calibration requested by nurse (long press)");
        transitionToState(SystemState::CALIBRATION);
        return;
      }
    }
    void handleAlarmState() {
      if (_button->wasShortPressed()) {
        Serial.println("Alarm cleared by nurse");
        _alert->clearAlarm();
        transitionToState(SystemState::POLLING);
      }
    }
    void handlePauseState() {
      if (isPauseDurationExpired()) {
        Serial.println("Pause expired, resuming monitoring");
        transitionToState(SystemState::POLLING);
      }
    }
    void handleCalibrationState() {
      if (isCalibrationDurationExpired()) {
        _calibrationThreshold = _sensor->getPressurePercentage();
        Serial.print("Calibration complete. New threshold: ");
        Serial.println(_calibrationThreshold);
        transitionToState(SystemState::POLLING);
      }
    }
    bool isPauseDurationExpired() {
      return (millis() - _stateStartTime) >= PAUSE_DURATION_MS;
    }
    bool isCalibrationDurationExpired() {
      return (millis() - _stateStartTime) >= CALIB_DURATION_MS;
    }
    void transitionToState(SystemState newState) {
      logStateTransition(newState);
      _currentState = newState;
      _stateStartTime = millis();
      if (newState == SystemState::POLLING) {
        _wasOccupied = _sensor->isOccupied();
      }
    }
    void logStateTransition(SystemState newState) {
      Serial.print("Transitioning to state: ");
      switch (newState) {
        case SystemState::SYSTEM_OFF: Serial.println("SYSTEM_OFF"); break;
        case SystemState::IDLE: Serial.println("IDLE"); break;
        case SystemState::POLLING: Serial.println("POLLING"); break;
        case SystemState::INPUT_PAUSED: Serial.println("INPUT_PAUSED"); break;
        case SystemState::CALIBRATION: Serial.println("CALIBRATION"); break;
        case SystemState::ALARM: Serial.println("ALARM"); break;
      }
    }
  public:
    FallDetector(IForceSensor* sensor, INurseInput* button, IAlertSystem* alert)
      : _sensor(sensor), _button(button), _alert(alert),
        _currentState(SystemState::SYSTEM_OFF), _stateStartTime(0),
        _wasOccupied(false), _calibrationThreshold(DEFAULT_PRESSURE_THRESHOLD) {}
    void init() {
      Serial.println("Initializing Fall Detection System...");
      _sensor->init();
      _button->init();
      _alert->init();
      transitionToState(SystemState::IDLE);
      Serial.println("System Armed - Ready for operation");
    }
    void update() {
      _button->update();
      switch (_currentState) {
        case SystemState::IDLE: handleIdleState(); break;
        case SystemState::POLLING: handlePollingState(); break;
        case SystemState::INPUT_PAUSED: handlePauseState(); break;
        case SystemState::CALIBRATION: handleCalibrationState(); break;
        case SystemState::ALARM: handleAlarmState(); break;
        case SystemState::SYSTEM_OFF: break;
      }
    }
    SystemState getCurrentState() const {
      return _currentState;
    }
};

// ============================================================================
// ARDUINO SKETCH ENTRY POINT
// ============================================================================

FallDetector* systemController = nullptr;


void setup() {
    delay(2000); // Give the VS Code extension 2 seconds to hook the serial port
    Serial.begin(115200);
    Serial.println("Reboot successful");}
// void setup() {
//     Serial.begin(SERIAL_BAUD_RATE);
//     delay(1000); // Wait for the monitor to catch up
    
//     Serial.println("========================================");
//     Serial.println("Patient Fall Detection System - V4 Board");
//     Serial.println("========================================");

//     // Dependency Injection
//     auto* sensor = new WokwiPotentiometer(34, DEFAULT_PRESSURE_THRESHOLD);
//     auto* button = new WokwiButton(15);
//     auto* alert  = new SerialConsoleAlert();
    
//     systemController = new FallDetector(sensor, button, alert);
//     systemController->init();
    
//     Serial.println("Setup Complete - System Armed");
// }

// void setup() {
//     Serial.begin(SERIAL_BAUD_RATE);
//     while (!Serial) { delay(10); } // Wait for serial port to connect
//     delay(500); // Brief pause to let the buffer clear

//     Serial.println("========================================");
//     Serial.println("Patient Fall Detection System");
//     Serial.println("========================================");
//     Serial.println("Running in SIMULATION mode (Wokwi)");
//     auto* sensor = new WokwiPotentiometer(34, DEFAULT_PRESSURE_THRESHOLD); // Pin 34 (ADC1_CH6)
//     auto* button = new WokwiButton(15);                                   // Pin 15
//     auto* alert  = new SerialConsoleAlert();
//     systemController = new FallDetector(sensor, button, alert);
//     systemController->init();
//     Serial.println("========================================");
//     Serial.println("Setup complete - Entering main loop");
//     Serial.println("========================================");
// }

void loop() {
  systemController->update();
  delay(SENSOR_SAMPLE_RATE_MS);
}
