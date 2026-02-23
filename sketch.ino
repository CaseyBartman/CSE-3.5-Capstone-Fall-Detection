// Auto-generated sketch.ino - Do not edit directly

#define IS_SIMULATION 1

// Running in SIMULATION mode (Wokwi)


// ===== include/constants\NetworkConstants.h =====

#pragma once

// WiFi Configuration (for production use)
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Connexxall API Configuration
const char* CONNEXXALL_API_ENDPOINT = "https://api.connexxall.com/alerts";
const char* CONNEXXALL_API_KEY = "YOUR_API_KEY";

// BLE Configuration
const char* BLE_DEVICE_MAC = "YOUR_BLE_MAC_ADDRESS";
const char* BLE_SERVICE_UUID = "YOUR_SERVICE_UUID";


// ===== include/constants\SystemConstants.h =====

#pragma once

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


// ===== include/interfaces\IAlertSystem.h =====

#pragma once

class IAlertSystem {
public:
    virtual void init() = 0;
    virtual void triggerFallAlarm() = 0;
    virtual void clearAlarm() = 0;
    virtual ~IAlertSystem() {}
};


// ===== include/interfaces\IForceSensor.h =====

#pragma once

class IForceSensor {
public:
    virtual void init() = 0;
    virtual float getPressurePercentage() = 0; // Returns 0.0 to 100.0
    virtual bool isOccupied() = 0;             // Returns true if > threshold
    virtual ~IForceSensor() {}
};


// ===== include/interfaces\INurseInput.h =====

#pragma once

class INurseInput {
public:
    virtual void init() = 0;
    virtual bool isPressed() = 0;              // Returns true if button is currently pressed
    virtual bool wasShortPressed() = 0;        // Returns true if short press detected
    virtual bool wasLongPressed() = 0;         // Returns true if long press (hold) detected
    virtual void update() = 0;                 // Call in loop to process button state
    virtual ~INurseInput() {}
};


// ===== include/models\SensorPayload.h =====

#pragma once

struct SensorPayload {
    float pressurePercentage;
    bool isOccupied;
    unsigned long timestamp;
    
    SensorPayload() : pressurePercentage(0.0f), isOccupied(false), timestamp(0) {}
    
    SensorPayload(float pressure, bool occupied, unsigned long time)
        : pressurePercentage(pressure), isOccupied(occupied), timestamp(time) {}
};


// ===== include/models\SystemState.h =====

#pragma once

enum class SystemState {
    SYSTEM_OFF,
    IDLE,
    POLLING,
    INPUT_PAUSED,
    CALIBRATION,
    ALARM
};


// ===== test/generators\SensorDataGenerator.h =====

#pragma once

#include <vector>

/**
 * Sensor Data Generator for Testing
 * Generates realistic sensor data patterns for testing
 */
class SensorDataGenerator {
public:
    /**
     * Generate a sequence showing gradual pressure increase (patient sitting down)
     */
    static std::vector<float> generateSittingDownSequence() {
        return {5.0f, 10.0f, 20.0f, 30.0f, 40.0f, 45.0f};
    }

    /**
     * Generate a sequence showing rapid pressure decrease (patient standing up/falling)
     */
    static std::vector<float> generateStandingUpSequence() {
        return {45.0f, 40.0f, 30.0f, 15.0f, 5.0f, 2.0f};
    }

    /**
     * Generate stable occupied pressure readings
     */
    static std::vector<float> generateStableOccupiedSequence(int count = 10) {
        std::vector<float> sequence;
        for (int i = 0; i < count; i++) {
            sequence.push_back(40.0f + (i % 3) - 1.0f); // Small variations around 40%
        }
        return sequence;
    }

    /**
     * Generate stable unoccupied pressure readings
     */
    static std::vector<float> generateStableUnoccupiedSequence(int count = 10) {
        std::vector<float> sequence;
        for (int i = 0; i < count; i++) {
            sequence.push_back(2.0f + (i % 2) * 0.5f); // Small variations around 2%
        }
        return sequence;
    }

    /**
     * Generate a complete fall detection scenario
     * Patient sits -> stable -> stands up -> unoccupied
     */
    static std::vector<float> generateFallScenario() {
        std::vector<float> scenario;
        
        // Sitting down
        auto sitting = generateSittingDownSequence();
        scenario.insert(scenario.end(), sitting.begin(), sitting.end());
        
        // Stable occupied
        auto stable = generateStableOccupiedSequence(20);
        scenario.insert(scenario.end(), stable.begin(), stable.end());
        
        // Standing up (fall detection)
        auto standing = generateStandingUpSequence();
        scenario.insert(scenario.end(), standing.begin(), standing.end());
        
        // Unoccupied
        auto unoccupied = generateStableUnoccupiedSequence(10);
        scenario.insert(scenario.end(), unoccupied.begin(), unoccupied.end());
        
        return scenario;
    }

