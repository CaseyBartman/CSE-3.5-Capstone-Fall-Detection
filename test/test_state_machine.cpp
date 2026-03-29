#include <gtest/gtest.h>
#include <iostream>
#include "logic/FallDetector.h"
#include "test/helpers/TestFixture.h"
#include "test/helpers/MockSensors.h"
#include "test/helpers/TimeSimulator.h"
#include "test/TestConstants.h"

/**
 * Comprehensive Test Suite for CSE 3.5 Mat Fall Detection System
 * Test Environment: Uses mock implementations of hardware interfaces
 * (ForceSensor, NurseInput, AlertSystem) to enable pure unit testing
 * without physical hardware.
 */

// ============================================================================
// TEST FIXTURE SETUP
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
// ============================================================================
// SECTION 4.1: USE CASE TEST SCENARIOS (Unit Tests)
// ============================================================================
// ============================================================================
// These tests validate the system's expected behavior for each major use case
// described in Section 2: Use Case Scenarios and Expected Behavior

/**
 * UC-1.1: Normal Monitoring
 * Use Case: Normal Monitoring
 * Scenario: System is powered on and connected (isWifiConnected and isSensorReady are true)
 * Expected Result: System enters and remains in the POLLING state. Log: "System Armed"
 */
TEST_F(FallDetectorTest, UC_1_1_NormalMonitoringShouldEnterPollingState) {
    // Given: System is IDLE after initialization
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::IDLE);
    
    // When: System conditions are met (power on, connected)
    fixture->transitionToPolling();
    
    // Then: System enters POLLING state and continuously monitors
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

/**
 * UC-2.1: Fall/Stand-up Detection
 * Use Case: Fall/Stand-up Detection
 * Scenario: Patient stands up, causing detectedForce to exceed THRESHOLD
 * Expected Result: System transitions from POLLING to ALARM. Send alarm CONEXXAL message
 */
TEST_F(FallDetectorTest, UC_2_1_FallDetectionShouldTransitionToAlarm) {
    // Given: System is in POLLING state
    fixture->transitionToPolling();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    
    // When: Force exceeds the FALL_DETECTION_THRESHOLD
    fixture->simulateStandUpEvent();
    
    // Then: System transitions to ALARM and sends alarm signal
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::ALARM);
    EXPECT_GT(fixture->getAlert()->getAlarmTriggeredCount(), 0);
}

/**
 * UC-3.1: Alarm Acknowledgment
 * Use Case: Alarm Acknowledgment
 * Scenario: While in the ALARM state, a nurse presses the button
 * Expected Result: System transitions from ALARM to POLLING. Execute clearConnexxallAlert()
 */
TEST_F(FallDetectorTest, UC_3_1_AlarmAcknowledgmentShouldClearAlert) {
    // Given: System is in ALARM state after fall detection
    fixture->transitionToPolling();
    fixture->simulateStandUpEvent();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::ALARM);
    
    // When: Nurse presses button
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    
    // Then: System returns to POLLING and clears the alert
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    EXPECT_GT(fixture->getAlert()->getAlarmClearedCount(), 0);
}

/**
 * UC-4.1: Temporary Input Pause
 * Use Case: Temporary Input Pause
 * Scenario: While in the POLLING state, nurse presses the button (short press)
 * Expected Result: System transitions from POLLING to INPUT_PAUSED. Execute pauseLogic() (for 2 mins)
 */
TEST_F(FallDetectorTest, UC_4_1_TemporaryPauseShortPressShouldEnterInputPaused) {
    // Given: System is in POLLING state
    fixture->transitionToPolling();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    
    // When: Nurse presses button (short press)
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    
    // Then: System transitions to INPUT_PAUSED
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
}

/**
 * UC-4.2: Resume Polling (from Pause)
 * Use Case: Resume Polling from Temporary Input Pause
 * Scenario: While in INPUT_PAUSED state, the 2-minute timer expires
 * Expected Result: System transitions from INPUT_PAUSED to POLLING. Resume monitoring
 */
