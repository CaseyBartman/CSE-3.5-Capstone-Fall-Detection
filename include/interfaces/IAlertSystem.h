#pragma once

class IAlertSystem {
public:
    virtual void init() = 0;
    virtual void triggerFallAlarm() = 0;
    virtual void clearAlarm() = 0;
    virtual ~IAlertSystem() {}
};
