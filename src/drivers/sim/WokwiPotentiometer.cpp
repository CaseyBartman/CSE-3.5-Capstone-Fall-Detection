#include "interfaces/IForceSensor.h"
#include <Arduino.h>

class WokwiPotentiometer : public IForceSensor {
private:
    const int _pin;
    const float _threshold;

public:
    WokwiPotentiometer(int pin, float threshold) : _pin(pin), _threshold(threshold) {}

    void init() override {
        pinMode(_pin, INPUT);
        Serial.print("WokwiPotentiometer initialized on pin ");
        Serial.println(_pin);
    }

    float getPressurePercentage() override {
        int raw = analogRead(_pin);
        // Map 12-bit ADC (0-4095) to 0-100%
        return (float)raw / 40.95f; 
    }

    bool isOccupied() override {
        return getPressurePercentage() > _threshold;
    }
};
