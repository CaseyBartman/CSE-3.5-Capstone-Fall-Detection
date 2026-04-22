// Auto-generated sketch.ino - Do not edit directly

#define IS_SIMULATION 1

// Running in SIMULATION mode (Wokwi)


// ===== include/constants\NetworkConstants.h =====

/**
 * @file NetworkConstants.h
 * @brief Network and alert endpoint configuration
 * 
 * Contains WiFi credentials, API endpoints, and alert service URLs.
 * Separated from system constants for security and ease of configuration.
 * 
 * WARNING: Credentials should be moved to secure config storage in production.
 * 
 * Services:
 *   - Ntfy: https://ntfy.sh/ (text-based HTTP push notifications)
 *   - Connexxall: Legacy healthcare integration (deprecated)
 *   - Wokwi: Simulated WiFi network for testing
 */

#pragma once

// WiFi Configuration
const char* WIFI_SSID = "WIFI_SSID";
const char* WIFI_PASSWORD = "WIFI_PASSWORD";
const char* WOKWI_WIFI_SSID = "Wokwi-GUEST";

// Connexxall API Configuration
const char* CONNEXXALL_API_ENDPOINT = "https://api.connexxall.com/alerts";
const char* CONNEXXALL_API_KEY = "API_KEY";

// BLE Configuration
const char* BLE_DEVICE_MAC = "BLE_MAC_ADDRESS";
const char* BLE_SERVICE_UUID = "SERVICE_UUID";

// Ntfy HTTP Configuration
const char* NTFY_HTTP_ENDPOINT = "https://ntfy.sh/patient_fall_alert_5911";

// ===== include/constants\SystemConstants.h =====

/**
 * @file SystemConstants.h
 * @brief System-wide timing and threshold constants
 * 
 * Centralized configuration for all system timings, pressure thresholds,
 * pin assignments, and sampling rates. These values must match the physical
 * hardware capabilities and user requirements.
 * 
 * Key Thresholds:
 *   - PAUSE_DURATION_MS: How long nurse can pause system (2 minutes)
 *   - CALIB_DURATION_MS: How long calibration samples baseline (5 seconds)
 *   - FALL_DETECTION_THRESHOLD: Pressure rise that triggers alarm (30%)
 *   - DEFAULT_PRESSURE_THRESHOLD: Baseline to detect occupancy (5%)
 * 
 * Pin Configuration (Simulation):
 *   - SIM_SENSOR_PIN: GPIO 34 (analog potentiometer)
 *   - SIM_BUTTON_PIN: GPIO 15 (digital button)
 */

#pragma once

// Timing Constants (in milliseconds)
const unsigned long PAUSE_DURATION_MS = 120000;  // 2 minutes pause duration
const unsigned long CALIB_DURATION_MS = 5000;    // 5 seconds calibration duration
const unsigned long DEBOUNCE_DELAY_MS = 50;      // Button debounce delay
const unsigned long LONG_PRESS_MS = 2000;        // Long press threshold

// Sensor Thresholds
const float DEFAULT_PRESSURE_THRESHOLD = 15.0;   // Default 15% threshold for occupied state
const float FALL_DETECTION_THRESHOLD = 30.0;      // Threshold for detecting stand-up (rise above this %)

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


// ===== include/interfaces\INetworkClient.h =====

#pragma once
#include <string>

class INetworkClient {
public:
    virtual ~INetworkClient() = default;
    virtual bool isNetworkConnected() const = 0;
    virtual int post(const char* endpointUrl, const char* contentType, const char* payload) = 0;
    virtual const char* getResponseBody() const = 0; // For logging
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


// ===== src/logic/FallDetector.h =====

/**
 * @file FallDetector.h
 * @brief Core state machine for patient fall detection system
 * 
 * Implements a 6-state finite state machine that monitors patient status
 * and triggers alerts when a fall is detected. Uses dependency injection
 * to remain hardware-agnostic and fully testable.
 * 
 * States: SYSTEM_OFF -> IDLE -> POLLING -> [ALARM | INPUT_PAUSED | CALIBRATION]
 * 
 * Dependencies:
 *   - IForceSensor: Pressure/occupancy monitoring
 *   - INurseInput: Button input for pause/calibration
 *   - IAlertSystem: Alert triggering and clearing
 */

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


// ===== src/logic/NtfyHttpAlert.h =====

#pragma once


#include <Arduino.h> // For Serial

class NtfyHttpAlert : public IAlertSystem {
private:
    INetworkClient* _networkClient;
    const char* _endpointUrl;
    bool _isAlarmActive;

