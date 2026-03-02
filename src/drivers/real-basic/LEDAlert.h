#pragma once
#include "interfaces/IAlertSystem.h"
#include <Arduino.h>

class LEDAlert : public IAlertSystem {
private:
    int _ledPin;
    bool _alarmActive;

public:
    LEDAlert(int ledPin);

    void init() override;
    void triggerFallAlarm() override;
    void clearAlarm() override;
};