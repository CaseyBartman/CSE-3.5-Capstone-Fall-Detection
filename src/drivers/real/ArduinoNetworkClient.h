#pragma once
#include "interfaces/INetworkClient.h"
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
        int prefixLen = isHttps ? 8 : 7;  // "https://" vs "http://"

        // Parse host, port, path
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
            port = isHttps ? 443 : 80;
        }
        path = (firstSlash < (int)url.length()) ? url.substring(firstSlash) : String("/");

        // Choose SSL or plain client based on scheme
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

        // Send HTTP POST
        client->print("POST "); client->print(path); client->println(" HTTP/1.1");
        client->print("Host: "); client->println(host);
        client->print("Content-Type: "); client->println(contentType);
        client->print("Content-Length: "); client->println(strlen(payload));
        client->println("Connection: close");
        client->println();
        client->print(payload);

        // Read response with timeout
        int statusCode = -1;
        bool readingBody = false;
        _lastResponseBody = "";

        unsigned long timeout = millis() + 8000;
        while ((client->connected() || client->available()) && millis() < timeout) {
            if (client->available()) {
                String line = client->readStringUntil('\n');
                if (statusCode < 0 && line.startsWith("HTTP/")) {
                    statusCode = line.substring(9, line.indexOf(' ', 9)).toInt();
                } else if (line == "\r") {
                    readingBody = true;
                } else if (readingBody) {
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