    bool sendNotification(const char* messagePayload) {
        if (!_networkClient->isNetworkConnected()) {
            Serial.println("ERROR: Network unavailable for transmission");
            return false;
        }
        
        const char* textContentType = "text/plain";
        int responseStatusCode = _networkClient->post(_endpointUrl, textContentType, messagePayload);
        
        Serial.print("Notification sent. Response status code: ");
        Serial.println(responseStatusCode);

        bool isSuccess = responseStatusCode > 0 && responseStatusCode < 400;
        if (!isSuccess) {
            Serial.print("ERROR: HTTP POST failed. Response body: ");
            Serial.println(_networkClient->getResponseBody());
        }
        
        return isSuccess;
    }

public:
    NtfyHttpAlert(INetworkClient* networkClient, const char* endpointUrl) 
        : _networkClient(networkClient), _endpointUrl(endpointUrl), _isAlarmActive(false) {}

    void init() override {
        if (!_networkClient->isNetworkConnected()) {
            Serial.println("WARNING: Network disconnected during initialization");
        }
    }

    void triggerFallAlarm() override {
        _isAlarmActive = true;
        if (!sendNotification("PATIENT HAS FALLEN!")) {
            Serial.println("DEBUG: Fall alarm transmission failed");
        }
    }

    void clearAlarm() override {
        if (_isAlarmActive) {
            _isAlarmActive = false;
            if (!sendNotification("ALARM CLEARED")) {
                Serial.println("DEBUG: Alarm clear transmission failed");
            }
        }
    }
};


// ===== src/drivers/real/EspNetworkClient.h =====

#pragma once

#include <WiFi.h>
#include <HTTPClient.h>

class EspNetworkClient : public INetworkClient {
private:
    String _lastResponseBody; // Use Arduino String to hold response body

public:
    bool isNetworkConnected() const override {
        return WiFi.status() == WL_CONNECTED;
    }

    int post(const char* endpointUrl, const char* contentType, const char* payload) override {
        HTTPClient httpClient;
        httpClient.begin(endpointUrl);
        httpClient.addHeader("Content-Type", contentType);
        
        int responseStatusCode = httpClient.POST(payload);
        
        if (responseStatusCode < 0 || responseStatusCode >= 400) {
            _lastResponseBody = httpClient.getString();
        } else {
            _lastResponseBody = "";
        }
        
        httpClient.end();
        
        return responseStatusCode;
    }

    const char* getResponseBody() const override {
        return _lastResponseBody.c_str();
    }
};


// ===== src/util\WiFiSetup.cpp =====

#include <WiFi.h>

class WiFiSetup {
public:
    static void setupWifi(const char* ssid, const char* password = "") {
        WiFi.begin(ssid, password);

        while (WiFi.status() != WL_CONNECTED) {
            delay(1000);
            Serial.println("Connecting...");
        }

        Serial.print("Connected to IP Address: ");
        Serial.println(WiFi.localIP());
    }
};


// ===== src/drivers/sim\SerialConsoleAlert.cpp =====

/**
 * @file SerialConsoleAlert.cpp
 * @brief Simulated alert system for Wokwi Arduino simulation
 * 
 * Logs fall detection alerts to the Serial monitor instead of sending
 * to a real alert system (Ntfy, Connexxall, etc.). Used exclusively in
 * simulation mode for testing the state machine logic.
 * 
 * Output: Serial monitor console messages with visual separators
 * Baud: 115200
 */

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
        Serial.println("Alert sent to Ntfy (Simulated)");
        Serial.println("========================================");
    }

    void clearAlarm() override {
        if (_alarmActive) {
            _alarmActive = false;
            Serial.println("========================================");
            Serial.println("Alarm cleared by nurse");
            Serial.println("Ntfy alert cleared (Simulated)");
            Serial.println("========================================");
        }
    }
};


// ===== src/drivers/sim\WokwiButton.cpp =====

/**
 * @file WokwiButton.cpp
 * @brief Simulated nurse input button for Wokwi Arduino simulation
 * 
 * Implements button press detection with debouncing and differentiation
 * between short presses (<2 seconds) and long presses (>=2 seconds).
 * 
 * Pin: GPIO 15 (pull-up button)
 * Short Press: <2000ms
 * Long Press: >=2000ms (currently unused per manual)
 * Debounce: 50ms
 */

#include <Arduino.h>

class WokwiButton : public INurseInput {
private:
    const int _pin;
    bool _lastState;
    bool _currentState;
    unsigned long _pressStartTime;
    bool _hasShortPress;
    bool _hasLongPress;
    bool _isLongPressTriggered;

public:
    WokwiButton(int pin) 
        : _pin(pin), 
          _lastState(HIGH), 
          _currentState(HIGH),
          _pressStartTime(0), 
          _hasShortPress(false), 
          _hasLongPress(false), 
          _isLongPressTriggered(false) {}

