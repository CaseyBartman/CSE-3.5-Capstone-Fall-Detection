#pragma once

#include "constants/SystemConstants.h"

/**
 * Test Data Constants
 * 
 * Used to drive test behavior in a data-driven way, removing magic numbers.
 * All values reference system constants or have clear semantic meaning.
 */

// Pressure test values - relative to FALL_DETECTION_THRESHOLD
const float TEST_PRESSURE_PATIENT_SITTING = 10.0f;              // Below occupied threshold
const float TEST_PRESSURE_PATIENT_STANDING = 100.0f;            // Well above any threshold
const float TEST_PRESSURE_BELOW_THRESHOLD = 18.0f;              // Below FALL_DETECTION_THRESHOLD
const float TEST_PRESSURE_NEAR_THRESHOLD = 25.0f;               // Near but below FALL_DETECTION_THRESHOLD
const float TEST_PRESSURE_ABOVE_THRESHOLD = 50.0f;              // Well above FALL_DETECTION_THRESHOLD

// Timing test values
const unsigned long TEST_TIME_BEFORE_EXPIRY = 1000;             // 1 second before timeout
const unsigned long TEST_OCCUPIED_STATE_INITIAL = 40.0f;        // Initial occupied pressure

// NTFY Testing Values (See TestConstants.cpp for initialization to fix global linker issue)
extern const char* TEST_NTFY_ENDPOINT;
extern const char* EXPECTED_FALL_PAYLOAD;
extern const char* EXPECTED_CLEAR_PAYLOAD;
extern const char* EXPECTED_CONTENT_TYPE;


// Validation helpers
static_assert(TEST_PRESSURE_BELOW_THRESHOLD < FALL_DETECTION_THRESHOLD, 
    "TEST_PRESSURE_BELOW_THRESHOLD must be below FALL_DETECTION_THRESHOLD");
static_assert(TEST_PRESSURE_NEAR_THRESHOLD < FALL_DETECTION_THRESHOLD, 
    "TEST_PRESSURE_NEAR_THRESHOLD must be below FALL_DETECTION_THRESHOLD");
static_assert(TEST_PRESSURE_ABOVE_THRESHOLD > FALL_DETECTION_THRESHOLD, 
    "TEST_PRESSURE_ABOVE_THRESHOLD must be above FALL_DETECTION_THRESHOLD");
