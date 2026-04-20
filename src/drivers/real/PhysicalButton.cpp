#include <Arduino.h>
#include "PhysicalButton.h"

PhysicalButton::PhysicalButton(int pin, INetworkClient* networkClient, const char* endpointUrl)
    : _pin(pin), _lastState(HIGH), _pressedTime(0),
      _shortPressFlag(false), _injectedPressFlag(false),
      _networkClient(networkClient), _endpointUrl(endpointUrl),
      _lastSignalSentTime(0), _isCurrentlyPressed(false) {}

void PhysicalButton::init() {
    pinMode(_pin, INPUT_PULLUP);
    Serial.print("PhysicalButton (ESP32 Joystick) initialized on pin ");
    Serial.println(_pin);
    
    if (_networkClient) {
        Serial.println("Network communication enabled for button signals");
    } else {
        Serial.println("WARNING: No network client provided for button signals");
    }
}

void PhysicalButton::sendButtonSignal(bool isPressed) {
    if (!_networkClient || !_endpointUrl) {
        return;
    }
    
    if (!_networkClient->isNetworkConnected()) {
        Serial.println("ERROR: Network unavailable for button signal transmission");
        return;
    }
    
    // Create payload: button state as plain text or JSON
    const char* payload = isPressed ? "BUTTON_PRESSED" : "BUTTON_RELEASED";
    const char* contentType = "text/plain";
    
    int responseStatus = _networkClient->post(_endpointUrl, contentType, payload);
    
    Serial.print("Button signal sent (state: ");
    Serial.print(isPressed ? "PRESSED" : "RELEASED");
    Serial.print("). Response: ");
    Serial.println(responseStatus);
    
    if (responseStatus < 0 || responseStatus >= 400) {
        Serial.print("ERROR: Button signal transmission failed. Response: ");
        Serial.println(_networkClient->getResponseBody());
    }
}

void PhysicalButton::update() {
    bool currentState = digitalRead(_pin);
    unsigned long currentTime = millis();

    // Detect button press/release edges
    if (_lastState == HIGH && currentState == LOW) {
        _pressedTime = currentTime;
        _isCurrentlyPressed = true;
        // Send immediate press signal
        sendButtonSignal(true);
        _lastSignalSentTime = currentTime;
    }
    else if (_lastState == LOW && currentState == HIGH) {
        unsigned long duration = currentTime - _pressedTime;
        if (duration >= DEBOUNCE_DELAY_MS) _shortPressFlag = true;
        
        _isCurrentlyPressed = false;
        // Send release signal
        sendButtonSignal(false);
        _lastSignalSentTime = currentTime;
    }
    // Send periodic update while button is held down
    else if (_isCurrentlyPressed && (currentTime - _lastSignalSentTime >= SIGNAL_SEND_INTERVAL_MS)) {
        sendButtonSignal(true);
        _lastSignalSentTime = currentTime;
    }

    _lastState = currentState;
}

bool PhysicalButton::wasShortPressed() {
    if (_shortPressFlag || _injectedPressFlag) {
        _shortPressFlag = false;
        _injectedPressFlag = false;
        return true;
    }
    return false;
}
bool PhysicalButton::isPressed() { return digitalRead(_pin) == LOW; }
bool PhysicalButton::wasLongPressed() { return false; }  // Long press not used

void PhysicalButton::injectShortPress() {
    Serial.println("[PhysicalButton] Injecting short press from ESP32 signal");
    _injectedPressFlag = true;
}

void PhysicalButton::handleHttpClient(WiFiClient client) {
    Serial.println("[PhysicalButton] ESP32 client connected");

    // Wait until data is available
    while (!client.available()) {
        delay(1);
    }

    // Read the request line
    String request = client.readStringUntil('\n');
    Serial.println("[PhysicalButton] Request: " + request);

    // Drain remaining headers
    while (client.available()) {
        client.read();
    }

    // Inject a short press if this is a /trigger request
    if (request.indexOf("GET /trigger") != -1) {
        injectShortPress();
    }

    // Send HTTP response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println("UNO OK");

    delay(1);
    client.stop();
    Serial.println("[PhysicalButton] ESP32 client disconnected");
}