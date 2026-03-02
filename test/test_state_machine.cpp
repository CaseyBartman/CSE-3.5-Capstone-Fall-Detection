#include <gtest/gtest.h>
#include <iostream>
#include "logic/FallDetector.h"
#include "test/helpers/TestFixture.h"
#include "test/helpers/MockSensors.h"
#include "test/helpers/TimeSimulator.h"

/**
 * Unit Tests for Fall Detection State Machine
 * 
 * Tests the correct behavior of the FallDetector using mock implementations
 * of the hardware interfaces (sensors, buttons, alerts).
 * 
 * Test Categories:
 * 1. System Initialization
 * 2. State Transitions
 * 3. Fall Detection Logic
 * 4. Button Handling
 * 5. Timing and State Expiration
 */

// ============================================================================
// TEST FIXTURE SETUP - Common initialization for all tests
// ============================================================================

class FallDetectorTest : public ::testing::Test {
protected:
    TestFixture* fixture;

    void SetUp() override {
        fixture = new TestFixture();
        fixture->init();
    }

    void TearDown() override {
        if (fixture != nullptr) {
            delete fixture;
            fixture = nullptr;
        }
    }
};

// ============================================================================
// TEST SUITE 1: SYSTEM INITIALIZATION
// ============================================================================

TEST_F(FallDetectorTest, InitializationCreatesAllComponents) {
    // Verify all components are initialized
    ASSERT_NE(fixture->getSensor(), nullptr);
    ASSERT_NE(fixture->getButton(), nullptr);
    ASSERT_NE(fixture->getAlert(), nullptr);
    ASSERT_NE(fixture->getDetector(), nullptr);
}

TEST_F(FallDetectorTest, SystemStartsInIdleState) {
    // After initialization, system should be in IDLE state
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::IDLE);
}

TEST_F(FallDetectorTest, AllMocksAreInitialized) {
    // Verify all mock components report initialized state
    EXPECT_TRUE(fixture->getSensor()->isInitialized());
    EXPECT_TRUE(fixture->getButton()->isInitialized());
    EXPECT_TRUE(fixture->getAlert()->isInitialized());
}

// ============================================================================
// TEST SUITE 2: STATE TRANSITIONS
// ============================================================================

TEST_F(FallDetectorTest, IdleToPollingTransition) {
    // Precondition: System is in IDLE state
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::IDLE);
    
    // System should transition to POLLING on update when ready
    fixture->transitionToPolling();
    
    // Verify state change
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

TEST_F(FallDetectorTest, PollingToAlarmOnFallDetection) {
    // Setup: Get to POLLING state
    fixture->transitionToPolling();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    
    // Simulate fall detection
    fixture->simulateStandUpEvent();
    
    // Verify transition to ALARM state
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::ALARM);
}

TEST_F(FallDetectorTest, AlarmToPolling_OnButtonPress) {
    // Setup: Get to ALARM state
    fixture->transitionToPolling();
    fixture->simulateStandUpEvent();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::ALARM);
    
    // Simulate nurse pressing button to clear alarm
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    
    // Verify return to POLLING state
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

TEST_F(FallDetectorTest, PollingToInputPaused_OnShortPress) {
    // Setup: Get to POLLING state
    fixture->transitionToPolling();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    
    // Simulate short button press
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    
    // Verify transition to INPUT_PAUSED state
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
}

TEST_F(FallDetectorTest, PollingToCalibration_OnLongPress) {
    // Setup: Get to POLLING state
    fixture->transitionToPolling();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    
    // Simulate long button press
    fixture->getButton()->simulateLongPress();
    fixture->getDetector()->update();
    
    // Verify transition to CALIBRATION state
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::CALIBRATION);
}

TEST_F(FallDetectorTest, InputPausedToPolling_OnTimeout) {
    // Setup: Get to INPUT_PAUSED state
    fixture->transitionToPolling();
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
    
    // Simulate pause duration expiring
    fixture->advanceTimeAndWaitForPauseExpiry();
    
    // Verify return to POLLING state
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

TEST_F(FallDetectorTest, CalibrationToPolling_OnTimeout) {
    // Setup: Get to CALIBRATION state
    fixture->transitionToPolling();
    fixture->getButton()->simulateLongPress();
    fixture->getDetector()->update();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::CALIBRATION);
    
    // Simulate calibration duration expiring
    fixture->advanceTimeAndWaitForCalibrationExpiry();
    
    // Verify return to POLLING state
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

// ============================================================================
// TEST SUITE 3: FALL DETECTION LOGIC
// ============================================================================

TEST_F(FallDetectorTest, FallDetectedWhenPressureDropsBelowThreshold) {
    // Setup: Get to POLLING state with patient sitting
    fixture->transitionToPolling();
    fixture->setPatientsitting(40.0f);
    fixture->getDetector()->update();
    
    // Verify system registered occupancy
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    
    // Patient stands up (pressure drops)
    fixture->setPatientStandingUp(100.0f);
    fixture->getDetector()->update();
    
    // Verify alarm triggered
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::ALARM);
    EXPECT_GT(fixture->getAlert()->getAlarmTriggeredCount(), 0);
}

