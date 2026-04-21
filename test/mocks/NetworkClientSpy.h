#pragma once
#include "interfaces/INetworkClient.h"
#include <cstring> // For strlen, strcpy

struct CapturedHttpRequest {
    char* targetUrl;
    char* contentType;
    char* payload;

    // Helper to free memory for a single captured request since we dynamically allocated everything with char*
    void clear() {
        delete[] targetUrl;
        delete[] contentType;
        delete[] payload;
        targetUrl = nullptr;
        contentType = nullptr;
        payload = nullptr;
    }
};

class NetworkClientSpy : public INetworkClient {
private:
    bool _shouldSimulateConnection;
    int _simulatedResponseCode;
    char* _simulatedResponseBody;
    static const int MAX_CAPTURED_REQUESTS = 50;

    CapturedHttpRequest _capturedRequests[MAX_CAPTURED_REQUESTS];
    int _capturedCount;

    // Helper to deep-copy strings (Some weird C++ string behavior with char*)
    char* duplicateString(const char* source) {
        if (!source) return nullptr;
        size_t size = std::strlen(source) + 1;
        char* copy = new char[size];
        strcpy_s(copy, size, source);
        return copy;
    }

public:
    NetworkClientSpy() : 
        _shouldSimulateConnection(true), 
        _simulatedResponseCode(200), 
        _simulatedResponseBody(nullptr),
        _capturedCount(0) {}

    ~NetworkClientSpy() {
        delete[] _simulatedResponseBody;
        clearCapturedState();
    }

    // Test Control Methods
    void setSimulatedConnectionState(bool isConnected) { _shouldSimulateConnection = isConnected; }
    void setSimulatedResponseCode(int statusCode) { _simulatedResponseCode = statusCode; }
    
    void setSimulatedResponseBody(const char* body) { 
        delete[] _simulatedResponseBody;
        _simulatedResponseBody = duplicateString(body);
    }
    
    // Data Extraction for Assertions
    int getCapturedCount() const { return _capturedCount; }
    const CapturedHttpRequest* getCapturedRequests() const { return _capturedRequests; }

    void clearCapturedState() {
        for (int i = 0; i < _capturedCount; ++i) {
            _capturedRequests[i].clear();
        }
        _capturedCount = 0;
    }

    // Interface Implementation
    bool isNetworkConnected() const override {
        return _shouldSimulateConnection;
    }

    int post(const char* endpointUrl, const char* contentType, const char* payload) override {
        if (_capturedCount < MAX_CAPTURED_REQUESTS) {
            _capturedRequests[_capturedCount].targetUrl = duplicateString(endpointUrl);
            _capturedRequests[_capturedCount].contentType = duplicateString(contentType);
            _capturedRequests[_capturedCount].payload = duplicateString(payload);
            _capturedCount++;
        }
        return _simulatedResponseCode;
    }

    const char* getResponseBody() const override {
        return _simulatedResponseBody;
    }
};