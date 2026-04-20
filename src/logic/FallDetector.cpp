#include "logic/FallDetector.h"
#include <Arduino.h>

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
            break;
    }
}

void FallDetector::handleIdleState() {
    bool isReady = true;
    
    if (isReady) {
        transitionToState(SystemState::POLLING);
    }
}

void FallDetector::handlePollingState() {
    float pressure = _sensor->getPressurePercentage();
    bool occupied = _sensor->isOccupied();
    
    if (occupied && pressure >= _calibrationThreshold) {
        Serial.println("Alert! Person detected on mat without supervision.");
        transitionToState(SystemState::ALARM);
        return;
    }

    if (_button->wasShortPressed()) {
        Serial.println("Pause requested by nurse (short press)");
        transitionToState(SystemState::INPUT_PAUSED);
        return;
    }

}

void FallDetector::handleAlarmState() {
    if (_button->wasShortPressed()) {
        Serial.println("Nurse pressed button - Clearing alarm");
        _alert->clearAlarm();
        transitionToState(SystemState::POLLING);
    }
}

void FallDetector::handlePauseState() {
    if (_button->wasShortPressed()) {
        Serial.println("Calibration requested by nurse (short press from pause)");
        transitionToState(SystemState::CALIBRATION);
        return;
    }
    
    if (isPauseDurationExpired()) {
        Serial.println("Pause expired, resuming monitoring");
        transitionToState(SystemState::POLLING);
    }
}

void FallDetector::handleCalibrationState() {
    float pressure = _sensor->getPressurePercentage();
    
    if (isCalibrationDurationExpired() || _button->wasShortPressed()) {
        _calibrationThreshold = pressure;
        Serial.print("Calibration complete - New threshold: ");
        Serial.print(_calibrationThreshold, 4);
        Serial.println("%");
        delay(CALIB_COMPLETE_DELAY_MS);
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
