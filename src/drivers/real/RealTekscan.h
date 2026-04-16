#pragma once
#include "interfaces/IForceSensor.h"
#include "constants/SystemConstants.h"
#include <Arduino.h>

class RealTekscan : public IForceSensor {
private:
    int _pin;
    float _threshold;
    const float MAX_ADC_VALUE = 1023.0f;  // Arduino Uno R4 ADC resolution (10-bit)
    const float MAX_VOLTAGE = 5.0f;        // Arduino Uno R4 max voltage
    const int SENSOR_MAX_RAW = 20;         // Tekscan A502 maximum expected raw value (~11-20 with heavy pressure)

public:
    RealTekscan(int pin, float threshold = DEFAULT_PRESSURE_THRESHOLD);

    void init() override;
    float getPressurePercentage() override;
    bool isOccupied() override;
};