    void init() override {
        pinMode(_pin, INPUT_PULLUP);
    }

    void update() override {
        resetPressFlags();
        
        _currentState = digitalRead(_pin);
        
        handleButtonLifecycle();
        
        _lastState = _currentState;
    }

    bool isPressed() override {
        return _currentState == LOW;
    }

    bool wasShortPressed() override {
        return _hasShortPress;
    }

    bool wasLongPressed() override {
        return _hasLongPress;
    }

private:
    void resetPressFlags() {
        _hasShortPress = false;
        _hasLongPress = false;
    }

    void handleButtonLifecycle() {
        if (buttonWasJustPressed()) {
            startPressTimer();
        } 
        else if (isButtonBeingHeld()) {
            checkLongPress();
        } 
        else if (wasButtonReleased()) {
            checkShortPress();
        }
    }

    bool buttonWasJustPressed() {
        return _currentState == LOW && _lastState == HIGH;
    }

    bool isButtonBeingHeld() {
        return _currentState == LOW && _lastState == LOW;
    }

    bool wasButtonReleased() {
        return _currentState == HIGH && _lastState == LOW;
    }

    void startPressTimer() {
        _pressStartTime = millis();
        _isLongPressTriggered = false;
    }

    void checkLongPress() {
        if (_isLongPressTriggered) return;

        if (getPressDuration() >= LONG_PRESS_MS) {
            _hasLongPress = true;
            _isLongPressTriggered = true;
            Serial.println("Long press detected");
        }
    }

    void checkShortPress() {
        if (getPressDuration() < LONG_PRESS_MS) {
            _hasShortPress = true;
            Serial.println("Short press detected");
        }
    }

    unsigned long getPressDuration() {
        return millis() - _pressStartTime;
    }
};

// ===== src/drivers/sim\WokwiPotentiometer.cpp =====

/**
 * @file WokwiPotentiometer.cpp
 * @brief Simulated pressure sensor for Wokwi Arduino simulation
 * 
 * Maps an analog potentiometer input (0-4095 ADC range) to a pressure
 * percentage (0-100%) to simulate a real force/pressure sensor in the
 * Wokwi simulator environment.
 * 
 * Pin: GPIO 34 (ADC input)
 * ADC Range: 0-4095 (12-bit)
 * Output: 0.0-100.0% pressure
 */

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

/**
 * @file FallDetector.cpp
 * @brief Implementation of the core state machine for fall detection
 * 
 * Implements all state handlers and transitions. The state machine follows
 * these rules:
 * 
 * IDLE: Initialization state - immediately transitions to POLLING
 * POLLING: Normal operation - monitors for falls and button input
 * ALARM: Fall detected - waits for nurse to acknowledge with button press
 * INPUT_PAUSED: Nurse paused system - waits 2 minutes or for calibration
 * CALIBRATION: Sampling baseline - runs for 5 seconds then returns to POLLING
 * SYSTEM_OFF: Powered down - used for shutdown/reset
 */

#include <Arduino.h>
#include <iostream>
#include <thread> // Required for std::this_thread::sleep_for
#include <chrono>

FallDetector::FallDetector(IForceSensor* sensor, INurseInput* button, IAlertSystem* alert)
    : _sensor(sensor), _button(button), _alert(alert),
      _currentState(SystemState::SYSTEM_OFF), _stateStartTime(0),
      _wasOccupied(false), _calibrationThreshold(DEFAULT_PRESSURE_THRESHOLD) {
}

void FallDetector::init() {
    Serial.println("Initializing Fall Detection System...");
    
    _sensor->init();
    _button->init();
    _alert->init();
    
    transitionToState(SystemState::IDLE);
    Serial.println("System Armed - Ready for operation");
}

void FallDetector::update() {
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
            // Tbd...
            break;
    }
}

void FallDetector::handleIdleState() {
    // Check if sensor is ready and WiFi connected (simulated as always ready in Wokwi)
    // In production, we would check something like WiFi.status() == WL_CONNECTED
    bool isReady = true; // Simplified for now
    
    if (isReady) {
        transitionToState(SystemState::POLLING);
    }
}

void FallDetector::handlePollingState() {
    float pressure = _sensor->getPressurePercentage();
    bool occupied = _sensor->isOccupied();
    
    if (occupied && pressure >= FALL_DETECTION_THRESHOLD) { //See that this is just a constant right now. We should have a whole system for detecitng the threshold, but this gets the job done for now
        Serial.println("Alert! Person detected on mat without supervision.");
        transitionToState(SystemState::ALARM);
        return;
    }

    // According to requirements: Short press → INPUT_PAUSED (pause for 2 mins)
    if (_button->wasShortPressed()) {
        Serial.println("Pause requested by nurse (short press)");
        transitionToState(SystemState::INPUT_PAUSED);
        return;
    }

    //THIS BEHAVIOR IS UNDEFINED.... what do we think?
    // if (_button->wasLongPressed()) {
    //     Serial.println("Calibration requested by nurse (long press)");
    //     transitionToState(SystemState::CALIBRATION);
    //     return;
    // }
}