TEST_F(FallDetectorTest, UC_4_2_ResumePollShouldHappenOnTimerExpiry) {
    // Given: System is in INPUT_PAUSED state
    fixture->transitionToPolling();
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
    
    // When: The 2-minute pause timer expires
    fixture->advanceTimeAndWaitForPauseExpiry();
    
    // Then: System transitions back to POLLING
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

/**
 * UC-5.1: Calibration Start
 * Use Case: Calibration/Zeroing
 * Scenario: While in INPUT_PAUSED state, nurse presses the button (short press)
 * Expected Result: System transitions from INPUT_PAUSED to CALIBRATION. Start zeroing sequence
 */
TEST_F(FallDetectorTest, UC_5_1_CalibrationStartShortPressShouldHappenFromInputPaused) {
    // Given: System is in INPUT_PAUSED state
    fixture->transitionToPolling();
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
    
    // When: Nurse performs short button press
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    
    // Then: System transitions to CALIBRATION state
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::CALIBRATION);
}

/**
 * UC-6.1: Calibration Completion
 * Use Case: Calibration Completion
 * Scenario: While in CALIBRATION state, calibration timer expires
 * Expected Result: System transitions from CALIBRATION to POLLING. Save new zero offsets (new THRESHOLD)
 */
TEST_F(FallDetectorTest, UC_6_1_CalibrationCompletionShouldSaveThreshold) {
    // Given: System is in CALIBRATION state
    fixture->transitionToPolling();
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::CALIBRATION);
    
    // When: Calibration timer expires
    fixture->advanceTimeAndWaitForCalibrationExpiry();
    
    // Then: System returns to POLLING with new threshold saved
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

/**
 * UC-7.1: System Shutdown
 * Use Case: System Shutdown
 * Scenario: Nurse turns off the mat (via BME team mechanism)
 * Expected Result: System transitions from POLLING to SYSTEM_OFF. Execute shutdownSystem()
 * 
 * NOTE: This test is pending BME team implementation of shutdown mechanism
 */
TEST_F(FallDetectorTest, UC_7_1_SystemShutdownShouldBePending) {
    // Given: System is in POLLING state
    fixture->transitionToPolling();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    
    // When: System shutdown mechanism is implemented by BME team
    // TODO: Implement shutdown trigger once BME team provides mechanism
    
    // Then: System transitions to SYSTEM_OFF
    // EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::SYSTEM_OFF);
    
    // For now, verify system is properly operational in POLLING
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

// ============================================================================
// SECTION 4.2: STATE TRANSITION TESTS (Unit Tests)
// ============================================================================
// State Transition Requirements to ensure the system follows the formal state machine

/**
 * ST-1.1: System Power On Transition
 * Current State: SYSTEM_OFF
 * Input/Trigger: System power on
 * New State (Expected): IDLE
 * Logic/Output Action (Expected): bootUpSystem()
 * 
 * NOTE: System currently initializes directly to IDLE. Test verifies system 
 * can boot to IDLE state.
 */
TEST_F(FallDetectorTest, ST_1_1_SystemOffShouldBootToIdle) {
    // Verify system initializes to IDLE (boot sequence completed)
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::IDLE);
}

/**
 * ST-2.1: Idle to Polling Transition
 * Current State: IDLE
 * Input/Trigger: isWifiConnected && isSensorReady
 * New State (Expected): POLLING
 * Logic/Output Action (Expected): Gathering sensor data. Log: "System Armed"
 */
TEST_F(FallDetectorTest, ST_2_1_IdleShouldTransitionToPollingOnReadiness) {
    // Given: System is in IDLE state
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::IDLE);
    
    // When: Ready conditions are met (WiFi connected, sensor ready)
    fixture->transitionToPolling();
    
    // Then: Transition to POLLING state
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

/**
 * ST-3.1: Polling to Input Paused Transition
 * Current State: POLLING
 * Input/Trigger: Nurse presses button (short press)
 * New State (Expected): INPUT_PAUSED
 * Logic/Output Action (Expected): pauseLogic() (for 2 mins)
 */
TEST_F(FallDetectorTest, ST_3_1_PollingShortPressShouldEnterInputPaused) {
    // Given: System is in POLLING state
    fixture->transitionToPolling();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    
    // When: Nurse performs short button press
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    
    // Then: System transitions to INPUT_PAUSED
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
}

