#include "interfaces/IAlertSystem.h"
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

class NtfyHttpAlert : public IAlertSystem {
private:
    const char* _endpoint;
    bool _alarmActive;

    bool sendNotification(const char* message) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("ERROR: WiFi not connected");
            return false;
        }
        HTTPClient http;
        http.begin(_endpoint);
        http.addHeader("Content-Type", "text/plain");
        int responseCode = http.POST(message);
        http.end();
        return responseCode > 0;
    }

public:
    NtfyHttpAlert() : _endpoint(NTFY_HTTP_ENDPOINT), _alarmActive(false) {}

    void init() override {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WARNING: WiFi not connected during initialization");
        }
        Serial.println("Alert system initialized");
    }

    void triggerFallAlarm() override {
        _alarmActive = true;
        if (sendNotification("PATIENT HAS FALLEN!")) {
            Serial.println("Fall alarm sent to Ntfy");
        } else {
            Serial.println("Failed to send fall alarm to Ntfy");
        }
    }

    void clearAlarm() override {
        if (_alarmActive) {
            _alarmActive = false;
            if (sendNotification("ALARM CLEARED")) {
                Serial.println("Alarm cleared via Ntfy");
            } else {
                Serial.println("Failed to send alarm clear to Ntfy");
            }
        }
    }
};
