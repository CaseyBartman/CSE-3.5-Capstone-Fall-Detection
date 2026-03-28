// Auto-generated sketch.ino - Do not edit directly

#define IS_SIMULATION 1

// Running in SIMULATION mode (Wokwi)


// ===== include/constants\NetworkConstants.h =====

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


// ===== src/drivers/real\BlueCharmBLE.cpp =====



#include <Arduino.h>
#include <iostream>		
#include <thread> // Required for std::this_thread::sleep_for		
#include <chrono>

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
    bool _wifiConnected;
    bool _alarmActive;

public:
    ConnexxallWiFi()
        : _wifiConnected(false), _alarmActive(false) {}

    void init() override {
        Serial.println("Initializing Connexxall connection...");
        // TODO: Implement actual Connexxall connection initialization
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


// ===== src/drivers/sim\NtfyHttpAlert.cpp =====


#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

class NtfyHttpAlert : public IAlertSystem {
private:
    const char* _endpoint;
    bool _alarmActive;

    bool sendNotification(const char* message) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("ERROR: WiFi not connected");
            return false;
        }
        HTTPClient http;
        http.begin(_endpoint);
        http.addHeader("Content-Type", "text/plain");
        int responseCode = http.POST(message);
        http.end();
        return responseCode > 0;
    }

public:
    NtfyHttpAlert() : _endpoint(NTFY_HTTP_ENDPOINT), _alarmActive(false) {}

    void init() override {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WARNING: WiFi not connected during initialization");
        }
        Serial.println("Alert system initialized");
    }

    void triggerFallAlarm() override {
        _alarmActive = true;
        if (sendNotification("PATIENT HAS FALLEN!")) {
            Serial.println("Fall alarm sent to Ntfy");
        } else {
            Serial.println("Failed to send fall alarm to Ntfy");
        }
    }

    void clearAlarm() override {
        if (_alarmActive) {
            _alarmActive = false;
            if (sendNotification("ALARM CLEARED")) {
                Serial.println("Alarm cleared via Ntfy");
            } else {
                Serial.println("Failed to send alarm clear to Ntfy");
            }
        }
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
        auto* alert  = new NtfyHttpAlert();
    #else
        Serial.println("Running in PRODUCTION mode (Real Hardware)");
        WiFiSetup::setupWifi(WIFI_SSID, WIFI_PASSWORD);

        auto* sensor = new TekscanA502(34, DEFAULT_PRESSURE_THRESHOLD);
        auto* button = new BlueCharmBLE(BLE_DEVICE_MAC);
        auto* alert  = new ConnexxallWiFi(WIFI_SSID, WIFI_PASSWORD);
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

