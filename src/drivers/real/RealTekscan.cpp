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
    int sensorValue = analogRead(_pin);
    float pressurePercentage = (float)sensorValue / MAX_ADC_VALUE * MAX_PRESSURE_PERCENTAGE;
    return max(MIN_PRESSURE_PERCENTAGE, min(MAX_PRESSURE_PERCENTAGE, pressurePercentage));
}

bool RealTekscan::isOccupied() {
    return getPressurePercentage() > _threshold;
}
