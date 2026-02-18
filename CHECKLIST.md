# Development Checklist

Use this checklist to track completion of the fall detection system.

## ✅ Phase 1: Architecture & Simulation (COMPLETE)

- [x] Create dependency injection architecture
- [x] Implement all interfaces (IForceSensor, INurseInput, IAlertSystem)
- [x] Implement complete state machine with all 6 states
- [x] Create simulation drivers for Wokwi
- [x] Configure PlatformIO for simulation and production builds
- [x] Create Wokwi circuit diagram
- [x] Write comprehensive documentation
- [x] Create test infrastructure (mocks, generators)

## 🔄 Phase 2: Simulation Testing (IN PROGRESS)

- [ ] Test in Wokwi simulator
  - [ ] Verify system boot sequence (SYSTEM_OFF → IDLE → POLLING)
  - [ ] Test fall detection (move potentiometer high then low)
  - [ ] Test short button press (clear alarm, start calibration)
  - [ ] Test long button press (pause for 2 minutes)
  - [ ] Verify pause timeout (2 minutes)
  - [ ] Verify calibration timeout (5 seconds)
  - [ ] Test all state transitions
  - [ ] Verify serial logging output

- [ ] Document any issues found in simulation
- [ ] Adjust thresholds if needed based on testing

## 📋 Phase 3: Hardware Driver Implementation (TODO)

### Force Sensor Driver (TekscanA502.cpp)

- [ ] Research Tekscan A502 communication protocol
- [ ] Determine if using I2C, SPI, or analog
- [ ] Implement proper initialization sequence
- [ ] Implement pressure reading with correct scaling
- [ ] Add calibration routine for zero offset
- [ ] Test with real hardware
- [ ] Document pin connections and configuration

### BLE Button Driver (BlueCharmBLE.cpp)

- [ ] Choose BLE library (NimBLE-Arduino recommended)
- [ ] Add library to platformio.ini
- [ ] Implement BLE scanning and connection
- [ ] Subscribe to button press characteristic
- [ ] Implement button press event handling
- [ ] Add connection status monitoring
- [ ] Add reconnection logic
- [ ] Test with real Blue Charm BC B2 button
- [ ] Document MAC address configuration

### Alert System Driver (ConnexxallWiFi.cpp)

- [ ] Add WiFi library to platformio.ini
- [ ] Add ArduinoJson library for API payloads
- [ ] Implement WiFi connection with retry logic
- [ ] Implement HTTP client setup
- [ ] Create JSON payload for fall alerts
- [ ] Implement POST request to trigger alarm
- [ ] Implement DELETE/PUT request to clear alarm
- [ ] Add error handling and retry logic
- [ ] Test with real Connexxall API
- [ ] Document API endpoint and credentials setup

## 🧪 Phase 4: Unit Testing (TODO)

- [ ] Set up native test environment in platformio.ini
- [ ] Implement test cases using Unity framework
  - [ ] Test state transitions
  - [ ] Test fall detection logic
  - [ ] Test button handling
  - [ ] Test timing (pause, calibration)
  - [ ] Test edge cases
- [ ] Achieve >80% code coverage
- [ ] Document test results

## 🔌 Phase 5: Hardware Integration (TODO)

### Hardware Setup

- [ ] Procure all hardware components
  - [ ] ESP32 development board
  - [ ] Tekscan A502 force sensor
  - [ ] Blue Charm BC B2 BLE button
  - [ ] Power supply
  - [ ] Enclosure

- [ ] Wire up the system
  - [ ] Connect force sensor to ESP32 pin 34
  - [ ] Power connections
  - [ ] Add status LED (optional)

### Configuration

- [ ] Set WiFi credentials in NetworkConstants.h
- [ ] Set Connexxall API endpoint and key
- [ ] Set Blue Charm MAC address
- [ ] Adjust thresholds if needed for real sensor

### Integration Testing

- [ ] Flash firmware to ESP32
- [ ] Verify WiFi connection
- [ ] Test force sensor readings
- [ ] Test BLE button connection
- [ ] Test fall detection with real person
- [ ] Test alert delivery to Connexxall
- [ ] Test full scenario end-to-end
- [ ] Verify battery life (if battery powered)

## 🚀 Phase 6: Production Deployment (TODO)

- [ ] Final testing in target environment
- [ ] Create installation guide
- [ ] Create user manual for nurses
- [ ] Set up monitoring/logging
- [ ] Create maintenance plan
- [ ] Plan for OTA updates (if needed)
- [ ] Train staff on system usage
- [ ] Deploy to production

## 🔧 Phase 7: Enhancements (FUTURE)

- [ ] Add data logging to SD card or cloud
- [ ] Implement analytics dashboard
- [ ] Add battery monitoring
- [ ] Add OTA firmware updates
- [ ] Create mobile configuration app
- [ ] Add multiple sensor support
- [ ] Implement sensor fusion (if adding accelerometer, etc.)
- [ ] Add machine learning for improved fall detection

## 📝 Notes Section

### Issues Found
<!-- Document any issues discovered during testing -->

### Threshold Adjustments
<!-- Document any changes made to thresholds -->
- Default pressure threshold: 15%
- Fall detection threshold: 5%
- Pause duration: 2 minutes
- Calibration duration: 5 seconds

### Hardware Notes
<!-- Document hardware-specific information -->

### API Configuration
<!-- Document Connexxall API details -->

---

## Quick Reference

### Build Commands
```bash
# Simulation
pio run -e wokwi-sim

# Production
pio run -e production-esp32
pio run -e production-esp32 -t upload
pio device monitor
```

### Pin Assignments
- Force Sensor: Pin 34 (ADC1_CH6)
- Nurse Button: Via BLE (no physical pin)
- Status LED: Pin 2 (built-in)

### Key Files
- State machine logic: `src/logic/FallDetector.cpp`
- Simulation drivers: `src/drivers/sim/`
- Production drivers: `src/drivers/real/`
- Configuration: `include/constants/`
