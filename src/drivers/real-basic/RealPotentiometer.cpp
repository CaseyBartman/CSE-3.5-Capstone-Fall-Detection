#include <Arduino.h>
#include "RealPotentiometer.h"

RealPotentiometer::RealPotentiometer(int pin, float threshold) 
    : _pin(pin), _threshold(threshold) {
}

void RealPotentiometer::init() {
    pinMode(_pin, INPUT);
    Serial.print("RealPotentiometerSensor initialized on pin ");
    Serial.println(_pin);
}

float RealPotentiometer::getPressurePercentage() {
    // Read raw ADC value (0-4095)
    int rawValue = analogRead(_pin);
    
    // Convert ADC value to voltage (0-3.3V)
    float voltage = (rawValue / MAX_ADC_VALUE) * MAX_VOLTAGE;
    
    // Convert voltage to pressure percentage (0-100%)
    float pressurePercentage = (voltage / MAX_VOLTAGE) * 100.0f;
    
    return pressurePercentage;
}

bool RealPotentiometer::isOccupied() {
    return getPressurePercentage() > _threshold;
}