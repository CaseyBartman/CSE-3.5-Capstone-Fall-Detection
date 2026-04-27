#pragma once
#include "interfaces/INurseInput.h"

class SoftwareButton : public INurseInput {
public:
    SoftwareButton();
    void init() override;
    bool isPressed() override;
    bool wasShortPressed() override;
    bool wasLongPressed() override;
    void update() override;

private:
    bool pressed;
    bool shortPressed;
    bool longPressed;
    unsigned long pressStartTime;
};