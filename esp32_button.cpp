/**
 * ESP32 Button Controller - Simplified Client
 * Sends HTTP GET requests to Arduino server when button is pressed
 */

#include <WiFi.h>
#include <HTTPClient.h>

// WiFi Configuration
const char* WIFI_SSID = "SpectrumSetup-03";
const char* WIFI_PASSWORD = "mellowstreet073";

// Arduino Server URL
const char* SERVER_URL = "http://192.168.1.181/trigger";

// Button pin
const int BUTTON_PIN = 13;

int lastButtonState = HIGH;
unsigned long lastPressTime = 0;

// Forward declaration
void sendSignalToArduino();

void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    delay(1000);
    Serial.println("ESP32 Button Controller Starting...");

    // Connect to WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\n✅ ESP32 Connected to WiFi!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Will send signals to: ");
    Serial.println(SERVER_URL);
}

void loop() {
    int currentState = digitalRead(BUTTON_PIN);

    // Detect button press (with debounce)
    if (currentState == LOW && lastButtonState == HIGH && millis() - lastPressTime > 300) {
        Serial.println("🔘 Button pressed → sending signal to Arduino");

        sendSignalToArduino();
        lastPressTime = millis();
    }

    lastButtonState = currentState;
    delay(50);
}

void sendSignalToArduino() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        Serial.println("📡 Sending HTTP request...");

        http.begin(SERVER_URL);
        int httpResponseCode = http.GET();

        Serial.print("Response code: ");
        Serial.println(httpResponseCode);

        if (httpResponseCode > 0) {
            String payload = http.getString();
            Serial.println("Arduino response: " + payload);
        } else {
            Serial.println("❌ Request failed");
        }

        http.end();
    } else {
        Serial.println("❌ WiFi not connected");
    }
}
