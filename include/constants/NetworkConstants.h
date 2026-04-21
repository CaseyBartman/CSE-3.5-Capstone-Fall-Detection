#pragma once

static const char* WIFI_SSID = "SpectrumSetup-03";
static const char* WIFI_PASSWORD = "mellowstreet073";
static const char* WOKWI_WIFI_SSID = "Wokwi-GUEST";

static const char* NTFY_HTTP_ENDPOINT = "https://ntfy.sh/patient_fall_alert_5911";
static const char* NTFY_FALL_MESSAGE = "PATIENT HAS FALLEN!";
static const char* NTFY_CLEAR_MESSAGE = "ALARM CLEARED";

static const char* ARDUINO_HOST = "192.168.1.181";
const int ARDUINO_PORT = 80;
static const char* ARDUINO_TRIGGER_PATH = "/trigger";
static const char* BUTTON_SIGNAL_ENDPOINT = "http://192.168.1.181/trigger";