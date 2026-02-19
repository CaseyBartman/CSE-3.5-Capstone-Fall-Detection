# Usage Guide - Patient Fall Detection System

This guide explains how to use and extend the fall detection system.

## Quick Start

### 1. Running in Wokwi Simulator

The easiest way to test the system is using the standalone Arduino sketch with Wokwi:

1. Open the project in VS Code
2. Install the Wokwi extension if not already installed
3. Press F1 and select "Wokwi: Start Simulator"
4. **Important:** Open the Serial Monitor in Wokwi to see the logs
5. The simulator will show:
   - ESP32 board
   - Slide potentiometer (force sensor)
   - Push button (nurse button)

**The sketch compiles automatically in Wokwi - no build step needed!**

### 2. Interacting with the Simulation

**Force Sensor (Potentiometer):**
- Slide UP: High pressure (patient sitting/lying)
- Slide DOWN: Low pressure (patient stands up)

**Nurse Button:**
- Quick click: Short press
  - In ALARM state: Clears the alarm
  - In INPUT_PAUSED state: Starts calibration
- Hold for 2+ seconds: Long press
  - In POLLING state: Pauses monitoring for 2 minutes

**Serial Monitor:**
- All state transitions and events are logged
- Look for messages like "Fall detected!" and "ALARM!"
- Instructions are printed at startup

### 3. Testing Fall Detection

Follow these steps to test a complete fall scenario:

1. Start the simulation - System enters IDLE then POLLING state
2. Move potentiometer UP (>15%) - Patient sits down
3. Wait 2 seconds - System recognizes occupied state
4. Quickly move potentiometer DOWN (<5%) - Simulates fall/stand-up
5. **Check Serial Monitor** - Observe ALARM state and alert messages
6. Click button briefly - Clears alarm and returns to POLLING

## System States Explained

### SYSTEM_OFF
- Initial state before power on
- No monitoring active

### IDLE
- System is powered but not yet monitoring
- Waiting for WiFi and sensor readiness
- Automatically transitions to POLLING when ready

### POLLING
- **Active monitoring state**
- Continuously samples force sensor
- Detects when patient stands up (fall detection)
- Can be interrupted by:
  - Long button press → INPUT_PAUSED
  - Fall detection → ALARM

### INPUT_PAUSED
- Monitoring temporarily suspended for 2 minutes
- Useful when nurse needs to adjust patient or bed
- Can transition to:
  - CALIBRATION (short button press)
  - POLLING (after 2 minute timeout)

### CALIBRATION
- Samples sensor for 5 seconds to establish new baseline
- Useful after bed adjustments or patient weight changes
- Automatically returns to POLLING after 5 seconds

### ALARM
- Fall detected - alert sent to Connexxall
- Waits for nurse acknowledgment
- Short button press → POLLING (clears alarm)

## Serial Monitor Output

The system provides detailed logging:

```
========================================
Patient Fall Detection System
========================================
Running in SIMULATION mode (Wokwi)
WokwiPotentiometer initialized on pin 34
WokwiButton initialized on pin 15
SerialConsoleAlert initialized
Initializing Fall Detection System...
System Armed - Ready for operation
State Transition: SYSTEM_OFF -> IDLE
System in IDLE state - Waiting for readiness
State Transition: IDLE -> POLLING
System Armed - Monitoring for falls
========================================
Setup complete - Entering main loop
========================================
Long press detected
State Transition: POLLING -> INPUT_PAUSED
System Paused - Input monitoring suspended for 2 minutes
Fall detected! Pressure dropped significantly.
State Transition: POLLING -> ALARM
========================================
!!!!! FALL ALARM TRIGGERED !!!!!
Alert sent to Connexxall (Simulated)
========================================
```

## Customizing the System

### Adjusting Thresholds

Edit `include/constants/SystemConstants.h`:

```cpp
// Adjust occupation threshold (default 15%)
const float DEFAULT_PRESSURE_THRESHOLD = 20.0;

// Adjust fall detection sensitivity (default 5%)
const float FALL_DETECTION_THRESHOLD = 8.0;

// Adjust pause duration (default 2 minutes)
const unsigned long PAUSE_DURATION_MS = 180000; // 3 minutes

// Adjust calibration time (default 5 seconds)
const unsigned long CALIB_DURATION_MS = 10000; // 10 seconds
```

