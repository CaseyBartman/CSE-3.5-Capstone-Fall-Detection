#include <Arduino.h>
#include "logic/FallDetector.h"
#include "constants/SystemConstants.h"

// Conditional imports based on platformio.ini build flags
#ifdef IS_SIMULATION
    #include "drivers/sim/WokwiPotentiometer.cpp"
    #include "drivers/sim/WokwiButton.cpp"
    #include "drivers/sim/SerialConsoleAlert.cpp"
#else
    #include "drivers/real/TekscanA502.cpp"
    #include "drivers/real/BlueCharmBLE.cpp"
    #include "drivers/real/ConnexxallWiFi.cpp"
    #include "constants/NetworkConstants.h"
#endif

// Global pointer (Initialized in setup)
FallDetector* systemController = nullptr;

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000); // Allow serial to initialize
    
    Serial.println("========================================");
    Serial.println("Patient Fall Detection System");
    Serial.println("========================================");

    // 1. Instantiate drivers based on environment
    #ifdef IS_SIMULATION
        Serial.println("Running in SIMULATION mode (Wokwi)");
        auto* sensor = new WokwiPotentiometer(34, DEFAULT_PRESSURE_THRESHOLD); // Pin 34 (ADC1_CH6)
        auto* button = new WokwiButton(15);                                     // Pin 15
        auto* alert  = new SerialConsoleAlert();
    #else
        Serial.println("Running in PRODUCTION mode (Real Hardware)");
        auto* sensor = new TekscanA502(34, DEFAULT_PRESSURE_THRESHOLD);
        auto* button = new BlueCharmBLE(BLE_DEVICE_MAC);
        auto* alert  = new ConnexxallWiFi(WIFI_SSID, WIFI_PASSWORD);
    #endif

    // 2. Inject dependencies into logic controller
    systemController = new FallDetector(sensor, button, alert);
    
    // 3. Initialize the system
    systemController->init();
    
    Serial.println("========================================");
    Serial.println("Setup complete - Entering main loop");
    Serial.println("========================================");
}

void loop() {
    // 4. Delegate to logic controller
    systemController->update();
    
    // Small delay for system stability
    delay(SENSOR_SAMPLE_RATE_MS);
}
