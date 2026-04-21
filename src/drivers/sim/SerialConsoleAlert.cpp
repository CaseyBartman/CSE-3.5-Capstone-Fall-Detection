/**
 * @file SerialConsoleAlert.cpp
 * @brief Simulated alert system for Wokwi Arduino simulation
 * 
 * Logs fall detection alerts to the Serial monitor instead of sending
 * to a real alert system (Ntfy, Connexxall, etc.). Used exclusively in
 * simulation mode for testing the state machine logic.
 * 
 * Output: Serial monitor console messages with visual separators
 * Baud: 115200
 */

#include "interfaces/IAlertSystem.h"
#include <Arduino.h>

class SerialConsoleAlert : public IAlertSystem {
private:
    bool _alarmActive;

public:
    SerialConsoleAlert() : _alarmActive(false) {}

    void init() override {
        Serial.println("SerialConsoleAlert initialized");
    }

    void triggerFallAlarm() override {
        _alarmActive = true;
        Serial.println("========================================");
        Serial.println("!!!!! FALL ALARM TRIGGERED !!!!!");
        Serial.println("Alert sent to Ntfy (Simulated)");
        Serial.println("========================================");
    }

    void clearAlarm() override {
        if (_alarmActive) {
            _alarmActive = false;
            Serial.println("========================================");
            Serial.println("Alarm cleared by nurse");
            Serial.println("Ntfy alert cleared (Simulated)");
            Serial.println("========================================");
        }
    }
};