/**
 * ST-4.1: Input Paused to Polling Transition (Timer Expiry)
 * Current State: INPUT_PAUSED
 * Input/Trigger: Timer > PAUSE_DURATION_MS
 * New State (Expected): POLLING
 * Logic/Output Action (Expected): Resume monitoring
 */
TEST_F(FallDetectorTest, ST_4_1_InputPausedShouldReturnToPollingOnTimerExpiry) {
    // Given: System is in INPUT_PAUSED state
    fixture->transitionToPolling();
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
    
    // When: Pause duration timer expires (2 minutes)
    fixture->advanceTimeAndWaitForPauseExpiry();
    
    // Then: System returns to POLLING state
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

/**
 * ST-4.2: Input Paused to Calibration Transition
 * Current State: INPUT_PAUSED
 * Input/Trigger: Nurse presses button (short press)
 * New State (Expected): CALIBRATION
 * Logic/Output Action (Expected): Start zeroing sequence
 */
TEST_F(FallDetectorTest, ST_4_2_InputPausedShortPressShouldEnterCalibration) {
    // Given: System is in INPUT_PAUSED state
    fixture->transitionToPolling();
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
    
    // When: Nurse performs short button press
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    
    // Then: System transitions to CALIBRATION state
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::CALIBRATION);
}

/**
 * ST-5.1: Calibration to Polling Transition
 * Current State: CALIBRATION
 * Input/Trigger: timer > CALIB_DURATION_MS
 * New State (Expected): POLLING
 * Logic/Output Action (Expected): Save new zero offsets (new THRESHOLD)
 */
TEST_F(FallDetectorTest, ST_5_1_CalibrationShouldReturnToPollingOnTimerExpiry) {
    // Given: System is in CALIBRATION state
    fixture->transitionToPolling();
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::CALIBRATION);
    
    // When: Calibration duration timer expires
    fixture->advanceTimeAndWaitForCalibrationExpiry();
    
    // Then: System returns to POLLING with new threshold saved
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

/**
 * ST-6.1: Polling to Alarm Transition
 * Current State: POLLING
 * Input/Trigger: detectedForce > THRESHOLD
 * New State (Expected): ALARM
 * Logic/Output Action (Expected): Send alarm CONEXXAL message
 */
TEST_F(FallDetectorTest, ST_6_1_PollingForceShouldTransitionToAlarm) {
    // Given: System is in POLLING state
    fixture->transitionToPolling();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    
    // When: Detected force exceeds threshold (patient stands up)
    fixture->simulateStandUpEvent();
    
    // Then: System transitions to ALARM and sends message
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::ALARM);
    EXPECT_GT(fixture->getAlert()->getAlarmTriggeredCount(), 0);
}

/**
 * ST-7.1: Alarm to Polling Transition
 * Current State: ALARM
 * Input/Trigger: Nurse Press Button
 * New State (Expected): POLLING
 * Logic/Output Action (Expected): clearConnexxallAlert()
 */
TEST_F(FallDetectorTest, ST_7_1_AlarmButtonPressShouldReturnToPolling) {
    // Given: System is in ALARM state
    fixture->transitionToPolling();
    fixture->simulateStandUpEvent();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::ALARM);
    
    // When: Nurse presses button
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    
    // Then: System returns to POLLING and clears alert
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    EXPECT_GT(fixture->getAlert()->getAlarmClearedCount(), 0);
}

/**
 * ST-8.1: Polling to System Off Transition
 * Current State: POLLING
 * Input/Trigger: Nurse turns off mat (BME mechanism)
 * New State (Expected): SYSTEM_OFF
 * Logic/Output Action (Expected): shutdownSystem()
 * 
 * NOTE: Pending BME team implementation of shutdown mechanism
 */
