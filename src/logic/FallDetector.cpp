#include "logic/FallDetector.h"
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
