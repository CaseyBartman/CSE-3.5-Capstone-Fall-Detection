#include <Arduino.h>
#include "PhysicalButton.h"

PhysicalButton::PhysicalButton(int pin)
    : _pin(pin), _lastState(HIGH), _pressedTime(0),
      _shortPressFlag(false), _longPressFlag(false) {}

void PhysicalButton::init() {
    pinMode(_pin, INPUT_PULLUP);
    Serial.print("PhysicalButton initialized on pin ");
    Serial.println(_pin);
}

void PhysicalButton::update() {
    bool currentState = digitalRead(_pin);

    // Reset flags every loop
    _shortPressFlag = false;
    _longPressFlag = false;

    if (_lastState == HIGH && currentState == LOW) {
        _pressedTime = millis();
    }
    else if (_lastState == LOW && currentState == HIGH) {
        unsigned long duration = millis() - _pressedTime;
        if (duration >= LONG_PRESS_MS) _longPressFlag = true;
        else if (duration >= DEBOUNCE_DELAY_MS) _shortPressFlag = true;
    }

    _lastState = currentState;
}

bool PhysicalButton::isPressed() { return digitalRead(_pin) == LOW; }
bool PhysicalButton::wasShortPressed() { return _shortPressFlag; }
bool PhysicalButton::wasLongPressed() { return _longPressFlag; }