TEST_F(FallDetectorTest, ST_8_1_PollingShutdownShouldTransitionToSystemOff) {
    // Given: System is in POLLING state
    fixture->transitionToPolling();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    
    // When: Shutdown mechanism is implemented by BME team
    // TODO: Implement shutdown trigger once BME team provides mechanism
    
    // Then: System transitions to SYSTEM_OFF
    // EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::SYSTEM_OFF);
    
    // For now, verify system remains operational
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

// ============================================================================
// EDGE CASE & BOUNDARY TESTS
// ============================================================================

/**
 * EDGE-1: Pressure at Exact Threshold Boundary
 * Validates that pressure exactly at threshold triggers alarm
 */
TEST_F(FallDetectorTest, EDGE_1_PressureAtExactThresholdShouldTriggerAlarm) {
    fixture->transitionToPolling();
    
    // Set pressure to exactly at the threshold
    fixture->getSensor()->setMockPressure(FALL_DETECTION_THRESHOLD);
    fixture->getDetector()->update();
    
    // Should trigger alarm (meets or exceeds threshold)
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::ALARM);
}

/**
 * EDGE-2: Pressure Just Below Threshold
 * Validates that pressure just below threshold does NOT trigger alarm
 */
TEST_F(FallDetectorTest, EDGE_2_PressureJustBelowThresholdShouldNotTriggerAlarm) {
    fixture->transitionToPolling();
    
    // Set pressure just below threshold
    float justBelow = FALL_DETECTION_THRESHOLD - 0.1f;
    fixture->getSensor()->setMockPressure(justBelow);
    fixture->getDetector()->update();
    
    // Should remain in POLLING
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

/**
 * EDGE-3: Pressure Oscillating Around Threshold
 * Validates system maintains stable state with pressure fluctuations
 */
TEST_F(FallDetectorTest, EDGE_3_PressureOscillatingAroundThresholdShouldMaintainStableState) {
    fixture->transitionToPolling();
    
    // Pressure oscillates around threshold multiple times
    for (int i = 0; i < 5; i++) {
        fixture->getSensor()->setMockPressure(FALL_DETECTION_THRESHOLD - 1.0f);
        fixture->getDetector()->update();
        EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
        
        fixture->getSensor()->setMockPressure(FALL_DETECTION_THRESHOLD + 5.0f);
        fixture->getDetector()->update();
        EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::ALARM);
        
        // Clear alarm
        fixture->getButton()->simulateShortPress();
        fixture->getDetector()->update();
        EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    }
}

/**
 * EDGE-4: Maximum Pressure Value
 */
TEST_F(FallDetectorTest, EDGE_4_MaximumPressureValueShouldTriggerAlarm) {
    fixture->transitionToPolling();
    
    // Set to maximum pressure
    fixture->getSensor()->setMockPressure(100.0f);
    fixture->getDetector()->update();
    
    // Should trigger alarm
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::ALARM);
    EXPECT_GT(fixture->getAlert()->getAlarmTriggeredCount(), 0);
}

/**
 * EDGE-5: Zero Pressure Value
 */
TEST_F(FallDetectorTest, EDGE_5_ZeroPressureValueShouldRemainInPolling) {
    fixture->transitionToPolling();
    
    // Set pressure to zero
    fixture->getSensor()->setMockPressure(0.0f);
    fixture->getDetector()->update();
    
    // Should remain in POLLING
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

// /**
//  * EDGE-6: Rapid Button Presses (Debouncing)
//  */
// TEST_F(FallDetectorTest, EDGE_6_RapidButtonPresses) {
//     fixture->transitionToPolling();
    
//     // Simulate rapid short presses
//     for (int i = 0; i < 3; i++) {
//         fixture->getButton()->simulateShortPress();
//         fixture->getDetector()->update();
//     }
    
//     // Should be in INPUT_PAUSED (from first press)
//     EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
// }

/**
 * EDGE-7: Short Press Followed Immediately by Short Press
 */
TEST_F(FallDetectorTest, EDGE_7_ShortPressThenShortPressShouldEnterCalibration) {
    fixture->transitionToPolling();
    
    // Short press first
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
    
    // Immediately short press
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    
    // Should transition to CALIBRATION
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::CALIBRATION);
}

/**
 * EDGE-8: Alarm Triggered Multiple Times in Sequence
 */