void FallDetector::handleAlarmState() {
    if (_button->wasShortPressed()) {
        Serial.println("Nurse pressed button - Clearing alarm");
        _alert->clearAlarm();
        transitionToState(SystemState::POLLING);
    }
}

void FallDetector::handlePauseState() {
    // According to requirements: From INPUT_PAUSED, short press → CALIBRATION
    if (_button->wasShortPressed()) {
        Serial.println("Calibration requested by nurse (short press from pause)");
        transitionToState(SystemState::CALIBRATION);
        return;
    }
    
    // If pause timer expires, return to POLLING
    if (isPauseDurationExpired()) {
        Serial.println("Pause expired, resuming monitoring");
        transitionToState(SystemState::POLLING);
    }
}

void FallDetector::handleCalibrationState() {
    float pressure = _sensor->getPressurePercentage();
    
    //This is a very basic implementation- not at all reflective of how we should handle the calibration, but should work for a rough draft simulation
    if (isCalibrationDurationExpired() || _button->wasShortPressed()) {
        // Save new threshold (current pressure reading)? 
        _calibrationThreshold = pressure;
        Serial.print("Calibration complete - New threshold: ");
        Serial.print(_calibrationThreshold);
        Serial.println("%");
        std::this_thread::sleep_for(std::chrono::seconds(3)); //Wait so the state switch isn't immediate
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

void FallDetector::logStateTransition(SystemState newState) {
    Serial.print("State Transition: ");
    switch (_currentState) {
        case SystemState::SYSTEM_OFF: Serial.print("SYSTEM_OFF"); break;
        case SystemState::IDLE: Serial.print("IDLE"); break;
        case SystemState::POLLING: Serial.print("POLLING"); break;
        case SystemState::INPUT_PAUSED: Serial.print("INPUT_PAUSED"); break;
        case SystemState::CALIBRATION: Serial.print("CALIBRATION"); break;
        case SystemState::ALARM: Serial.print("ALARM"); break;
    }
    
    Serial.print(" -> ");
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

/**
 * @file main.cpp
 * @brief Composition root and Arduino entry point
 * 
 * Initializes hardware, wires dependencies via constructor injection,
 * and runs the main control loop. Supports three build modes:
 * 
 * 1. IS_SIMULATION: Wokwi simulation (potentiometer + button + Serial logging)
 * 2. IS_REAL: Real hardware (Tekscan + WiFi button + Ntfy alerts)
 * 3. IS_ARCHIVE: Legacy implementation (deprecated)
 * 
 * Dependency Injection Flow:
 *   setup() -> Creates sensor/button/alert instances -> FallDetector
 *   loop()  -> Calls systemController->update() every 50ms
 */

#include <Arduino.h>




// Conditional imports based on build flags
#ifdef IS_SIMULATION



#else



#endif

FallDetector* systemController = nullptr;

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000);
    
    Serial.println("========================================");
    Serial.println("Patient Fall Detection System");
    Serial.println("========================================");

    #ifdef IS_SIMULATION
        Serial.println("Running in SIMULATION mode (Wokwi)");
        WiFiSetup::setupWifi(WOKWI_WIFI_SSID);

        auto* sensor = new WokwiPotentiometer(34, DEFAULT_PRESSURE_THRESHOLD);
        auto* button = new WokwiButton(15);
        auto* networkClient = new EspNetworkClient(); // Use real network client in Sim
        auto* alert  = new NtfyHttpAlert(networkClient, NTFY_HTTP_ENDPOINT);
    #else
        Serial.println("Running in PRODUCTION mode (Real Hardware)");
        WiFiSetup::setupWifi(WIFI_SSID, WIFI_PASSWORD);

        auto* sensor = new TekscanA502(34, DEFAULT_PRESSURE_THRESHOLD);
        auto* button = new BlueCharmBLE(BLE_DEVICE_MAC);
        auto* networkClient = new EspNetworkClient();
        auto* alert  = new NtfyHttpAlert(networkClient, NTFY_HTTP_ENDPOINT);
    #endif

    systemController = new FallDetector(sensor, button, alert);
    systemController->init();
    
    Serial.println("========================================");
    Serial.println("Setup complete - Entering main loop");
    Serial.println("========================================");
}

void loop() {
    systemController->update();
    
    //Small delay for system stability. To be tested with physical sensor...
    delay(SENSOR_SAMPLE_RATE_MS);
}

