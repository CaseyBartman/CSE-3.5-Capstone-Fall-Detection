#pragma once

// WiFi Configuration
const char* WIFI_SSID = "SpectrumSetup-03";
const char* WIFI_PASSWORD = "mellowstreet073";
const char* WOKWI_WIFI_SSID = "Wokwi-GUEST";

// // Connexxall API Configuration
// const char* CONNEXXALL_API_ENDPOINT = "https://api.connexxall.com/alerts";
// const char* CONNEXXALL_API_KEY = "API_KEY";

// // BLE Configuration
// const char* BLE_DEVICE_MAC = "BLE_MAC_ADDRESS";
// const char* BLE_SERVICE_UUID = "SERVICE_UUID";

// Ntfy HTTP Configuration
const char* NTFY_HTTP_ENDPOINT = "https://ntfy.sh/patient_fall_alert_5911";

// ESP32 Button Controller -> Arduino Uno R4 Configuration
const char* ARDUINO_HOST = "192.168.1.181";  // Arduino Uno R4 WiFi IP address
const int ARDUINO_PORT = 8080;               // HTTP server port on Arduino
const char* BUTTON_SIGNAL_ENDPOINT = "http://192.168.1.181:8080/button";  // Full endpoint URL