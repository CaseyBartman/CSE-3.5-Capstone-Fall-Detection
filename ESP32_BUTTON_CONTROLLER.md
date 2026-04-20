# ESP32 Button Controller - Documentation

## Overview

This system allows an **ESP32** to read a joystick button and communicate button press signals to an **Arduino Uno R4 WiFi** board via HTTP over WiFi. The communication happens regularly (every 500ms while pressed) to ensure timely signal delivery.

## Architecture

```
ESP32 (Button Controller)
├── Joystick Button Input (GPIO pin)
├── WiFi Connection
└── HTTP POST Requests to Arduino Uno R4
    └── Arduino Uno R4 WiFi (Tekscan Sensor Side)
        ├── Receives HTTP Button Signals
        ├── Tekscan A502 Force Sensor
        └── Alert System
```

## Hardware Setup

### ESP32 Side (Button Controller)
- **ESP32-WROOM-32** development board
- **Joystick button** connected to GPIO pin (default: GPIO36)
  - Joystick GND → ESP32 GND
  - Joystick Button → ESP32 GPIO36 (or specified pin)
  - Use internal pullup (configured in code)
- WiFi connectivity (built-in)

### Arduino Uno R4 WiFi Side (Receiver)
- **Arduino Uno R4 WiFi** board
- **Tekscan A502** force sensor (existing setup)
- HTTP server listening on port 8080 (default)
- Static IP address (or mDNS hostname)

## Software Changes

### Modified Files

1. **`include/constants/NetworkConstants.h`**
   - Added: `BUTTON_SIGNAL_ENDPOINT` - HTTP endpoint URL for Arduino Uno R4

2. **`src/drivers/real/PhysicalButton.h`**
   - Now accepts `INetworkClient*` for HTTP communication
   - Optionally accepts endpoint URL
   - Added periodic signal sending (500ms intervals)
   - Sends signals: `"BUTTON_PRESSED"` or `"BUTTON_RELEASED"`

3. **`src/drivers/real/PhysicalButton.cpp`**
   - Implemented `sendButtonSignal()` method for HTTP POST
   - Sends immediate signal on press/release
   - Sends periodic updates every 500ms while held
   - Error handling and logging for network issues

4. **`platformio.ini`**
   - Added `[env:esp32_button_controller]` environment
   - Configured for ESP32-WROOM-32 board
   - Includes WiFi and HTTPClient libraries
   - Build filter to exclude unnecessary files

### New Files

1. **`src/main_esp32_button.cpp`**
   - Example main program for ESP32 button controller
   - Handles WiFi setup and connection
   - Creates and updates PhysicalButton instance

2. **`doc_BUTTON_RECEIVER_EXAMPLE.cpp`**
   - Example HTTP endpoint handler for Arduino Uno R4
   - Shows how to receive and parse button signals
   - Can be integrated into existing Arduino sketch

## Configuration

### Step 1: Update Network Settings

Edit `include/constants/NetworkConstants.h`:
```cpp
// ESP32 Button Controller -> Arduino Uno R4 Endpoint
const char* BUTTON_SIGNAL_ENDPOINT = "http://192.168.1.100:8080/button";  
// ^^^ Update 192.168.1.100 with Arduino Uno R4's actual IP address
```

### Step 2: Configure WiFi Credentials

Edit `src/main_esp32_button.cpp`:
```cpp
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
```

### Step 3: Adjust GPIO Pin (Optional)

If using a different GPIO pin than GPIO36:
```cpp
const int JOYSTICK_BUTTON_PIN = 36;  // Change to your pin
```

## Compilation

### Build for ESP32 Button Controller

```bash
# Using PlatformIO
platformio run -e esp32_button_controller

# Or build and upload directly
platformio run -e esp32_button_controller -t upload
```

### Build for Arduino Uno R4 WiFi (Sensor Side)

```bash
# Existing build (unchanged)
platformio run -e uno_r4_wifi
```

## HTTP Protocol

### Button Signal Format

**Endpoint:** `POST /button`

**Headers:**
```
Content-Type: text/plain
```

**Payload:**
- `BUTTON_PRESSED` - when button pressed
- `BUTTON_RELEASED` - when button released

**Frequency:**
- Immediate signal on button press/release
- Every 500ms while button is held down (to ensure delivery)

**Example Request:**
```http
POST /button HTTP/1.1
Host: 192.168.1.100:8080
Content-Type: text/plain
Content-Length: 14

BUTTON_PRESSED
```

## Arduino Uno R4 Integration

To integrate the button signal receiver into your existing Arduino sketch:

1. Add WiFi server setup:
```cpp
#include <WiFiS3.h>
WiFiServer buttonServer(8080);  // In setup()
buttonServer.begin();
```

2. In your main loop, add:
```cpp
WiFiClient client = buttonServer.available();
if (client) {
    handleButtonSignalRequest(client);
}
```

3. Implement the handler:
```cpp
void handleButtonSignalRequest(WiFiClient client) {
    // Read request and process button signal
    // See doc_BUTTON_RECEIVER_EXAMPLE.cpp for full implementation
}
```

## Debugging

### Serial Output from ESP32

The ESP32 logs:
- WiFi connection status
- Button press/release events
- HTTP request status codes
- Network errors

Monitor with:
```bash
platformio device monitor -e esp32_button_controller
```

### Common Issues

| Issue | Solution |
|-------|----------|
| WiFi won't connect | Verify SSID/password, check WiFi signal range |
| HTTP requests fail | Verify Arduino IP address, ensure Arduino is running receiver |
| Button press not detected | Check GPIO pin number, verify button wiring |
| Slow button response | Normal 500ms update interval; network latency may add 100-200ms |

## API Reference

### PhysicalButton Class

```cpp
// Constructor - for ESP32 with network communication
PhysicalButton(int pin, INetworkClient* networkClient, const char* endpointUrl);

// Initialize button (configure GPIO pin)
void init();

// Update button state (must be called in loop)
void update();

// Check current button state (true = pressed)
bool isPressed();

// Check if short press occurred (debounced)
bool wasShortPressed();

// Check if long press occurred (>2 seconds)
bool wasLongPressed();
```

## Troubleshooting Network Communication

1. **Verify Arduino IP Address:**
   ```cpp
   // On Arduino Uno R4, print IP in setup
   Serial.println(WiFi.localIP());
   ```

2. **Test Endpoint with curl (on same WiFi network):**
   ```bash
   curl -X POST -H "Content-Type: text/plain" \
        -d "BUTTON_PRESSED" \
        http://192.168.1.100:8080/button
   ```

3. **Check Firewall:**
   - Ensure port 8080 is not blocked on Arduino
   - Check WiFi router for network isolation settings

## Future Enhancements

- Add JSON payload support (currently text/plain)
- Increase update frequency if network allows
- Add ack/feedback mechanism for reliability
- Implement connection retry logic
- Add button debouncing over network

## References

- [ESP32 Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [Arduino Uno R4 WiFi Documentation](https://docs.arduino.cc/hardware/uno-r4-wifi)
- [HTTPClient Library](https://github.com/espressif/arduino-esp32/blob/master/libraries/HTTPClient/src/HTTPClient.h)
