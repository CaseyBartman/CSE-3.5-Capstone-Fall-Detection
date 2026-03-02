#pragma once

struct SensorPayload {
    float pressurePercentage;
    bool isOccupied;
    unsigned long timestamp;
    
    SensorPayload() : pressurePercentage(0.0f), isOccupied(false), timestamp(0) {}
    
    SensorPayload(float pressure, bool occupied, unsigned long time)
        : pressurePercentage(pressure), isOccupied(occupied), timestamp(time) {}
};
