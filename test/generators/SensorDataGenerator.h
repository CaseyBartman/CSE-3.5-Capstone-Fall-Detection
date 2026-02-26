#pragma once

#include "models/SensorPayload.h"
#include <vector>

/**
 * Sensor Data Generator for Testing
 * Generates realistic sensor data patterns for testing
 */
class SensorDataGenerator {
public:
    /**
     * Generate a sequence showing gradual pressure increase (patient sitting down)
     */
    static std::vector<float> generateSittingDownSequence() {
        return {5.0f, 10.0f, 20.0f, 30.0f, 40.0f, 45.0f};
    }

    /**
     * Generate a sequence showing rapid pressure decrease (patient standing up/falling)
     */
    static std::vector<float> generateStandingUpSequence() {
        return {45.0f, 40.0f, 30.0f, 15.0f, 5.0f, 2.0f};
    }

    /**
     * Generate stable occupied pressure readings
     */
    static std::vector<float> generateStableOccupiedSequence(int count = 10) {
        std::vector<float> sequence;
        for (int i = 0; i < count; i++) {
            sequence.push_back(40.0f + (i % 3) - 1.0f); // Small variations around 40%
        }
        return sequence;
    }

    /**
     * Generate stable unoccupied pressure readings
     */
    static std::vector<float> generateStableUnoccupiedSequence(int count = 10) {
        std::vector<float> sequence;
        for (int i = 0; i < count; i++) {
            sequence.push_back(2.0f + (i % 2) * 0.5f); // Small variations around 2%
        }
        return sequence;
    }

    /**
     * Generate a complete fall detection scenario
     * Patient sits -> stable -> stands up -> unoccupied
     */
    static std::vector<float> generateFallScenario() {
        std::vector<float> scenario;
        
        // Sitting down
        auto sitting = generateSittingDownSequence();
        scenario.insert(scenario.end(), sitting.begin(), sitting.end());
        
        // Stable occupied
        auto stable = generateStableOccupiedSequence(20);
        scenario.insert(scenario.end(), stable.begin(), stable.end());
        
        // Standing up (fall detection)
        auto standing = generateStandingUpSequence();
        scenario.insert(scenario.end(), standing.begin(), standing.end());
        
        // Unoccupied
        auto unoccupied = generateStableUnoccupiedSequence(10);
        scenario.insert(scenario.end(), unoccupied.begin(), unoccupied.end());
        
        return scenario;
    }

    /**
     * Create a SensorPayload from pressure reading
     */
    static SensorPayload createPayload(float pressure, unsigned long timestamp = 0) {
        bool occupied = pressure > 15.0f; // Default threshold
        return SensorPayload(pressure, occupied, timestamp);
    }
};