    /**
     * Create a SensorPayload from pressure reading
     */
    static SensorPayload createPayload(float pressure, unsigned long timestamp = 0) {
        bool occupied = pressure > 15.0f; // Default threshold
        return SensorPayload(pressure, occupied, timestamp);
    }
};


// ===== test/helpers\MockSensors.h =====

#pragma once



/**
 * Mock Force Sensor for Unit Testing
 * Allows manual control of sensor readings and state
 */
class MockForceSensor : public IForceSensor {
private:
    float _mockPressure;
    float _threshold;
    bool _initialized;

public:
    MockForceSensor(float threshold = 15.0f) 
        : _mockPressure(0.0f), _threshold(threshold), _initialized(false) {}

    void init() override {
        _initialized = true;
    }

    float getPressurePercentage() override {
        return _mockPressure;
    }

    bool isOccupied() override {
        return _mockPressure > _threshold;
    }

    // Test helper methods
    void setMockPressure(float pressure) {
        _mockPressure = pressure;
    }

    bool isInitialized() const {
        return _initialized;
    }
};

/**
 * Mock Nurse Input for Unit Testing
 * Allows simulation of button presses
 */
class MockNurseInput : public INurseInput {
private:
    bool _shortPress;
    bool _longPress;
    bool _initialized;

public:
    MockNurseInput() 
        : _shortPress(false), _longPress(false), _initialized(false) {}

    void init() override {
        _initialized = true;
    }

    void update() override {
        // In mock, we manually set the press flags
    }

    bool isPressed() override {
        return false;
    }

    bool wasShortPressed() override {
        bool result = _shortPress;
        _shortPress = false; // Clear flag after reading
        return result;
    }

    bool wasLongPressed() override {
        bool result = _longPress;
        _longPress = false; // Clear flag after reading
        return result;
    }

    // Test helper methods
    void simulateShortPress() {
        _shortPress = true;
    }

    void simulateLongPress() {
        _longPress = true;
    }

    bool isInitialized() const {
        return _initialized;
    }
};

/**
 * Mock Alert System for Unit Testing
 * Tracks whether alarms were triggered/cleared
 */
class MockAlertSystem : public IAlertSystem {
private:
    bool _initialized;
    int _alarmTriggeredCount;
    int _alarmClearedCount;
    bool _currentAlarmState;

public:
    MockAlertSystem() 
        : _initialized(false), _alarmTriggeredCount(0), 
          _alarmClearedCount(0), _currentAlarmState(false) {}

    void init() override {
        _initialized = true;
    }

    void triggerFallAlarm() override {
        _alarmTriggeredCount++;
        _currentAlarmState = true;
    }

    void clearAlarm() override {
        _alarmClearedCount++;
        _currentAlarmState = false;
    }

    // Test helper methods
    int getAlarmTriggeredCount() const {
        return _alarmTriggeredCount;
    }

    int getAlarmClearedCount() const {
        return _alarmClearedCount;
    }

    bool isAlarmActive() const {
        return _currentAlarmState;
    }

    bool isInitialized() const {
        return _initialized;
    }

    void resetCounters() {
        _alarmTriggeredCount = 0;
        _alarmClearedCount = 0;
        _currentAlarmState = false;
    }
};


// ===== src/logic\FallDetector.h =====

#pragma once





class FallDetector {
private:
    IForceSensor* _sensor;
    INurseInput* _button;
    IAlertSystem* _alert;
    
    SystemState _currentState;
    unsigned long _stateStartTime;
    bool _wasOccupied;
    float _calibrationThreshold;

    // Helper functions (Stepdown Rule)
    void handleIdleState();
    void handlePollingState();
    void handleAlarmState();
    void handlePauseState();
    void handleCalibrationState();
    bool isPauseDurationExpired();
    bool isCalibrationDurationExpired();
    void transitionToState(SystemState newState);
    void logStateTransition(SystemState newState);

public:
    // Constructor Injection
    FallDetector(IForceSensor* sensor, INurseInput* button, IAlertSystem* alert);
    
    void init();
    void update(); // Called every loop()
    SystemState getCurrentState() const { return _currentState; }
};


// ===== src/drivers/real\BlueCharmBLE.cpp =====



#include <Arduino.h>

