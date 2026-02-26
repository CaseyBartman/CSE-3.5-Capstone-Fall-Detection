#pragma once

#include "interfaces/IForceSensor.h"
#include "interfaces/INurseInput.h"
#include "interfaces/IAlertSystem.h"

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
