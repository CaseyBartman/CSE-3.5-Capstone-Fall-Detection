#ifdef ESP32

#include <WiFi.h>
#include <HTTPClient.h>
#include "constants/NetworkConstants.h"
#include "constants/SystemConstants.h"

int lastButtonReading = HIGH;
unsigned long lastPressTime = 0;

void sendSignalToArduino();

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    pinMode(ESP32_BUTTON_PIN, INPUT_PULLUP);

    delay(ESP32_STARTUP_DELAY_MS);
    Serial.println("ESP32 Button Controller Starting...");

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");

    while (WiFi.status() != WL_CONNECTED) {
        delay(WIFI_CONNECT_DELAY_MS);
        Serial.print(".");
    }

    Serial.println("\nESP32 Connected to WiFi!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Will send signals to: ");
    Serial.println(BUTTON_SIGNAL_ENDPOINT);
}

void loop() {
    int currentButtonReading = digitalRead(ESP32_BUTTON_PIN);

    if (currentButtonReading == LOW && lastButtonReading == HIGH && millis() - lastPressTime > ESP32_BUTTON_DEBOUNCE_MS) {
        Serial.println("Button pressed -> sending signal to Arduino");

        sendSignalToArduino();
        lastPressTime = millis();
    }

    lastButtonReading = currentButtonReading;
    delay(ESP32_LOOP_DELAY_MS);
}

void sendSignalToArduino() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        Serial.println("Sending HTTP request...");

        http.begin(BUTTON_SIGNAL_ENDPOINT);
        int httpResponseCode = http.GET();

        Serial.print("Response code: ");
        Serial.println(httpResponseCode);

        if (httpResponseCode > 0) {
            String payload = http.getString();
            Serial.println("Arduino response: " + payload);
        } else {
            Serial.println("Request failed");
        }

        http.end();
    } else {
        Serial.println("WiFi not connected");
    }
}

#endif
