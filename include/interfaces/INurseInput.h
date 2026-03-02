#pragma once

class INurseInput {
public:
    virtual void init() = 0;
    virtual bool isPressed() = 0;              // Returns true if button is currently pressed
    virtual bool wasShortPressed() = 0;        // Returns true if short press detected
    virtual bool wasLongPressed() = 0;         // Returns true if long press (hold) detected
    virtual void update() = 0;                 // Call in loop to process button state
    virtual ~INurseInput() {}
};
