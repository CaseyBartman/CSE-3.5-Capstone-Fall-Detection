#ifdef ESP32
#include <WiFi.h>
#else
#include <WiFiS3.h>
#endif

class WiFiSetup {
public:
    static bool setupWifi(const char* ssid, const char* password = "") {
        WiFi.begin(ssid, password);

        int attempts = 0;
        const int maxAttempts = 20;

        while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
            delay(500);
            Serial.println("Connecting...");
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