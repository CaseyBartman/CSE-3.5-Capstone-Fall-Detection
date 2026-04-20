#pragma once

const unsigned long PAUSE_DURATION_MS = 120000;
const unsigned long CALIB_DURATION_MS = 5000;
const unsigned long DEBOUNCE_DELAY_MS = 50;
const unsigned long LONG_PRESS_MS = 2000;
const unsigned long WIFI_CONNECT_DELAY_MS = 500;
const unsigned long NETWORK_RESPONSE_TIMEOUT_MS = 8000;
const unsigned long ESP32_STARTUP_DELAY_MS = 1000;
const unsigned long ESP32_LOOP_DELAY_MS = 50;
const unsigned long ESP32_BUTTON_DEBOUNCE_MS = 300;

const float DEFAULT_PRESSURE_THRESHOLD = 5.0;
const float FALL_DETECTION_THRESHOLD = 40.0;
const float MIN_PRESSURE_PERCENTAGE = 0.0f;
const float MAX_PRESSURE_PERCENTAGE = 100.0f;

const int SENSOR_SAMPLE_RATE_MS = 50;
const int SERIAL_BAUD_RATE = 115200;
const int WIFI_MAX_ATTEMPTS = 20;
const int ESP32_BUTTON_PIN = 13;

const int HTTP_DEFAULT_PORT = 80;
const int HTTPS_DEFAULT_PORT = 443;
const int HTTP_PREFIX_LENGTH = 7;
const int HTTPS_PREFIX_LENGTH = 8;
const int HTTP_STATUS_CODE_OFFSET = 9;
