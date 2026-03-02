#pragma once

#include <cstddef>
#include "test/helpers/MockSensors.h"
#include "test/helpers/TimeSimulator.h"
#include "logic/FallDetector.h"

/**
 * TestFixture Base Class
 * 
 * Provides common setup and teardown for all FallDetector tests.
 * Ensures consistent initialization and cleanup between test runs.
 * 
 * Usage in test files:
 * 
 *   // Global variables (declared once at file level)
 *   static TestFixture* fixture;
 *   
 *   void setUp(void) {
 *       fixture = new TestFixture();
 *       fixture->init();
 *   }
 *   
 *   void tearDown(void) {
 *       fixture->cleanup();
 *       delete fixture;
 *   }
 *   
 *   void test_example() {
 *       fixture->getSensor()->setMockPressure(40.0f);
 *       fixture->getDetector()->update();
 *       // Make assertions...
 *   }
 */
class TestFixture {
private:
    MockForceSensor* _sensor;
    MockNurseInput* _button;
    MockAlertSystem* _alert;
    FallDetector* _detector;
    TimeSimulator* _time;

public:
    TestFixture() 
        : _sensor(nullptr), _button(nullptr), _alert(nullptr), 
          _detector(nullptr), _time(nullptr) {}

    /**
     * Initialize all mocks and the system under test
     * Call this in each test's setUp() function
     */
    void init() {
        // Create mocks
        _sensor = new MockForceSensor(DEFAULT_PRESSURE_THRESHOLD);
        _button = new MockNurseInput();
        _alert = new MockAlertSystem();
        
        // Initialize time simulator
        _time = TimeSimulator::getInstance();
        _time->reset();
        
        // Create and initialize system under test
        _detector = new FallDetector(_sensor, _button, _alert);
        _detector->init();
    }

    /**
     * Clean up all allocated resources
     * Call this in each test's tearDown() function
     */
    void cleanup() {
        if (_detector != nullptr) {
            delete _detector;
            _detector = nullptr;
        }
        if (_alert != nullptr) {
            delete _alert;
            _alert = nullptr;
        }
        if (_button != nullptr) {
            delete _button;
            _button = nullptr;
        }
        if (_sensor != nullptr) {
            delete _sensor;
            _sensor = nullptr;
        }
        
        // Reset time simulator
        if (_time != nullptr) {
            _time->reset();
        }
    }

    /**
     * Destructor - ensure cleanup is called
     */
    ~TestFixture() {
        cleanup();
    }

    // ============= Accessors for mocks and system under test =============

    MockForceSensor* getSensor() const {
        return _sensor;
    }

    MockNurseInput* getButton() const {
        return _button;
    }

    MockAlertSystem* getAlert() const {
        return _alert;
    }

    FallDetector* getDetector() const {
        return _detector;
    }

    TimeSimulator* getTime() const {
        return _time;
    }

    // ============= Convenience helper methods =============

    /**
     * Verify system is in IDLE state after init
     */
    bool isInIdleState() const {
        return _detector->getCurrentState() == SystemState::IDLE;
    }

    /**
     * Verify system is in POLLING state
     */
    bool isInPollingState() const {
        return _detector->getCurrentState() == SystemState::POLLING;
    }

    /**
     * Verify system is in ALARM state
     */
    bool isInAlarmState() const {
        return _detector->getCurrentState() == SystemState::ALARM;
    }

    /**
     * Verify system is in INPUT_PAUSED state
     */
    bool isInPauseState() const {
        return _detector->getCurrentState() == SystemState::INPUT_PAUSED;
    }

    /**
     * Verify system is in CALIBRATION state
     */
    bool isInCalibrationState() const {
        return _detector->getCurrentState() == SystemState::CALIBRATION;
    }

    /**
     * Transition system from IDLE to POLLING
     * Simulates system readiness check
     */
    void transitionToPolling() {
        _detector->update();  // IDLE -> POLLING
    }

    /**
     * Simulate patient sitting on mat
     * Sets sensor pressure to "occupied" level
     */
    void setPatientsitting(float pressure = 40.0f) {
        _sensor->setMockPressure(pressure);
    }

    /**
     * Simulate patient standing up from mat
     * Sets sensor pressure to "unoccupied" level (triggers fall detection)
     */
    void setPatientStandingUp(float pressure = 0.0f) {
        _sensor->setMockPressure(pressure);
    }

    /**
     * Simulate mat being empty
     * Sets sensor pressure to zero/minimal
     */
    void setMatEmpty(float pressure = 0.0f) {
        _sensor->setMockPressure(pressure);
    }

    /**
     * Helper: Simulate patient interaction and fall detection
     * Sequence: Sitting -> Standing -> Alarm triggered
     */
    void simulateStandUpEvent() {
        setPatientsitting(40.0f);
        _detector->update();  // Detect occupancy
        
        setPatientStandingUp(3.0f);
        _detector->update();  // Trigger alarm
    }

    /**
     * Helper: Simulate pause duration expiration
     */
    void advanceTimeAndWaitForPauseExpiry() {
        _time->advanceMs(PAUSE_DURATION_MS);
        _detector->update();
    }

    /**
     * Helper: Simulate calibration duration expiration
     */
    void advanceTimeAndWaitForCalibrationExpiry() {
        _time->advanceMs(CALIB_DURATION_MS);
        _detector->update();
    }

    /**
     * Get current state as readable string (for debugging)
     */
    const char* getStateString() const {
        switch (_detector->getCurrentState()) {
            case SystemState::SYSTEM_OFF: return "SYSTEM_OFF";
            case SystemState::IDLE: return "IDLE";
            case SystemState::POLLING: return "POLLING";
            case SystemState::INPUT_PAUSED: return "INPUT_PAUSED";
            case SystemState::CALIBRATION: return "CALIBRATION";
            case SystemState::ALARM: return "ALARM";
            default: return "UNKNOWN";
        }
    }
};

