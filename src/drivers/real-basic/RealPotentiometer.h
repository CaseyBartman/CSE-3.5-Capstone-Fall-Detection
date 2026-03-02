#pragma once
#include "interfaces/IForceSensor.h"
#include "constants/SystemConstants.h"
#include <Arduino.h>

class RealPotentiometer : public IForceSensor {
private:
    int _pin;
    float _threshold;
    const float MAX_ADC_VALUE = 4095.0f;  // ESP32 ADC resolution
    const float MAX_VOLTAGE = 3.3f;        // ESP32 max voltage

public:
    RealPotentiometer(int pin, float threshold = DEFAULT_PRESSURE_THRESHOLD);

    void init() override;
    float getPressurePercentage() override;
    bool isOccupied() override;
};