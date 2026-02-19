// Patient Fall Detection System - Wokwi Sketch
// This is a standalone Arduino sketch for Wokwi simulation
// It maintains the dependency injection architecture in a single file

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
// WOKWI SIMULATION DRIVERS
// ============================================================================

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
        // Clear previous flags
        _shortPressFlag = false;
        _longPressFlag = false;
        
        // Read current state (LOW = pressed with pullup)
        _currentState = digitalRead(_pin);
        
        // Button just pressed
        if (_currentState == LOW && _lastState == HIGH) {
            _pressStartTime = millis();
            _longPressTriggered = false;
        }
        
        // Button is being held
        if (_currentState == LOW && _lastState == LOW) {
            unsigned long pressDuration = millis() - _pressStartTime;
            
            // Long press detected
            if (pressDuration >= LONG_PRESS_MS && !_longPressTriggered) {
                _longPressFlag = true;
                _longPressTriggered = true;
                Serial.println("Long press detected");
            }
        }
        
        // Button just released
        if (_currentState == HIGH && _lastState == LOW) {
            unsigned long pressDuration = millis() - _pressStartTime;
            
            // Short press (released before long press threshold)
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
// FALL DETECTOR (State Machine Logic)
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
        // Check if sensor is ready
        bool isReady = true; // Simplified for simulation
        
        if (isReady) {
            transitionToState(SystemState::POLLING);
        }
    }

    void handlePollingState() {
        // Read sensor data
        float pressure = _sensor->getPressurePercentage();
        bool currentlyOccupied = _sensor->isOccupied();
        
        // Detect stand-up (fall detection): was occupied, now pressure dropped significantly
        if (_wasOccupied && pressure < FALL_DETECTION_THRESHOLD) {
            Serial.println("Fall detected! Pressure dropped significantly.");
            transitionToState(SystemState::ALARM);
            return;
        }
        
        // Update occupied state
        _wasOccupied = currentlyOccupied;
        
        // Check for nurse button hold (long press) -> pause
        if (_button->wasLongPressed()) {
            Serial.println("Nurse pressed hold button - Pausing input");
            transitionToState(SystemState::INPUT_PAUSED);
            return;
        }
    }

    void handleAlarmState() {
        // Wait for nurse to press button to clear alarm
        if (_button->wasShortPressed()) {
            Serial.println("Nurse pressed button - Clearing alarm");
            _alert->clearAlarm();
            transitionToState(SystemState::POLLING);
        }
    }

    void handlePauseState() {
        // Check for short press to start calibration
        if (_button->wasShortPressed()) {
            Serial.println("Starting calibration sequence");
            transitionToState(SystemState::CALIBRATION);
            return;
        }
        
        // Check if pause duration expired
        if (isPauseDurationExpired()) {
            Serial.println("Pause duration expired - Resuming monitoring");
            transitionToState(SystemState::POLLING);
        }
    }

    void handleCalibrationState() {
        // During calibration, continuously sample the sensor
        float pressure = _sensor->getPressurePercentage();
        
        if (isCalibrationDurationExpired()) {
            // Save new threshold
            _calibrationThreshold = pressure;
            Serial.print("Calibration complete - New threshold: ");
            Serial.print(_calibrationThreshold);
            Serial.println("%");
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
        if (newState == _currentState) {
            return;
        }
        
        logStateTransition(newState);
        _currentState = newState;
        _stateStartTime = millis();
        
        // Execute state entry actions
        switch (newState) {
            case SystemState::IDLE:
                Serial.println("System in IDLE state - Waiting for readiness");
                break;
            case SystemState::POLLING:
                Serial.println("System Armed - Monitoring for falls");
                _wasOccupied = _sensor->isOccupied();
                break;
            case SystemState::INPUT_PAUSED:
                Serial.println("System Paused - Input monitoring suspended for 2 minutes");
                break;
            case SystemState::CALIBRATION:
                Serial.println("Calibration Mode - Sampling sensor for baseline");
                break;
            case SystemState::ALARM:
                Serial.println("ALARM! Fall detected - Triggering alert");
                _alert->triggerFallAlarm();
                break;
            case SystemState::SYSTEM_OFF:
                Serial.println("System shutting down");
                break;
        }
    }

    void logStateTransition(SystemState newState) {
        Serial.print("State Transition: ");
        
        // Log current state
        switch (_currentState) {
            case SystemState::SYSTEM_OFF: Serial.print("SYSTEM_OFF"); break;
            case SystemState::IDLE: Serial.print("IDLE"); break;
            case SystemState::POLLING: Serial.print("POLLING"); break;
            case SystemState::INPUT_PAUSED: Serial.print("INPUT_PAUSED"); break;
            case SystemState::CALIBRATION: Serial.print("CALIBRATION"); break;
            case SystemState::ALARM: Serial.print("ALARM"); break;
        }
        
        Serial.print(" -> ");
        
        // Log new state
        switch (newState) {
            case SystemState::SYSTEM_OFF: Serial.print("SYSTEM_OFF"); break;
            case SystemState::IDLE: Serial.print("IDLE"); break;
            case SystemState::POLLING: Serial.print("POLLING"); break;
            case SystemState::INPUT_PAUSED: Serial.print("INPUT_PAUSED"); break;
            case SystemState::CALIBRATION: Serial.print("CALIBRATION"); break;
            case SystemState::ALARM: Serial.print("ALARM"); break;
        }
        
        Serial.println();
    }

public:
    FallDetector(IForceSensor* sensor, INurseInput* button, IAlertSystem* alert)
        : _sensor(sensor), _button(button), _alert(alert),
          _currentState(SystemState::SYSTEM_OFF), _stateStartTime(0),
          _wasOccupied(false), _calibrationThreshold(DEFAULT_PRESSURE_THRESHOLD) {
    }

    void init() {
        Serial.println("Initializing Fall Detection System...");
        
        // Initialize all hardware components
        _sensor->init();
        _button->init();
        _alert->init();
        
        // Boot up system - transition to IDLE
        transitionToState(SystemState::IDLE);
        Serial.println("System Armed - Ready for operation");
    }

    void update() {
        // Update button state first
        _button->update();
        
        // State machine
        switch (_currentState) {
            case SystemState::IDLE:
                handleIdleState();
                break;
            case SystemState::POLLING:
                handlePollingState();
                break;
            case SystemState::INPUT_PAUSED:
                handlePauseState();
                break;
            case SystemState::CALIBRATION:
                handleCalibrationState();
                break;
            case SystemState::ALARM:
                handleAlarmState();
                break;
            case SystemState::SYSTEM_OFF:
                // Do nothing in off state
                break;
        }
    }

    SystemState getCurrentState() const { return _currentState; }
};

