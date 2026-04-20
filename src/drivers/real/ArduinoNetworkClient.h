#pragma once
#include "interfaces/INetworkClient.h"
#include <WiFiS3.h>

class ArduinoNetworkClient : public INetworkClient {
private:
    String _lastResponseBody; // Use Arduino String to hold response body

public:
    bool isNetworkConnected() const override {
        return WiFi.status() == WL_CONNECTED;
    }

    int post(const char* endpointUrl, const char* contentType, const char* payload) override {
        // Parse URL - for simplicity, assume it's http://ip:port/path
        String url = endpointUrl;
        int colonIndex = url.indexOf(':', 7); // After http://
        int slashIndex = url.indexOf('/', colonIndex);
        
        String host = url.substring(7, colonIndex); // Remove http://
        int port = url.substring(colonIndex + 1, slashIndex).toInt();
        String path = url.substring(slashIndex);
        
        WiFiClient client;
        if (client.connect(host.c_str(), port)) {
            // Send HTTP POST request
            client.print("POST ");
            client.print(path);
            client.println(" HTTP/1.1");
            client.print("Host: ");
            client.println(host);
            client.print("Content-Type: ");
            client.println(contentType);
            client.print("Content-Length: ");
            client.println(strlen(payload));
            client.println("Connection: close");
            client.println();
            client.println(payload);
            
            // Read response
            String response = "";
            bool readingBody = false;
            int statusCode = -1;
            
            while (client.connected() || client.available()) {
                if (client.available()) {
                    String line = client.readStringUntil('\n');
                    if (line.startsWith("HTTP/1.1 ")) {
                        // Parse status code
                        int spaceIndex = line.indexOf(' ', 9);
                        statusCode = line.substring(9, spaceIndex).toInt();
                    } else if (line == "\r") {
                        readingBody = true;
                    } else if (readingBody) {
                        response += line;
                    }
                }
            }
            
            client.stop();
            _lastResponseBody = response;
            return statusCode;
        } else {
            _lastResponseBody = "Connection failed";
            return -1;
        }
    }

    const char* getResponseBody() const override {
        return _lastResponseBody.c_str();
    }
};