class BlueCharmBLE : public INurseInput {
private:
    const char* _deviceMAC;
    bool _shortPressFlag;
    bool _longPressFlag;
    bool _connected;

public:
    BlueCharmBLE(const char* deviceMAC) 
        : _deviceMAC(deviceMAC), _shortPressFlag(false), 
          _longPressFlag(false), _connected(false) {}

    void init() override {
        Serial.println("Initializing Blue Charm BC B2 BLE connection...");
        // TODO: Implement actual BLE initialization
        // This would include:
        // - BLE stack initialization
        // - Scanning for the device with _deviceMAC
        // - Connecting to the device
        // - Subscribing to button press notifications
        Serial.print("Attempting to connect to BLE device: ");
        Serial.println(_deviceMAC);
    }

    void update() override {
        // Clear flags
        _shortPressFlag = false;
        _longPressFlag = false;
        
        // TODO: Implement BLE event processing
        // This would check for incoming BLE notifications
        // and set the appropriate flags based on button press type
    }

    bool isPressed() override {
        // TODO: Return actual button state from BLE
        return false;
    }

    bool wasShortPressed() override {
        return _shortPressFlag;
    }

    bool wasLongPressed() override {
        return _longPressFlag;
    }

    bool isConnected() const {
        return _connected;
    }
};


// ===== src/drivers/real\ConnexxallWiFi.cpp =====



#include <Arduino.h>

class ConnexxallWiFi : public IAlertSystem {
private:
    const char* _ssid;
    const char* _password;
    bool _wifiConnected;
    bool _alarmActive;

public:
    ConnexxallWiFi(const char* ssid, const char* password)
        : _ssid(ssid), _password(password), 
          _wifiConnected(false), _alarmActive(false) {}

    void init() override {
        Serial.println("Initializing WiFi and Connexxall connection...");
        // TODO: Implement actual WiFi initialization
        // This would include:
        // - WiFi.begin(_ssid, _password);
        // - Wait for connection
        // - Set up HTTP client for Connexxall API
        Serial.print("Connecting to WiFi: ");
        Serial.println(_ssid);
    }

    void triggerFallAlarm() override {
        _alarmActive = true;
        Serial.println("Sending fall alarm to Connexxall...");
        
        // TODO: Implement actual HTTP POST to Connexxall API
        // This would include:
        // - Create JSON payload with alert details
        // - POST to CONNEXXALL_API_ENDPOINT
        // - Include CONNEXXALL_API_KEY in headers
        // - Handle response and retry logic
        
        Serial.println("Fall alarm sent to Connexxall");
    }

    void clearAlarm() override {
        if (_alarmActive) {
            _alarmActive = false;
            Serial.println("Clearing alarm in Connexxall...");
            
            // TODO: Implement actual HTTP request to clear alarm
            // This might be a DELETE or PUT request depending on Connexxall API
            
            Serial.println("Alarm cleared in Connexxall");
        }
    }

    bool isWiFiConnected() const {
        // TODO: Return WiFi.status() == WL_CONNECTED
        return _wifiConnected;
    }
};


// ===== src/drivers/real\TekscanA502.cpp =====


#include <Arduino.h>

class TekscanA502 : public IForceSensor {
private:
    const int _pin;
    const float _threshold;
    float _zeroOffset;

public:
    TekscanA502(int pin, float threshold = 15.0f) 
        : _pin(pin), _threshold(threshold), _zeroOffset(0.0f) {}

    void init() override {
        pinMode(_pin, INPUT);
        Serial.println("TekscanA502 force sensor initialized");
        // TODO: Implement actual Tekscan A502 initialization
        // This may include I2C/SPI setup or specific calibration routines
    }

    float getPressurePercentage() override {
        // TODO: Implement actual Tekscan A502 reading logic
        // This is a placeholder that reads from ADC
        int raw = analogRead(_pin);
        float pressure = (float)raw / 40.95f;
        return pressure - _zeroOffset;
    }

    bool isOccupied() override {
        return getPressurePercentage() > _threshold;
    }

    void calibrateZero() {
        // Read current pressure and set as zero offset
        int raw = analogRead(_pin);
        _zeroOffset = (float)raw / 40.95f;
        Serial.print("Zero offset calibrated to: ");
        Serial.println(_zeroOffset);
    }
};


// ===== src/drivers/sim\SerialConsoleAlert.cpp =====


#include <Arduino.h>

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


// ===== src/drivers/sim\WokwiButton.cpp =====



#include <Arduino.h>

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


// ===== src/drivers/sim\WokwiPotentiometer.cpp =====


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


// ===== src/logic\FallDetector.cpp =====


#include <Arduino.h>

