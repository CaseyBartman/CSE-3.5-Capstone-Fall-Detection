#pragma once
#include "interfaces/IForceSensor.h"
#include "constants/SystemConstants.h"
#include <Arduino.h>

class RealTekscan : public IForceSensor {
private:
    int _pin;
    float _threshold;
    const float MAX_ADC_VALUE = 1023.0f;

public:
    RealTekscan(int pin, float threshold = DEFAULT_PRESSURE_THRESHOLD);

    void init() override;
    float getPressurePercentage() override;
    bool isOccupied() override;
};