// ============================================================================
// ARDUINO SKETCH (Main Application)
// ============================================================================

// Global pointer (Initialized in setup)
FallDetector* systemController = nullptr;

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000); // Allow serial to initialize
    
    Serial.println("========================================");
    Serial.println("Patient Fall Detection System");
    Serial.println("========================================");
    Serial.println("Running in SIMULATION mode (Wokwi)");

    // 1. Instantiate simulation drivers
    auto* sensor = new WokwiPotentiometer(34, DEFAULT_PRESSURE_THRESHOLD); // Pin 34 (ADC1_CH6)
    auto* button = new WokwiButton(15);                                     // Pin 15
    auto* alert  = new SerialConsoleAlert();

    // 2. Inject dependencies into logic controller
    systemController = new FallDetector(sensor, button, alert);
    
    // 3. Initialize the system
    systemController->init();
    
    Serial.println("========================================");
    Serial.println("Setup complete - Entering main loop");
    Serial.println("========================================");
    Serial.println();
    Serial.println("INSTRUCTIONS:");
    Serial.println("- Move potentiometer UP (>15%) to simulate patient sitting");
    Serial.println("- Move potentiometer DOWN (<5%) to simulate fall/stand-up");
    Serial.println("- CLICK button briefly for short press");
    Serial.println("- HOLD button 2+ seconds for long press");
    Serial.println("========================================");
}

void loop() {
    // 4. Delegate to logic controller
    systemController->update();
    
    // Small delay for system stability
    delay(SENSOR_SAMPLE_RATE_MS);
}
