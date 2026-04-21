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

#include "interfaces/INurseInput.h"
#include "constants/SystemConstants.h"
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