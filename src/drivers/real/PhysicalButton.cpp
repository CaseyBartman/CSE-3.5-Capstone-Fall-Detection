#include <Arduino.h>
#include "PhysicalButton.h"
#include "constants/NetworkConstants.h"

PhysicalButton::PhysicalButton(int pin)
    : _pin(pin), _lastState(HIGH), _pressedTime(0),
      _shortPressFlag(false), _injectedPressFlag(false) {}

void PhysicalButton::init() {
    pinMode(_pin, INPUT_PULLUP);
    Serial.print("PhysicalButton initialized on pin ");
    Serial.println(_pin);
}

void PhysicalButton::update() {
    bool isPinLow = digitalRead(_pin) == LOW;
    unsigned long currentTime = millis();

    if (isPinLow && _lastState == HIGH) {
        _pressedTime = currentTime;
    }
    else if (!isPinLow && _lastState == LOW) {
        unsigned long duration = currentTime - _pressedTime;
        if (duration >= DEBOUNCE_DELAY_MS) _shortPressFlag = true;
    }

    _lastState = isPinLow ? LOW : HIGH;
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
bool PhysicalButton::wasLongPressed() { return false; }

void PhysicalButton::injectShortPress() {
    Serial.println("[PhysicalButton] Injecting short press from ESP32 signal");
    _injectedPressFlag = true;
}

void PhysicalButton::handleHttpClient(WiFiClient client) {
    Serial.println("[PhysicalButton] ESP32 client connected");

    while (!client.available()) {
        delay(1);
    }

    String request = client.readStringUntil('\n');
    Serial.println("[PhysicalButton] Request: " + request);

    while (client.available()) {
        client.read();
    }

    String expectedTriggerLine = String("GET ") + ARDUINO_TRIGGER_PATH;
    if (request.indexOf(expectedTriggerLine) != -1) {
        injectShortPress();
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println("UNO OK");

    delay(1);
    client.stop();
    Serial.println("[PhysicalButton] ESP32 client disconnected");
}