#pragma once
#include "interfaces/IAlertSystem.h"
#include "interfaces/INetworkClient.h"
#include <Arduino.h> // For Serial

class NtfyHttpAlert : public IAlertSystem {
private:
    INetworkClient* _networkClient;
    const char* _endpointUrl;
    bool _isAlarmActive;

    bool sendNotification(const char* messagePayload) {
        if (!_networkClient->isNetworkConnected()) {
            Serial.println("ERROR: Network unavailable for transmission");
            return false;
        }
        
        const char* textContentType = "text/plain";
        int responseStatusCode = _networkClient->post(_endpointUrl, textContentType, messagePayload);
        
        Serial.print("Notification sent. Response status code: ");
        Serial.println(responseStatusCode);

        bool isSuccess = responseStatusCode > 0 && responseStatusCode < 400;
        if (!isSuccess) {
            Serial.print("ERROR: HTTP POST failed. Response body: ");
            Serial.println(_networkClient->getResponseBody());
        }
        
        return isSuccess;
    }

public:
    NtfyHttpAlert(INetworkClient* networkClient, const char* endpointUrl) 
        : _networkClient(networkClient), _endpointUrl(endpointUrl), _isAlarmActive(false) {}

    void init() override {
        if (!_networkClient->isNetworkConnected()) {
            Serial.println("WARNING: Network disconnected during initialization");
        }
    }

    void triggerFallAlarm() override {
        _isAlarmActive = true;
        if (!sendNotification("PATIENT HAS FALLEN!")) {
            Serial.println("DEBUG: Fall alarm transmission failed");
        }
    }

    void clearAlarm() override {
        if (_isAlarmActive) {
            _isAlarmActive = false;
            if (!sendNotification("ALARM CLEARED")) {
                Serial.println("DEBUG: Alarm clear transmission failed");
            }
        }
    }
};
