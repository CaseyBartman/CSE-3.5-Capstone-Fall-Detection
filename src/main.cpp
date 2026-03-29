#include <Arduino.h>
#include "logic/FallDetector.h"
#include "constants/SystemConstants.h"

// Conditional imports based on build flags
#ifdef IS_SIMULATION
    #include "drivers/sim/WokwiPotentiometer.cpp"
    #include "drivers/sim/WokwiButton.cpp"
    #include "drivers/sim/SerialConsoleAlert.cpp"
#elif defined(REAL_BASIC)
    #include "drivers/real-basic/RealTekscan.h"
    #include "drivers/real-basic/PhysicalButton.h"
    #include "drivers/real-basic/LEDAlert.h"
#else
    #include "drivers/real/TekscanA502.cpp"
    #include "drivers/real/BlueCharmBLE.cpp"
    #include "drivers/real/ConnexxallWiFi.cpp"
    #include "constants/NetworkConstants.h"
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
        auto* sensor = new WokwiPotentiometer(34, DEFAULT_PRESSURE_THRESHOLD);
        auto* button = new WokwiButton(15);
        auto* alert  = new SerialConsoleAlert();
    #elif defined(REAL_BASIC)
        Serial.println("Running in REAL-BASIC mode (Tekscan A502 + LED + Button)");
        /* Physical pins(replace when changed)*/
        auto* sensor = new RealTekscan(A0, DEFAULT_PRESSURE_THRESHOLD);
        auto* button = new PhysicalButton(2); 
        auto* alert  = new LEDAlert(9);   
    #else
        Serial.println("Running in PRODUCTION mode (Real Hardware)");
        auto* sensor = new TekscanA502(34, DEFAULT_PRESSURE_THRESHOLD);
        auto* button = new BlueCharmBLE(BLE_DEVICE_MAC);
        auto* alert  = new ConnexxallWiFi(WIFI_SSID, WIFI_PASSWORD);
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
