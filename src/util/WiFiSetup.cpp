#ifdef ESP32
#include <WiFi.h>
#else
#include <WiFiS3.h>
#endif
#include "constants/SystemConstants.h"

class WiFiSetup {
public:
    static bool setupWifi(const char* ssid, const char* password = "") {
        WiFi.begin(ssid, password);

        int attempts = 0;

        while (WiFi.status() != WL_CONNECTED && attempts < WIFI_MAX_ATTEMPTS) {
            delay(WIFI_CONNECT_DELAY_MS);
            Serial.println("Connecting...");
            Serial.println(WiFi.status()); // Add this line
            attempts++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("WiFi connected!");
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
            return true;
        } else {
            Serial.println("WiFi connection FAILED");
            return false;
        }
    }
};