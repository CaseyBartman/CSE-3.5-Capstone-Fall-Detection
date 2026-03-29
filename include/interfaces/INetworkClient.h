#pragma once
#include <string>

class INetworkClient {
public:
    virtual ~INetworkClient() = default;
    virtual bool isNetworkConnected() const = 0;
    virtual int post(const char* endpointUrl, const char* contentType, const char* payload) = 0;
    virtual const char* getResponseBody() const = 0; // For logging
};
