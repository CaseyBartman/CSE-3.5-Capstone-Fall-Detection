#include <Arduino.h>
#include "RealTekscan.h"

RealTekscan::RealTekscan(int pin, float threshold) 
    : _pin(pin), _threshold(threshold) {
}

void RealTekscan::init() {
    pinMode(_pin, INPUT);
    Serial.println("[TEKSCAN] Initializing Tekscan A502 sensor...");
}

float RealTekscan::getPressurePercentage() {
    // Read raw ADC value from Tekscan A502 (0-1023 with 10-bit ADC)
    int sensorValue = analogRead(_pin);
    
    // Map full 10-bit ADC range (0-1023) to 0-100%
    // The A502 output voltage scales linearly with applied force;
    // at max force the output reaches the supply voltage, saturating the ADC at 1023.
    float pressurePercentage = (float)sensorValue / MAX_ADC_VALUE * 100.0f;
    pressurePercentage = max(0.0f, min(100.0f, pressurePercentage)); // Clamp 0-100
    
    return pressurePercentage;
}

bool RealTekscan::isOccupied() {
    return getPressurePercentage() > _threshold;
}