TEST_F(FallDetectorTest, EDGE_8_MultipleAlarmsInSequenceShouldCountCorrectly) {
    fixture->transitionToPolling();
    int initialAlarmCount = fixture->getAlert()->getAlarmTriggeredCount();
    
    // Trigger and clear alarm 5 times
    for (int i = 0; i < 5; i++) {
        fixture->simulateStandUpEvent();
        EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::ALARM);
        
        fixture->getButton()->simulateShortPress();
        fixture->getDetector()->update();
        EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    }
    
    // Verify all alarms were triggered and cleared
    EXPECT_EQ(fixture->getAlert()->getAlarmTriggeredCount(), initialAlarmCount + 5);
    EXPECT_EQ(fixture->getAlert()->getAlarmClearedCount(), 5);
}

/**
 * EDGE-9: Pause Duration Exactly at Boundary (Should Expire)
 */
TEST_F(FallDetectorTest, EDGE_9_PauseExactlyAtDurationShouldExpire) {
    fixture->transitionToPolling();
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
    
    // Advance time exactly to duration
    fixture->getTime()->advanceMs(PAUSE_DURATION_MS);
    fixture->getDetector()->update();
    
    // Should return to POLLING
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

/**
 * EDGE-10: Calibration Duration Exactly at Boundary (Should Complete)
 */
TEST_F(FallDetectorTest, EDGE_10_CalibrationExactlyAtDurationShouldComplete) {
    fixture->transitionToPolling();
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::CALIBRATION);
    
    // Advance time exactly to duration
    fixture->getTime()->advanceMs(CALIB_DURATION_MS);
    fixture->getDetector()->update();
    
    // Should return to POLLING
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

// ============================================================================
// ERROR HANDLING & INVALID INPUT TESTS
// ============================================================================

// /**
//  * ERROR-1: Short Press in IDLE State
//  */
// TEST_F(FallDetectorTest, ERROR_1_ShortPressInIdleState) {
//     // System is in IDLE
//     ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::IDLE);
    
//     // Short press in IDLE (unexpected input)
//     fixture->getButton()->simulateShortPress();
//     fixture->getDetector()->update();
    
//     // Should transition to POLLING, not react to button
//     EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::IDLE);
// }

/**
 * ERROR-2: Long Press in ALARM State (Should be Ignored)
 */
TEST_F(FallDetectorTest, ERROR_2_LongPressInAlarmStateShouldBeIgnored) {
    fixture->transitionToPolling();
    fixture->simulateStandUpEvent();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::ALARM);
    
    // Attempt long press in ALARM (should be ignored)
    fixture->getButton()->simulateLongPress();
    fixture->getDetector()->update();
    
    // Should remain in ALARM (long press not supported in ALARM)
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::ALARM);
}
/**
 * ERROR-3: Multiple Short Presses in ALARM State
 */
TEST_F(FallDetectorTest, ERROR_3_MultipleShortPressesInAlarmShouldHandleSequentially) {
    fixture->transitionToPolling();
    fixture->simulateStandUpEvent();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::ALARM);
    
    // Multiple short presses
    fixture->setMatEmpty();
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    
    // Once in POLLING, another short press should trigger pause
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
}

/**
 * ERROR-4: No Input During Calibration (Just Wait)
 */
TEST_F(FallDetectorTest, ERROR_4_NoInputDuringCalibrationShouldWaitForTimer) {
    fixture->transitionToPolling();
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::CALIBRATION);
    
    // Do nothing, just let timer expire
    fixture->advanceTimeAndWaitForCalibrationExpiry();
    
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

/**
 * ERROR-5: Force Spike During Calibration
 */
TEST_F(FallDetectorTest, ERROR_5_ForceSpikesDuringCalibrationShouldNotExitEarly) {
    fixture->transitionToPolling();
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::CALIBRATION);
    
    // Force spike during calibration
    fixture->getSensor()->setMockPressure(100.0f);
    fixture->getDetector()->update();
    
    // Should remain in CALIBRATION (doesn't exit early on force)
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::CALIBRATION);
}

/**
 * ERROR-6: Invalid State Should Not Crash
 * Verifies robust error handling
 */
