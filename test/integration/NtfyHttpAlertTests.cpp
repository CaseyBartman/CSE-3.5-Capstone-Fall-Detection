#include "logic/NtfyHttpAlert.h"
#include "mocks/NetworkClientSpy.h"
#include "gtest/gtest.h"
#include "TestConstants.h"

class NtfyHttpAlertTests : public ::testing::Test {
protected:
    NetworkClientSpy networkSpy;
    std::unique_ptr<NtfyHttpAlert> alertSystem;

    void SetUp() override {
        alertSystem = std::make_unique<NtfyHttpAlert>(&networkSpy, TEST_NTFY_ENDPOINT);
        networkSpy.setSimulatedConnectionState(true); // Assume network is connected by default
        networkSpy.setSimulatedResponseCode(200);     // Assume success HTTP response by default
        alertSystem->init();
    }

    void TearDown() override {
        networkSpy.clearCapturedState();
    }
};

// Test case: Verifies that triggering a fall alarm sends the correct HTTP payload
TEST_F(NtfyHttpAlertTests, ALARM_NTFY_TriggerFallAlarm_TransmitsCorrectPayload) {
    // Act: Trigger the fall alarm
    alertSystem->triggerFallAlarm();

    // Assert: Check if the correct HTTP request was captured
    const auto* requests = networkSpy.getCapturedRequests();
    
    // We use getCapturedCount() because raw arrays don't have .size()
    ASSERT_EQ(networkSpy.getCapturedCount(), 1) << "Expected exactly one HTTP request to be captured";
    
    // We use EXPECT_STREQ for char* to compare the text content instead of the memory address
    EXPECT_STREQ(requests[0].targetUrl, TEST_NTFY_ENDPOINT)  << "Request sent to incorrect endpoint";
    EXPECT_STREQ(requests[0].contentType, EXPECTED_CONTENT_TYPE)  << "Incorrect content type header";
    EXPECT_STREQ(requests[0].payload, EXPECTED_FALL_PAYLOAD)  << "Captured payload did not match expected fall alert";
}

// Test case: Verifies that clearing an active alarm sends the correct HTTP payload
TEST_F(NtfyHttpAlertTests, ALARM_NTFY_ClearAlarm_WhenAlarmIsActive_TransmitsCorrectPayload) {
    // Arrange: First, trigger the alarm to make it active
    alertSystem->triggerFallAlarm();
    networkSpy.clearCapturedState(); // Clear the request from the trigger action

    // Act: Clear the alarm
    alertSystem->clearAlarm();

    // Assert: Check if the correct "cleared" message was sent
    const auto* requests = networkSpy.getCapturedRequests();
    ASSERT_EQ(networkSpy.getCapturedCount(), 1) << "Expected exactly one HTTP request for clearing the alarm";
    EXPECT_STREQ(requests[0].payload, EXPECTED_CLEAR_PAYLOAD)  << "Captured payload did not match expected clear message";
}

// Test case: Verifies that no HTTP request is sent when trying to clear an alarm that is not active
TEST_F(NtfyHttpAlertTests, ALARM_NTFY_ClearAlarm_WhenAlarmIsNotActive_DoesNotTransmit) {
    // Act: Attempt to clear an alarm that was never triggered
    alertSystem->clearAlarm();

    // Assert: Ensure no requests were sent
    EXPECT_EQ(networkSpy.getCapturedCount(), 0) << "Expected no HTTP requests when clearing an inactive alarm";
}

// Test case: Verifies that no HTTP request is sent when the network is disconnected
TEST_F(NtfyHttpAlertTests, ALARM_NTFY_TriggerFallAlarm_WhenNetworkIsDisconnected_DoesNotTransmit) {
    // Arrange: Simulate a disconnected network
    networkSpy.setSimulatedConnectionState(false);

    // Act: Trigger the fall alarm
    alertSystem->triggerFallAlarm();

    // Assert: Ensure no requests were sent due to the network being down
    EXPECT_EQ(networkSpy.getCapturedCount(), 0) << "Expected no HTTP requests when the network is disconnected";
}

// Test case: Verifies that the system handles a failed HTTP POST correctly
TEST_F(NtfyHttpAlertTests, ALARM_NTFY_TriggerFallAlarm_WhenPostFails_HandlesFailure) {
    // Arrange: Simulate a server error response
    networkSpy.setSimulatedResponseCode(500);

    // Act: Trigger the fall alarm
    alertSystem->triggerFallAlarm();

    // Assert: A request should have been made, even if it failed.
    // But we can confirm that an attempt was made.
    EXPECT_EQ(networkSpy.getCapturedCount(), 1) << "Expected one HTTP request attempt even if it fails";
}
