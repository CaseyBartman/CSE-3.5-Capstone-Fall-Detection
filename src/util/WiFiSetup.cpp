#include <WiFi.h>

class WiFiSetup {
public:
    static void setupWifi(const char* ssid, const char* password = "") {
        WiFi.begin(ssid, password);

        while (WiFi.status() != WL_CONNECTED) {
            delay(1000);
            Serial.println("Connecting...");
        }

        Serial.print("Connected to IP Address: ");
        Serial.println(WiFi.localIP());
    }
};