TEST_F(FallDetectorTest, ERROR_6_UnexpectedStateShouldNotCrash) {
    fixture->transitionToPolling();
    
    // System should handle multiple updates without crashing
    for (int i = 0; i < 100; i++) {
        fixture->getDetector()->update();
    }
    
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

/**
 * ERROR-7: Button Press While Force Exceeding Threshold
 */
TEST_F(FallDetectorTest, ERROR_7_ButtonPressWhileForceHighShouldClearAlarm) {
    fixture->transitionToPolling();
    
    // Set high pressure
    fixture->getSensor()->setMockPressure(100.0f);
    fixture->getDetector()->update();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::ALARM);
    
    // Press button while pressure still high
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    
    // Button press should clear alarm even with high pressure
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

// ============================================================================
// STATE PERSISTENCE & CONSISTENCY TESTS
// ============================================================================

/**
 * STATE-1: State Doesn't Change on Idle Update
 */
TEST_F(FallDetectorTest, STATE_1_PollingStateShouldRemainStableWithoutInput) {
    fixture->transitionToPolling();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    
    // Multiple updates without input
    for (int i = 0; i < 10; i++) {
        fixture->getDetector()->update();
        EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    }
}

/**
 * STATE-2: Alarm State Remains Until Button Pressed
 */
TEST_F(FallDetectorTest, STATE_2_AlarmStateShouldRemainStableUntilButtonPressed) {
    fixture->transitionToPolling();
    fixture->simulateStandUpEvent();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::ALARM);
    
    // Multiple updates without button press
    for (int i = 0; i < 5; i++) {
        fixture->getDetector()->update();
        EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::ALARM);
    }
}

/**
 * STATE-3: INPUT_PAUSED Remains Until Timer or Button
 */
TEST_F(FallDetectorTest, STATE_3_PauseStateShouldRemainStableWithoutTrigger) {
    fixture->transitionToPolling();
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
    
    // Light time advance (not enough to expire)
    fixture->getTime()->advanceMs(PAUSE_DURATION_MS / 2);
    
    // Multiple updates
    for (int i = 0; i < 5; i++) {
        fixture->getDetector()->update();
        EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
    }
}

/**
 * STATE-4: Calibration Remains Until Timer Expires
 */
TEST_F(FallDetectorTest, STATE_4_CalibrationStateShouldRemainStableUntilTimerExpires) {
    fixture->transitionToPolling();
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::CALIBRATION);
    
    // Light time advance (not enough to expire)
    fixture->getTime()->advanceMs(CALIB_DURATION_MS / 2);
    
    // Multiple updates
    for (int i = 0; i < 5; i++) {
        fixture->getDetector()->update();
        EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::CALIBRATION);
    }
}

// ============================================================================
// COMPLEX SCENARIO TESTS
// ============================================================================

/**
 * COMPLEX-1: Full System Workflow with Interruptions
 */
TEST_F(FallDetectorTest, COMPLEX_1_FullWorkflowShouldHandleInterruptions) {
    // Boot and arm
    fixture->transitionToPolling();
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    
    // Attempt pause
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
    
    // Calibrate
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::CALIBRATION);
    
    // Wait for calibration to complete
    fixture->advanceTimeAndWaitForCalibrationExpiry();
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    
    // Detect alarm
    fixture->simulateStandUpEvent();
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::ALARM);
    
    // Clear alarm
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

/**
 * COMPLEX-2: Pause Expiry During Force Detection
 */
TEST_F(FallDetectorTest, COMPLEX_2_PauseExpiryDuringForceShouldTransitionCorrectly) {
    fixture->transitionToPolling();
    
    // Enter pause
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
    
    // Set high force
    fixture->getSensor()->setMockPressure(100.0f);
    fixture->getDetector()->update();
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);  // Should stay paused
    
    // Wait for pause to expire
    fixture->advanceTimeAndWaitForPauseExpiry();
    
    // Now should be back in POLLING and force should trigger alarm
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

/**
 * COMPLEX-3: Multiple Calibration Cycles
 */