### Adding New Alert Methods

To add a new alert system (e.g., SMS, Email):

1. Create a new class implementing `IAlertSystem`:

```cpp
// src/drivers/real/SMSAlert.cpp
#include "interfaces/IAlertSystem.h"

class SMSAlert : public IAlertSystem {
public:
    void init() override {
        // Initialize SMS service
    }
    
    void triggerFallAlarm() override {
        // Send SMS alert
    }
    
    void clearAlarm() override {
        // Send SMS "alarm cleared"
    }
};
```

2. Update `main.cpp` to inject the new alert system:

```cpp
#ifndef IS_SIMULATION
    auto* alert = new SMSAlert();
#endif
```

### Adding New Sensors

To add additional sensors (e.g., heart rate monitor):

1. Create a new interface in `include/interfaces/`:

```cpp
// include/interfaces/IHeartRateMonitor.h
class IHeartRateMonitor {
public:
    virtual void init() = 0;
    virtual int getHeartRate() = 0;
    virtual ~IHeartRateMonitor() {}
};
```

2. Inject into FallDetector constructor
3. Use in state machine logic

## Production Deployment

### WiFi Configuration

Edit `include/constants/NetworkConstants.h`:

```cpp
const char* WIFI_SSID = "YourNetworkName";
const char* WIFI_PASSWORD = "YourPassword";
const char* CONNEXXALL_API_ENDPOINT = "https://your-instance.connexxall.com/api/alerts";
const char* CONNEXXALL_API_KEY = "your-api-key-here";
```

### Building for Hardware

```bash
# Build for ESP32
pio run -e production-esp32

# Upload to device
pio run -e production-esp32 -t upload

# Monitor output
pio device monitor
```

### Hardware Connections

| Component | ESP32 Pin | Notes |
|-----------|-----------|-------|
| Tekscan A502 Force Sensor | Pin 34 (ADC1_CH6) | 0-3.3V analog |
| Blue Charm BC B2 Button | BLE | No physical connection needed |
| Status LED (optional) | Pin 2 | Built-in LED |

## Troubleshooting

### Problem: Alarm triggers too easily
**Solution:** Increase `FALL_DETECTION_THRESHOLD` in SystemConstants.h

### Problem: System doesn't detect falls
**Solution:** Decrease `FALL_DETECTION_THRESHOLD` or check sensor calibration

### Problem: Button presses not detected
**Solution:** 
- Verify button connections (Pin 15 with pullup)
- Check `LONG_PRESS_MS` timing
- Ensure `update()` is called frequently in loop

### Problem: WiFi not connecting (production)
**Solution:**
- Verify credentials in NetworkConstants.h
- Check WiFi signal strength
- Add connection retry logic in ConnexxallWiFi.cpp

## Testing Checklist

Before deploying to production, test:

- [ ] Normal operation (no false alarms during normal use)
- [ ] Fall detection (triggers when patient stands quickly)
- [ ] Button functionality (short and long press)
- [ ] Pause feature (2 minute timeout)
- [ ] Calibration sequence
- [ ] Alarm acknowledgment
- [ ] WiFi connection stability
- [ ] Alert delivery to Connexxall
- [ ] Power cycle recovery
- [ ] Battery backup (if applicable)

## Architecture Benefits

This dependency injection architecture provides:

✅ **Testability** - Mock components for unit testing  
✅ **Flexibility** - Swap implementations without changing logic  
✅ **Simulation** - Develop without hardware  
✅ **Maintainability** - Clear separation of concerns  
✅ **Scalability** - Easy to add new features  

## Next Steps

- Implement real Tekscan A502 driver with actual sensor protocol
- Add BLE communication with Blue Charm button
- Integrate with Connexxall API (HTTP POST/DELETE)
- Add data logging for analytics
- Implement battery monitoring
- Add OTA (Over-The-Air) firmware updates
- Create mobile app for configuration

## Support

For questions or issues:
- Review the README.md
- Check inline code documentation
- Examine test cases in test/ directory
- Review state transition table in problem statement
