#pragma once
#include "interfaces/INurseInput.h"
#include "interfaces/INetworkClient.h"
#include "constants/SystemConstants.h"
#include <Arduino.h>

class PhysicalButton : public INurseInput {
private:
    int _pin;
    bool _lastState;
    unsigned long _pressedTime;
    bool _shortPressFlag;
    
    // Network communication for button sensor to Arduino Uno R4
    INetworkClient* _networkClient;
    const char* _endpointUrl;
    unsigned long _lastSignalSentTime;
    static const unsigned long SIGNAL_SEND_INTERVAL_MS = 500;  // Send status every 500ms while pressed
    bool _isCurrentlyPressed;

    void sendButtonSignal(bool isPressed);

public:
    PhysicalButton(int pin, INetworkClient* networkClient = nullptr, const char* endpointUrl = nullptr);

    void init() override;
    void update() override;

    bool isPressed() override;
    bool wasShortPressed() override;
    bool wasLongPressed() override;
};
