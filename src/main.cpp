#include <Arduino.h>
#include "logic/FallDetector.h"
#include "constants/SystemConstants.h"
#include "constants/NetworkConstants.h"
#include "util/WiFiSetup.cpp"

#ifdef REAL_BASIC
    #define IS_REAL
#endif

#ifdef IS_SIMULATION
    #include "drivers/sim/WokwiPotentiometer.cpp"
    #include "drivers/sim/WokwiButton.cpp"
    #include "drivers/sim/SerialConsoleAlert.cpp"
#elif defined(IS_REAL)
    #include "drivers/real/RealTekscan.h"
    #include "drivers/real/PhysicalButton.h"
    #include "drivers/real/NtfyHttpAlert.h"
    #include "drivers/real/ArduinoNetworkClient.h"
    #include <WiFiS3.h>
#elif defined(IS_ARCHIVE)
    #include "drivers/archive/TekscanA502.cpp"
    #include "drivers/archive/BlueCharmBLE.cpp"
    #include "drivers/archive/LEDAlert.cpp"
    #include "drivers/real/EspNetworkClient.h"
#endif

FallDetector* systemController = nullptr;

#ifdef IS_REAL
WiFiServer* buttonServer = nullptr;
PhysicalButton* esp32ButtonRef = nullptr;
#endif

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(SERIAL_STARTUP_DELAY_MS);
    
    Serial.println("========================================");
    Serial.println("Patient Fall Detection System");
    Serial.println("========================================");

    #ifdef IS_SIMULATION
        Serial.println("Running in SIMULATION mode (Wokwi)");
        WiFiSetup::setupWifi(WOKWI_WIFI_SSID);

        auto* sensor = new WokwiPotentiometer(SIM_SENSOR_PIN, DEFAULT_PRESSURE_THRESHOLD);
        auto* button = new WokwiButton(SIM_BUTTON_PIN);
        auto* alert  = new SerialConsoleAlert();

    #elif defined(IS_REAL)
        Serial.println("Running in REAL mode (Tekscan A502 + PhysicalButton + Ntfy)");
        WiFiSetup::setupWifi(WIFI_SSID, WIFI_PASSWORD);
        /* Physical pins (replace when changed) */
        auto* sensor        = new RealTekscan(ARDUINO_SENSOR_PIN, DEFAULT_PRESSURE_THRESHOLD);
        auto* button        = new PhysicalButton(ARDUINO_BUTTON_PIN);
        esp32ButtonRef      = button;
        auto* networkClient = new ArduinoNetworkClient();
        auto* alert         = new NtfyHttpAlert(networkClient, NTFY_HTTP_ENDPOINT);

        buttonServer = new WiFiServer(ARDUINO_PORT);
        buttonServer->begin();
        Serial.println("HTTP server started - waiting for ESP32 signals at /trigger");

    #elif defined(IS_ARCHIVE)
        Serial.println("Running in ARCHIVE mode (TekscanA502 + BlueCharm BLE + LED)");
        auto* sensor = new TekscanA502(ARCHIVE_SENSOR_PIN, DEFAULT_PRESSURE_THRESHOLD);
        auto* button = new BlueCharmBLE(BLE_DEVICE_MAC);
        auto* alert  = new LEDAlert(ARCHIVE_LED_PIN);

    #endif

    systemController = new FallDetector(sensor, button, alert);
    systemController->init();
    
    Serial.println("========================================");
    Serial.println("Setup complete - Entering main loop");
    Serial.println("========================================");
}

void loop() {
    systemController->update();
    
    #ifdef IS_REAL
    if (buttonServer && esp32ButtonRef) {
        WiFiClient client = buttonServer->available();
        if (client) {
            esp32ButtonRef->handleHttpClient(client);
        }
    }
    #endif
    
    delay(SENSOR_SAMPLE_RATE_MS);
}

