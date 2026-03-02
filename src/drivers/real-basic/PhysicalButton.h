#pragma once
#include "interfaces/INurseInput.h"
#include "constants/SystemConstants.h"
#include <Arduino.h>

class PhysicalButton : public INurseInput {
private:
    int _pin;
    bool _lastState;
    unsigned long _pressedTime;
    bool _shortPressFlag;
    bool _longPressFlag;

public:
    PhysicalButton(int pin);

    void init() override;
    void update() override;

    bool isPressed() override;
    bool wasShortPressed() override;
    bool wasLongPressed() override;
};