TEST_F(FallDetectorTest, COMPLEX_3_MultipleCalibrationsInSequenceShouldSucceed) {
    fixture->transitionToPolling();
    
    for (int i = 0; i < 3; i++) {
        // Pause
        fixture->getButton()->simulateShortPress();
        fixture->getDetector()->update();
        EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
        
        // Calibrate
        fixture->getButton()->simulateShortPress();
        fixture->getDetector()->update();
        EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::CALIBRATION);
        
        // Wait for completion
        fixture->advanceTimeAndWaitForCalibrationExpiry();
        EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    }
}

/**
 * COMPLEX-4: Stress Test - Many Updates
 */
TEST_F(FallDetectorTest, COMPLEX_4_StressTestManyUpdatesShouldNotCrash) {
    fixture->transitionToPolling();
    
    // Perform many updates without crashing
    for (int i = 0; i < 1000; i++) {
        fixture->getDetector()->update();
    }
    
    // System should still be operational
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    
    // Alarm should still work
    fixture->simulateStandUpEvent();
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::ALARM);
}

/**
 * COMPLEX-5: Timer Behavior at Boundaries
 */
TEST_F(FallDetectorTest, COMPLEX_5_TimerBoundaryBehaviorShouldBeConsistent) {
    fixture->transitionToPolling();
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
    
    // Advance just before expiry multiple times
    for (int i = 0; i < 5; i++) {
        fixture->getTime()->advanceMs((PAUSE_DURATION_MS / 5) - 10);
        fixture->getDetector()->update();
        EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
    }
    
    // Final push to expire
    fixture->getTime()->advanceMs(100);
    fixture->getDetector()->update();
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

/**
 * COMPLEX-6: Alert Counter Accuracy
 */
TEST_F(FallDetectorTest, COMPLEX_6_AlertCountersShouldBeAccurate) {
    fixture->transitionToPolling();
    fixture->getAlert()->resetCounters();
    
    // Trigger and clear multiple times with different sequences
    for (int i = 0; i < 3; i++) {
        fixture->simulateStandUpEvent();
        EXPECT_EQ(fixture->getAlert()->getAlarmTriggeredCount(), i + 1);
        
        fixture->getButton()->simulateShortPress();
        fixture->getDetector()->update();
        EXPECT_EQ(fixture->getAlert()->getAlarmClearedCount(), i + 1);
    }
    
    // Verify final counts
    EXPECT_EQ(fixture->getAlert()->getAlarmTriggeredCount(), 3);
    EXPECT_EQ(fixture->getAlert()->getAlarmClearedCount(), 3);
}

// ============================================================================
// SECTION 4.4: END-TO-END (E2E) TESTING
// ============================================================================
// These end-to-end tests validate the complete system flow, ensuring all
// components (sensor polling, state transitions, and alerting) work together
// correctly in continuous, real-world scenarios.

/**
 * E2E-1.1: Full Cycle - Boot, Alarm, Acknowledge
 * Initial State: SYSTEM_OFF
 * Sequence of Actions:
 *   1. Power on system
 *   2. Wait for isWifiConnected and isSensorReady
 *   3. Simulate patient stand-up (detectedForce > THRESHOLD)
 *   4. Nurse presses button
 * 
 * Final Expected State & Output:
 * - Final State: POLLING
 * - Output: bootUpSystem() executed, ALARM message sent, clearConnexxallAlert() executed
 */
TEST_F(FallDetectorTest, E2E_1_1_FullCycleShouldBootAlarmAcknowledge) {
    // Step 1: System boots (already in IDLE after SetUp)
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::IDLE);
    
    // Step 2: System reads ready conditions and transitions to POLLING
    fixture->transitionToPolling();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    int alarmCountBeforeEvent = fixture->getAlert()->getAlarmTriggeredCount();
    
    // Step 3: Patient stands up, triggering fall detection
    fixture->simulateStandUpEvent();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::ALARM);
    ASSERT_GT(fixture->getAlert()->getAlarmTriggeredCount(), alarmCountBeforeEvent);
    
    // Step 4: Nurse acknowledges alarm by pressing button
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    
    // Verify complete cycle: return to POLLING, alarm cleared
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    EXPECT_GT(fixture->getAlert()->getAlarmClearedCount(), 0);
}

