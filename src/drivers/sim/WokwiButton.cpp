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
        _shortPressFlag = false;
        _longPressFlag = false;
        
        _currentState = digitalRead(_pin);
        
        bool buttonWasJustPressed = _currentState == LOW && _lastState == HIGH;
        if (buttonWasJustPressed) {
            _pressStartTime = millis();
            _longPressTriggered = false;
        }
        
        bool isButtonBeingHeld = _currentState == LOW && _lastState == LOW;
        if (isButtonBeingHeld) {
            unsigned long pressDuration = millis() - _pressStartTime;
            
            bool wasButtonPressLong = pressDuration >= LONG_PRESS_MS && !_longPressTriggered;
            if (wasButtonPressLong) {
                _longPressFlag = true;
                _longPressTriggered = true;
                Serial.println("Long press detected");
            }
        }
        
        bool wasButtonReleased = _currentState == HIGH && _lastState == LOW;
        if (wasButtonReleased) {
            unsigned long pressDuration = millis() - _pressStartTime;
            
            bool wasButtonPressShort = pressDuration < LONG_PRESS_MS;
            if (wasButtonPressShort) {
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
