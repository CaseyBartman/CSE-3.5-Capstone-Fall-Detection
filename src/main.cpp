#include <Arduino.h>
#include "logic/FallDetector.h"
#include "constants/SystemConstants.h"
#include "constants/NetworkConstants.h"
#include "util/WiFiSetup.cpp"

// Conditional imports based on build flags
#ifdef IS_SIMULATION
    #include "drivers/sim/WokwiPotentiometer.cpp"
    #include "drivers/sim/WokwiButton.cpp"
    #include "drivers/sim/SerialConsoleAlert.cpp"
#elif defined(IS_REAL)
    #include "drivers/real/RealTekscan.h"
    #include "drivers/real/PhysicalButton.h"
    #include "drivers/real/NtfyHttpAlert.h"
    #include "drivers/real/EspNetworkClient.h"
#elif defined(IS_ARCHIVE)
    #include "drivers/archive/TekscanA502.cpp"
    #include "drivers/archive/BlueCharmBLE.cpp"
    #include "drivers/archive/LEDAlert.cpp"
    #include "drivers/real/EspNetworkClient.h" // shared network client for 
#else
    #error "No build mode defined. Set IS_SIMULATION, IS_REAL, or IS_ARCHIVE via build_flags."
#endif

FallDetector* systemController = nullptr;

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(2000);  // Give serial monitor time to connect before printing startup messages
    
    Serial.println("========================================");
    Serial.println("Patient Fall Detection System");
    Serial.println("========================================");

    #ifdef IS_SIMULATION
        Serial.println("Running in SIMULATION mode (Wokwi)");
        WiFiSetup::setupWifi(WOKWI_WIFI_SSID);

        auto* sensor = new WokwiPotentiometer(34, DEFAULT_PRESSURE_THRESHOLD);
        auto* button = new WokwiButton(15);
        auto* alert  = new SerialConsoleAlert();

    #elif defined(IS_REAL)
        Serial.println("Running in REAL mode (Tekscan A502 + PhysicalButton + Ntfy)");
        WiFiSetup::setupWifi(WIFI_SSID, WIFI_PASSWORD);
        /* Physical pins (replace when changed) */
        auto* sensor        = new RealTekscan(A0, DEFAULT_PRESSURE_THRESHOLD);
        auto* button        = new PhysicalButton(2);
        auto* networkClient = new EspNetworkClient();
        auto* alert         = new NtfyHttpAlert(networkClient, NTFY_HTTP_ENDPOINT);

    #elif defined(IS_ARCHIVE)
        Serial.println("Running in ARCHIVE mode (TekscanA502 + BlueCharm BLE + LED)");
        auto* sensor = new TekscanA502(34, DEFAULT_PRESSURE_THRESHOLD);
        auto* button = new BlueCharmBLE(BLE_DEVICE_MAC);
        auto* alert  = new LEDAlert(9);

    #endif

    systemController = new FallDetector(sensor, button, alert);
    systemController->init();
    
    Serial.println("========================================");
    Serial.println("Setup complete - Entering main loop");
    Serial.println("========================================");
}

void loop() {
    systemController->update();
    
    //Small delay for system stability. To be tested with physical sensor...
    delay(SENSOR_SAMPLE_RATE_MS);
}
