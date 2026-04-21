#include <Arduino.h>
#include "LEDAlert.h"

LEDAlert::LEDAlert(int ledPin) : _ledPin(ledPin), _alarmActive(false) {}

void LEDAlert::init() {
    pinMode(_ledPin, OUTPUT);
    digitalWrite(_ledPin, LOW);
    Serial.print("LEDAlert initialized on pin ");
    Serial.println(_ledPin);
}

void LEDAlert::triggerFallAlarm() {
    if (!_alarmActive) {
        _alarmActive = true;
        digitalWrite(_ledPin, HIGH);
        Serial.println("LED Alarm ON");
    }
}

void LEDAlert::clearAlarm() {
    if (_alarmActive) {
        _alarmActive = false;
        digitalWrite(_ledPin, LOW);
        Serial.println("LED Alarm OFF");
    }
}