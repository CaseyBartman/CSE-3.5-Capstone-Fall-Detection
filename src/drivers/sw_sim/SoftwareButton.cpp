#include "drivers/sw_sim/SoftwareButton.h"
#include <Arduino.h>
#include "constants/SystemConstants.h"

SoftwareButton::SoftwareButton() : pressed(false), shortPressed(false), longPressed(false), pressStartTime(0) {}

void SoftwareButton::init() {
    Serial.println("SoftwareButton initialized. Send 'p' to press, 'r' to release.");
}

void SoftwareButton::update() {
    if (Serial.available() > 0) {
        char command = Serial.read();
        if (command == 'p') {
            if (!pressed) {
                pressed = true;
                pressStartTime = millis();
                Serial.println("Button pressed");
            }
        } else if (command == 'r') {
            if (pressed) {
                pressed = false;
                unsigned long pressDuration = millis() - pressStartTime;
                if (pressDuration >= LONG_PRESS_MS) {
                    longPressed = true;
                    Serial.println("Long press detected");
                } else if (pressDuration >= SHORT_PRESS_MS) {
                    shortPressed = true;
                    Serial.println("Short press detected");
                }
                pressStartTime = 0;
            }
        }
    }
}

bool SoftwareButton::isPressed() {
    return pressed;
}

bool SoftwareButton::wasShortPressed() {
    if (shortPressed) {
        shortPressed = false;
        return true;
    }
    return false;
}

bool SoftwareButton::wasLongPressed() {
    if (longPressed) {
        longPressed = false;
        return true;
    }
    return false;
}
