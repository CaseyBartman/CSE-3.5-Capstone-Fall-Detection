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
    // Read raw ADC value from Tekscan A502 (0-1023)
    int sensorValue = analogRead(_pin);
    
    // Convert ADC value to voltage (0-5V)
    float voltage = sensorValue * (MAX_VOLTAGE / MAX_ADC_VALUE);
    
    // Map sensor to 0-100% using FIXED maximum (based on actual sensor range)
    // Tekscan outputs ~0-11 raw with heavy pressure, so use 20 as the 100% reference
    float pressurePercentage = (float)sensorValue / (float)SENSOR_MAX_RAW * 100.0f;
    pressurePercentage = max(0.0f, min(100.0f, pressurePercentage)); // Clamp 0-100
    
    return pressurePercentage;
}

bool RealTekscan::isOccupied() {
    return getPressurePercentage() > _threshold;
}