FallDetector::FallDetector(IForceSensor* sensor, INurseInput* button, IAlertSystem* alert)
    : _sensor(sensor), _button(button), _alert(alert),
      _currentState(SystemState::SYSTEM_OFF), _stateStartTime(0),
      _wasOccupied(false), _calibrationThreshold(DEFAULT_PRESSURE_THRESHOLD) {
}

void FallDetector::init() {
    Serial.println("Initializing Fall Detection System...");
    
    // Initialize all hardware components
    _sensor->init();
    _button->init();
    _alert->init();
    
    // Boot up system - transition to IDLE
    transitionToState(SystemState::IDLE);
    Serial.println("System Armed - Ready for operation");
}

void FallDetector::update() {
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

void FallDetector::handleIdleState() {
    // Check if sensor is ready and WiFi connected (simulated as always ready in Wokwi)
    // In production, you would check WiFi.status() == WL_CONNECTED
    bool isReady = true; // Simplified for now
    
    if (isReady) {
        transitionToState(SystemState::POLLING);
    }
}

void FallDetector::handlePollingState() {
    // Read sensor data
    float pressure = _sensor->getPressurePercentage();
    bool occupied = _sensor->isOccupied();
    
    // Track occupation state changes
    if (!_wasOccupied && occupied) {
        _wasOccupied = true;
    }
    
    // Detect stand-up (fall detection): was occupied, now pressure dropped significantly
    if (_wasOccupied && !occupied && pressure < FALL_DETECTION_THRESHOLD) {
        Serial.println("Fall detected! Occupant left bed.");
        _alert->triggerFallAlarm();
        transitionToState(SystemState::ALARM);
        return;
    }
    
    // Check for nurse button presses
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

void FallDetector::handleAlarmState() {
    // Alarm is already triggered in transitionToState
    // Wait for nurse to press button to clear alarm
    if (_button->wasShortPressed()) {
        Serial.println("Nurse pressed button - Clearing alarm");
        _alert->clearAlarm();
        transitionToState(SystemState::POLLING);
    }
}

void FallDetector::handlePauseState() {
    // Check if pause duration expired, resume monitoring
    if (isPauseDurationExpired()) {
        Serial.println("Pause expired, resuming monitoring");
        transitionToState(SystemState::POLLING);
    }
}

void FallDetector::handleCalibrationState() {
    // During calibration, continuously sample the sensor
    float pressure = _sensor->getPressurePercentage();
    
    if (isCalibrationDurationExpired()) {
        // Save new threshold (current pressure reading)
        _calibrationThreshold = pressure;
        Serial.print("Calibration complete - New threshold: ");
        Serial.print(_calibrationThreshold);
        Serial.println("%");
        transitionToState(SystemState::POLLING);
    }
}

bool FallDetector::isPauseDurationExpired() {
    return (millis() - _stateStartTime) >= PAUSE_DURATION_MS;
}

bool FallDetector::isCalibrationDurationExpired() {
    return (millis() - _stateStartTime) >= CALIB_DURATION_MS;
}

void FallDetector::transitionToState(SystemState newState) {
    if (newState == _currentState) {
        return; // Already in this state
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

void FallDetector::logStateTransition(SystemState newState) {
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


// ===== src/main.cpp =====

#include <Arduino.h>


// Conditional imports based on platformio.ini build flags
#ifdef IS_SIMULATION



#else




#endif

// Global pointer (Initialized in setup)
FallDetector* systemController = nullptr;

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000); // Allow serial to initialize
    
    Serial.println("========================================");
    Serial.println("Patient Fall Detection System");
    Serial.println("========================================");

    // 1. Instantiate drivers based on environment
    #ifdef IS_SIMULATION
        Serial.println("Running in SIMULATION mode (Wokwi)");
        auto* sensor = new WokwiPotentiometer(34, DEFAULT_PRESSURE_THRESHOLD);
        auto* button = new WokwiButton(15);
        auto* alert  = new SerialConsoleAlert();
    #else
        Serial.println("Running in PRODUCTION mode (Real Hardware)");
        auto* sensor = new TekscanA502(34, DEFAULT_PRESSURE_THRESHOLD);
        auto* button = new BlueCharmBLE(BLE_DEVICE_MAC);
        auto* alert  = new ConnexxallWiFi(WIFI_SSID, WIFI_PASSWORD);
    #endif

    // 2. Inject dependencies into logic controller
    systemController = new FallDetector(sensor, button, alert);
    
    // 3. Initialize the system
    systemController->init();
    
    Serial.println("========================================");
    Serial.println("Setup complete - Entering main loop");
    Serial.println("========================================");
}

void loop() {
    // 4. Delegate to logic controller
    systemController->update();
    
    // Small delay for system stability
    delay(SENSOR_SAMPLE_RATE_MS);
}