TEST_F(FallDetectorTest, NoFalseAlarmOnGradualPressureChange) {
    // Setup: Get to POLLING state
    fixture->transitionToPolling();
    
    // Simulate gradual pressure change (no alarm should trigger)
    fixture->getSensor()->setMockPressure(35.0f);
    fixture->getDetector()->update();
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    
    fixture->getSensor()->setMockPressure(25.0f);
    fixture->getDetector()->update();
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    
    fixture->getSensor()->setMockPressure(18.0f);
    fixture->getDetector()->update();
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

TEST_F(FallDetectorTest, AlarmTriggerCountIncrementsCorrectly) {
    // Setup: Get to POLLING state
    fixture->transitionToPolling();
    ASSERT_EQ(fixture->getAlert()->getAlarmTriggeredCount(), 0);
    
    // Trigger first alarm
    fixture->simulateStandUpEvent();
    EXPECT_EQ(fixture->getAlert()->getAlarmTriggeredCount(), 1);
    
    // Clear alarm
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    EXPECT_EQ(fixture->getAlert()->getAlarmClearedCount(), 1);
}

TEST_F(FallDetectorTest, AlarmClearedOnButtonPress) {
    // Setup: Get to ALARM state
    fixture->transitionToPolling();
    fixture->simulateStandUpEvent();
    ASSERT_EQ(fixture->getAlert()->getAlarmTriggeredCount(), 1);
    
    // Button press should clear alarm
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    
    // Verify alarm was cleared
    EXPECT_EQ(fixture->getAlert()->getAlarmClearedCount(), 1);
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

// ============================================================================
// TEST SUITE 4: BUTTON HANDLING
// ============================================================================

TEST_F(FallDetectorTest, ShortPressClearsAlarm) {
    // Setup: System in ALARM state
    fixture->transitionToPolling();
    fixture->simulateStandUpEvent();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::ALARM);
    
    // Short button press
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    
    // Should return to POLLING
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

TEST_F(FallDetectorTest, ShortPressPausesMonitoring) {
    // Setup: System in POLLING state
    fixture->transitionToPolling();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    
    // Short button press while monitoring
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    
    // Should transition to INPUT_PAUSED
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
}

TEST_F(FallDetectorTest, LongPressTriggerCalibration) {
    // Setup: System in POLLING state
    fixture->transitionToPolling();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    
    // Long button press
    fixture->getButton()->simulateLongPress();
    fixture->getDetector()->update();
    
    // Should transition to CALIBRATION
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::CALIBRATION);
}

TEST_F(FallDetectorTest, MultipleButtonPressesHandleSequentially) {
    // Setup: System in POLLING state
    fixture->transitionToPolling();
    
    // Simulate sequence: short press, wait, long press
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
    
    // Wait for pause to expire
    fixture->advanceTimeAndWaitForPauseExpiry();
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    
    // Long press
    fixture->getButton()->simulateLongPress();
    fixture->getDetector()->update();
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::CALIBRATION);
}

// ============================================================================
// TEST SUITE 5: TIMING AND STATE EXPIRATION
// ============================================================================

TEST_F(FallDetectorTest, PauseDurationExpires_AtExactTime) {
    // Setup: System in INPUT_PAUSED state
    fixture->transitionToPolling();
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
    
    // Advance time to exact expiration point
    fixture->getTime()->advanceMs(PAUSE_DURATION_MS);
    fixture->getDetector()->update();
    
    // Should transition back to POLLING
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

TEST_F(FallDetectorTest, CalibrationDurationExpires_AtExactTime) {
    // Setup: System in CALIBRATION state
    fixture->transitionToPolling();
    fixture->getButton()->simulateLongPress();
    fixture->getDetector()->update();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::CALIBRATION);
    
    // Advance time to exact expiration point
    fixture->getTime()->advanceMs(CALIB_DURATION_MS);
    fixture->getDetector()->update();
    
    // Should transition back to POLLING
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

TEST_F(FallDetectorTest, PauseDoesNotExpireEarly) {
    // Setup: System in INPUT_PAUSED state
    fixture->transitionToPolling();
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
    
    // Advance time to 1 second before expiration
    fixture->getTime()->advanceMs(PAUSE_DURATION_MS - 1000);
    fixture->getDetector()->update();
    
    // Should still be paused
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
}

TEST_F(FallDetectorTest, CalibrationDoesNotExpireEarly) {
    // Setup: System in CALIBRATION state
    fixture->transitionToPolling();
    fixture->getButton()->simulateLongPress();
    fixture->getDetector()->update();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::CALIBRATION);
    
    // Advance time to 1 second before expiration
    fixture->getTime()->advanceMs(CALIB_DURATION_MS - 1000);
    fixture->getDetector()->update();
    
    // Should still be in calibration
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::CALIBRATION);
}

// ============================================================================
// TEST SUITE 6: EDGE CASES AND INTEGRATION
// ============================================================================

TEST_F(FallDetectorTest, StateTransitionSequence) {
    // Test complete control flow: IDLE -> POLLING -> ALARM -> POLLING -> INPUT_PAUSED -> POLLING
    
    // 1. IDLE -> POLLING
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::IDLE);
    fixture->transitionToPolling();
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    
    // 2. POLLING -> ALARM
    fixture->simulateStandUpEvent();
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::ALARM);
    
    // 3. ALARM -> POLLING
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    
    // 4. POLLING -> INPUT_PAUSED
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
    
    // 5. INPUT_PAUSED -> POLLING (timeout)
    fixture->advanceTimeAndWaitForPauseExpiry();
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

TEST_F(FallDetectorTest, ResetBetweenTests) {
    // Verify TimeSimulator is reset between tests
    EXPECT_EQ(fixture->getTime()->getCurrentTime(), 0);
}

TEST_F(FallDetectorTest, AlertCountersResetOnNewDetector) {
    // Create a fresh detector and verify clean state
    auto newAlert = fixture->getAlert();
    newAlert->resetCounters();
    
    EXPECT_EQ(newAlert->getAlarmTriggeredCount(), 0);
    EXPECT_EQ(newAlert->getAlarmClearedCount(), 0);
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    return result;
}
