#pragma once
#include "interfaces/INetworkClient.h"
#include <WiFi.h>
#include <HTTPClient.h>

class EspNetworkClient : public INetworkClient {
private:
    String _lastResponseBody; // Use Arduino String to hold response body

public:
    bool isNetworkConnected() const override {
        return WiFi.status() == WL_CONNECTED;
    }

    int post(const char* endpointUrl, const char* contentType, const char* payload) override {
        HTTPClient httpClient;
        httpClient.begin(endpointUrl);
        httpClient.addHeader("Content-Type", contentType);
        
        int responseStatusCode = httpClient.POST(payload);
        
        if (responseStatusCode < 0 || responseStatusCode >= 400) {
            _lastResponseBody = httpClient.getString();
        } else {
            _lastResponseBody = "";
        }
        
        httpClient.end();
        
        return responseStatusCode;
    }

    const char* getResponseBody() const override {
        return _lastResponseBody.c_str();
    }
};
