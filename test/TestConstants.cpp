#include "test/TestConstants.h"

// NTFY Testing Values. Initialized here to prevent linker issues from conflicting with production code
const char* TEST_NTFY_ENDPOINT = "http://ntfy.sh/test_topic";
const char* EXPECTED_FALL_PAYLOAD = "PATIENT HAS FALLEN!";
const char* EXPECTED_CLEAR_PAYLOAD = "ALARM CLEARED";
const char* EXPECTED_CONTENT_TYPE = "text/plain";
