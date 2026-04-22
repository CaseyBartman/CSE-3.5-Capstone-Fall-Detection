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

#include "interfaces/IForceSensor.h"
#include "interfaces/INurseInput.h"
#include "interfaces/IAlertSystem.h"
#include "models/SystemState.h"
#include "constants/SystemConstants.h"

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
