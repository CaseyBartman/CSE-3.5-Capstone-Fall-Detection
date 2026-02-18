#include "interfaces/INurseInput.h"
#include "constants/SystemConstants.h"
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