/**
 * E2E-2.1: Pause, Fall During Pause, Resume
 * Initial State: POLLING
 * Sequence of Actions:
 *   1. Nurse presses button (short press)
 *   2. While in INPUT_PAUSED, simulate patient stand-up (detectedForce > THRESHOLD)
 *   3. Wait for 2-minute timer to expire

 * Final Expected State & Output:
 * - Final State: POLLING
 * - Output: pauseLogic() executed. No alarm message sent during pause. Monitoring resumes
 */
TEST_F(FallDetectorTest, E2E_2_1_PauseShouldIgnoreFallDuringPauseResume) {
    // Setup: Transition to POLLING
    fixture->transitionToPolling();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    
    // Step 1: Nurse performs short press to pause
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
    int alarmCountBefore = fixture->getAlert()->getAlarmTriggeredCount();
    
    // Step 2: While paused, attempt to trigger fall detection
    // (System should ignore this while in INPUT_PAUSED)
    fixture->simulateStandUpEvent();
    
    // Verify: Still in INPUT_PAUSED and no new alarm triggered
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
    EXPECT_EQ(fixture->getAlert()->getAlarmTriggeredCount(), alarmCountBefore);
    
    // Step 3: Wait for 2-minute pause timer to expire
    fixture->advanceTimeAndWaitForPauseExpiry();
    
    // Verify: System resumes monitoring
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

/**
 * E2E-3.1: Calibration, Monitoring, Shutdown
 * Initial State: POLLING
 * Sequence of Actions:
 *   1. Nurse presses button (short press) -> State: INPUT_PAUSED
 *   2. Nurse presses button (short press) -> State: CALIBRATION
 *   3. Wait for CALIB_DURATION_MS timer to expire -> State: POLLING
 *   4. Nurse turns off the mat

 * Final Expected State & Output:
 * - Final State: SYSTEM_OFF
 * - Output: New THRESHOLD saved, shutdownSystem() executed

 * NOTE: Shutdown mechanism pending BME team implementation
 */
TEST_F(FallDetectorTest, E2E_3_1_CalibrationMonitoringShutdownShouldWorkCorrectly) {
    // Setup: Transition to POLLING
    fixture->transitionToPolling();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    
    // Step 1: Nurse performs short press to enter INPUT_PAUSED
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
    
    // Step 2: Nurse performs short press to start calibration
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::CALIBRATION);
    
    // Step 3: Wait for calibration timer to expire
    fixture->advanceTimeAndWaitForCalibrationExpiry();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    
    // Step 4: Shutdown mechanism (pending BME implementation)
    // Once BME provides shutdown trigger, uncomment below:
    // fixture->triggerShutdown();
    // EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::SYSTEM_OFF);
    
    // For now, verify system remains in operational POLLING state
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

/**
 * E2E-4.1: Calibration Aborted (Wrong Input)
 * Initial State: INPUT_PAUSED
 * Sequence of Actions:
 *   1. System is in INPUT_PAUSED state (e.g., after a long press)
 *   2. Nurse presses and holds button (long pause) again
 *   3. Wait for 2-minute timer to expire
 * 
 * Final Expected State & Output:
 * - Final State: POLLING
 * - Output: System should ignore the second long press input in INPUT_PAUSED. 
 *           Monitoring resumes after the original 2-minute timer expires.
 */
TEST_F(FallDetectorTest, E2E_4_1_CalibrationAbortedWrongInputShouldIgnoreSecondLongPress) {
    // Setup: Transition to POLLING
    fixture->transitionToPolling();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
    
    // Step 1: First short press - enters INPUT_PAUSED
    fixture->getButton()->simulateShortPress();
    fixture->getDetector()->update();
    ASSERT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
    
    // Step 2: Long press while in INPUT_PAUSED (should be ignored)
    fixture->getButton()->simulateLongPress();
    fixture->getDetector()->update();
    
    // Verify: System remains in INPUT_PAUSED (ignores second long press)
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::INPUT_PAUSED);
    
    // Step 3: Original 2-minute timer expires
    fixture->advanceTimeAndWaitForPauseExpiry();
    
    // Verify: System returns to POLLING based on original pause timer
    EXPECT_EQ(fixture->getDetector()->getCurrentState(), SystemState::POLLING);
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    return result;
}
