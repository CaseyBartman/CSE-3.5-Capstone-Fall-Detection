#include <Arduino.h>
#include "logic/FallDetector.h"
#include "constants/SystemConstants.h"
#include "constants/NetworkConstants.h"
#include "util/WiFiSetup.cpp"

// Build mode mapping
#ifdef REAL_BASIC
    #define IS_REAL
#endif

// Conditional imports based on build flags
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
    #ifdef ESP32_BUTTON
    #include "drivers/real/EspNetworkClient.h"
    #endif
#elif defined(IS_ARCHIVE)
    #include "drivers/archive/TekscanA502.cpp"
    #include "drivers/archive/BlueCharmBLE.cpp"
    #include "drivers/archive/LEDAlert.cpp"
    #include "drivers/real/EspNetworkClient.h" // shared network client for
#endif

FallDetector* systemController = nullptr;

#ifdef IS_REAL
// Simple HTTP server for ESP32 button signals
WiFiServer* buttonServer = nullptr;
volatile bool esp32ButtonPressed = false;
unsigned long lastEsp32SignalTime = 0;

// Forward declaration
void handleEsp32ButtonRequest(WiFiClient client);
#endif

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
        #ifdef ESP32_BUTTON
        auto* networkClient = new EspNetworkClient();
        auto* alert         = new NtfyHttpAlert(networkClient, NTFY_HTTP_ENDPOINT);
        #else
        auto* networkClient = new ArduinoNetworkClient();
        auto* alert         = new NtfyHttpAlert(networkClient, NTFY_HTTP_ENDPOINT);
        #endif

        // Initialize simple HTTP server for ESP32 button signals
        buttonServer = new WiFiServer(80);  // Listen on port 80
        buttonServer->begin();
        Serial.println("HTTP server started on port 80 - waiting for ESP32 signals at /trigger");

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
    
    #ifdef IS_REAL
    // Check for incoming ESP32 button signal HTTP requests
    if (buttonServer) {
        WiFiClient client = buttonServer->available();
        if (client) {
            handleEsp32ButtonRequest(client);
        }
    }
    
    // Process ESP32 button press events
    if (esp32ButtonPressed && (millis() - lastEsp32SignalTime > 100)) {
        Serial.println("🚨 ESP32 button signal received - triggering manual alert!");
        // TODO: Integrate with fall detection system - could trigger alert or test mode
        esp32ButtonPressed = false;
    }
    #endif
    
    //Small delay for system stability. To be tested with physical sensor...
    delay(SENSOR_SAMPLE_RATE_MS);
}

#ifdef IS_REAL
void handleEsp32ButtonRequest(WiFiClient client) {
    Serial.println("📡 ESP32 client connected");

    // Wait until data is available
    while (!client.available()) {
        delay(1);
    }

    // Read the request line
    String request = client.readStringUntil('\n');
    Serial.println("Request: " + request);

    // Clear remaining data
    while (client.available()) {
        client.read();
    }

    // Check for trigger endpoint
    if (request.indexOf("GET /trigger") != -1) {
        Serial.println("🚨 ESP32 BUTTON SIGNAL RECEIVED!");
        esp32ButtonPressed = true;
        lastEsp32SignalTime = millis();
    }

    // Send HTTP response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println("UNO OK");

    delay(1);
    client.stop();
    Serial.println("🔌 ESP32 client disconnected");
}
#endif
