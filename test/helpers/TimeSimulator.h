#pragma once

/**
 * Time Simulator for Deterministic Testing
 * 
 * This class provides a singleton that allows tests to control system time
 * without actually waiting for real milliseconds. This enables:
 * - Fast test execution (2 minute pause becomes 2ms in test)
 * - Deterministic test results (no timing variability)
 * - Comprehensive edge case testing (test exact millisecond boundaries)
 * 
 * Usage in tests:
 * 
 *   TimeSimulator* time = TimeSimulator::getInstance();
 *   time->reset();
 *   
 *   // Simulate time passing
 *   time->advanceMs(100);
 *   assert(time->getCurrentTime() == 100);
 */
class TimeSimulator {
private:
    static TimeSimulator* _instance;
    unsigned long _currentTimeMs;

    // Private constructor for singleton
    TimeSimulator() : _currentTimeMs(0) {}

public:
    // Get singleton instance
    static TimeSimulator* getInstance();

    /**
     * Advance simulated time by specified milliseconds
     * @param milliseconds The amount of time to advance
     */
    void advanceMs(unsigned long milliseconds) {
        _currentTimeMs += milliseconds;
    }

    /**
     * Get current simulated time in milliseconds
     * @return Current simulated time (unsigned long)
     */
    unsigned long getCurrentTime() const {
        return _currentTimeMs;
    }

    /**
     * Reset time to zero
     * Called in setUp() to reset state between tests
     */
    void reset() {
        _currentTimeMs = 0;
    }

    /**
     * Set time to specific value
     * Useful for testing specific time points
     */
    void setTime(unsigned long timeMs) {
        _currentTimeMs = timeMs;
    }

    /**
     * Destructor
     */
    ~TimeSimulator() {}

    // Delete copy constructor and assignment operator (singleton pattern)
    TimeSimulator(const TimeSimulator&) = delete;
    TimeSimulator& operator=(const TimeSimulator&) = delete;
};
