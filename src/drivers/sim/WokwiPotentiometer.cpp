/**
 * @file WokwiPotentiometer.cpp
 * @brief Simulated pressure sensor for Wokwi Arduino simulation
 * 
 * Maps an analog potentiometer input (0-4095 ADC range) to a pressure
 * percentage (0-100%) to simulate a real force/pressure sensor in the
 * Wokwi simulator environment.
 * 
 * Pin: GPIO 34 (ADC input)
 * ADC Range: 0-4095 (12-bit)
 * Output: 0.0-100.0% pressure
 */

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
