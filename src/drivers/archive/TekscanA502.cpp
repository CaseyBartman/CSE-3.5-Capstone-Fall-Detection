#include "interfaces/IForceSensor.h"
#include <Arduino.h>

class TekscanA502 : public IForceSensor {
private:
    const int _pin;
    const float _threshold;
    float _zeroOffset;

public:
    TekscanA502(int pin, float threshold = 15.0f) 
        : _pin(pin), _threshold(threshold), _zeroOffset(0.0f) {}

    void init() override {
        pinMode(_pin, INPUT);
        Serial.println("TekscanA502 force sensor initialized");
        // TODO: Implement actual Tekscan A502 initialization
        // This may include I2C/SPI setup or specific calibration routines
    }

    float getPressurePercentage() override {
        // TODO: Implement actual Tekscan A502 reading logic
        // This is a placeholder that reads from ADC
        int raw = analogRead(_pin);
        float pressure = (float)raw / 40.95f;
        return pressure - _zeroOffset;
    }

    bool isOccupied() override {
        return getPressurePercentage() > _threshold;
    }

    void calibrateZero() {
        // Read current pressure and set as zero offset
        int raw = analogRead(_pin);
        _zeroOffset = (float)raw / 40.95f;
        Serial.print("Zero offset calibrated to: ");
        Serial.println(_zeroOffset);
    }
};
