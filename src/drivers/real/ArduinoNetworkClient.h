#pragma once
#include "interfaces/INetworkClient.h"
#include "constants/SystemConstants.h"
#include <WiFiS3.h>

class ArduinoNetworkClient : public INetworkClient {
private:
    String _lastResponseBody;

public:
    bool isNetworkConnected() const override {
        return WiFi.status() == WL_CONNECTED;
    }

    int post(const char* endpointUrl, const char* contentType, const char* payload) override {
        String url = endpointUrl;
        bool isHttps = url.startsWith("https://");
        int prefixLen = isHttps ? HTTPS_PREFIX_LENGTH : HTTP_PREFIX_LENGTH;

        int colonAfterHost = url.indexOf(':', prefixLen);
        int firstSlash = url.indexOf('/', prefixLen);
        if (firstSlash < 0) firstSlash = url.length();

        String host;
        int port;
        String path;

        if (colonAfterHost > 0 && colonAfterHost < firstSlash) {
            host = url.substring(prefixLen, colonAfterHost);
            port = url.substring(colonAfterHost + 1, firstSlash).toInt();
        } else {
            host = url.substring(prefixLen, firstSlash);
            port = isHttps ? HTTPS_DEFAULT_PORT : HTTP_DEFAULT_PORT;
        }
        path = (firstSlash < (int)url.length()) ? url.substring(firstSlash) : String("/");

        WiFiClient plainClient;
        WiFiSSLClient sslClient;
        Client* client = isHttps ? (Client*)&sslClient : (Client*)&plainClient;

        Serial.print("[Network] Connecting to ");
        Serial.print(host);
        Serial.print(":");
        Serial.println(port);

        if (!client->connect(host.c_str(), port)) {
            _lastResponseBody = "Connection failed";
            Serial.println("[Network] Connection failed");
            return -1;
        }

        client->print("POST "); client->print(path); client->println(" HTTP/1.1");
        client->print("Host: "); client->println(host);
        client->print("Content-Type: "); client->println(contentType);
        client->print("Content-Length: "); client->println(strlen(payload));
        client->println("Connection: close");
        client->println();
        client->print(payload);

        int statusCode = -1;
        bool isReadingBody = false;
        _lastResponseBody = "";

        unsigned long timeout = millis() + NETWORK_RESPONSE_TIMEOUT_MS;
        while ((client->connected() || client->available()) && millis() < timeout) {
            if (client->available()) {
                String line = client->readStringUntil('\n');
                if (statusCode < 0 && line.startsWith("HTTP/")) {
                    statusCode = line.substring(HTTP_STATUS_CODE_OFFSET, line.indexOf(' ', HTTP_STATUS_CODE_OFFSET)).toInt();
                } else if (line == "\r") {
                    isReadingBody = true;
                } else if (isReadingBody) {
                    _lastResponseBody += line;
                }
            }
        }

        client->stop();
        return statusCode;
    }

    const char* getResponseBody() const override {
        return _lastResponseBody.c_str();
    }
};