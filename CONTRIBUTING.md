# Contributing to Fall Detection System

Thank you for contributing to the Patient Fall Detection System! This guide will help you understand how to work with the codebase effectively.

## Code Organization

Our architecture follows **Clean Code** and **SOLID** principles with dependency injection.

### Layer Responsibilities

```
┌─────────────────────────────────────┐
│         Main (Composition)          │  ← Wires everything together
├─────────────────────────────────────┤
│      Logic (FallDetector)           │  ← State machine, business rules
├─────────────────────────────────────┤
│   Interfaces (IForceSensor, etc)    │  ← Contracts/Abstractions
├─────────────────────────────────────┤
│  Drivers (Sim & Real Hardware)      │  ← Hardware implementations
└─────────────────────────────────────┘
```

### Key Principles

1. **Dependency Injection**: Logic never directly creates hardware instances
2. **Interface Segregation**: Small, focused interfaces
3. **Single Responsibility**: Each class has one clear purpose
4. **Open/Closed**: Extend via new implementations, don't modify logic
5. **No Magic Numbers**: All constants in Constants.h files

## Making Changes

### Adding a New Sensor

1. **Create Interface** (`include/interfaces/INewSensor.h`):
```cpp
#pragma once

class INewSensor {
public:
    virtual void init() = 0;
    virtual float getValue() = 0;
    virtual ~INewSensor() {}
};
```

2. **Create Simulation Driver** (`src/drivers/sim/WokwiNewSensor.cpp`):
```cpp
#include "interfaces/INewSensor.h"
#include <Arduino.h>

class WokwiNewSensor : public INewSensor {
private:
    const int _pin;

public:
    WokwiNewSensor(int pin) : _pin(pin) {}
    
    void init() override {
        pinMode(_pin, INPUT);
    }
    
    float getValue() override {
        return analogRead(_pin) / 40.95f;
    }
};
```

3. **Create Real Driver Stub** (`src/drivers/real/RealNewSensor.cpp`):
```cpp
#include "interfaces/INewSensor.h"

class RealNewSensor : public INewSensor {
public:
    void init() override {
        // TODO: Implement real hardware initialization
    }
    
    float getValue() override {
        // TODO: Implement real hardware reading
        return 0.0f;
    }
};
```

4. **Update FallDetector** to accept new sensor via constructor

5. **Update main.cpp** to inject the sensor

### Modifying State Machine Logic

All state machine logic is in `src/logic/FallDetector.cpp`.

**Follow the Stepdown Rule**: High-level logic at top, details below.

```cpp
void FallDetector::update() {
    _button->update();
    
    switch (_currentState) {
        case SystemState::POLLING:
            handlePollingState();  // ← Call high-level handler
            break;
        // ...
    }
}

void FallDetector::handlePollingState() {
    // Implementation details here
}
```

### Adding New Constants

Add to `include/constants/SystemConstants.h`:
```cpp
const unsigned long NEW_TIMEOUT_MS = 30000;  // 30 seconds
const float NEW_THRESHOLD = 25.0f;
```

Never use magic numbers directly in logic code!

## Testing Strategy

### 1. Simulation Testing (Wokwi)
- Fast iteration
- No hardware needed
- Test basic logic flow

### 2. Unit Testing (Future)
- Use mock objects from `test/helpers/MockSensors.h`
- Test individual state transitions
- Test edge cases

### 3. Hardware Testing
- Final validation
- Real-world conditions
- Integration testing

## Code Style

### Naming Conventions

- **Classes**: `PascalCase` (e.g., `FallDetector`)
- **Interfaces**: `IPascalCase` (e.g., `IForceSensor`)
- **Private members**: `_camelCase` with underscore prefix
- **Public methods**: `camelCase`
- **Constants**: `SCREAMING_SNAKE_CASE`

### Comments

Write comments that explain **WHY**, not what:

```cpp
// ❌ Bad: Obvious what it does
// Check if pressure is greater than threshold
if (pressure > threshold) { ... }

// ✅ Good: Explains reasoning
// Only trigger alarm if previously occupied to avoid false positives
if (_wasOccupied && pressure < FALL_DETECTION_THRESHOLD) { ... }
```

### File Headers

Keep interface headers clean and focused:

```cpp
#pragma once  // ✅ Use pragma once, not include guards

class IForceSensor {
public:
    virtual void init() = 0;
    virtual float getPressurePercentage() = 0;
    virtual ~IForceSensor() {}
};
```

## Git Workflow

### Branch Naming
- Feature: `feature/add-heart-rate-monitor`
- Bugfix: `fix/alarm-false-trigger`
- Refactor: `refactor/extract-wifi-logic`

### Commit Messages

Follow conventional commits:

```
feat: add heart rate monitoring interface
fix: prevent false alarms during calibration
refactor: extract WiFi connection logic
docs: update usage guide with new features
test: add unit tests for fall detection
```

### Pull Request Checklist

- [ ] Code compiles for both `wokwi-sim` and `production-esp32`
- [ ] Tested in Wokwi simulator
- [ ] Added/updated documentation
- [ ] Followed naming conventions
- [ ] No magic numbers - all in Constants.h
- [ ] Used dependency injection
- [ ] Serial logging added for debugging
- [ ] Updated README if adding new features

## Common Patterns

### Adding Serial Logging

```cpp
void FallDetector::handlePollingState() {
    Serial.print("Current pressure: ");
    Serial.print(_sensor->getPressurePercentage());
    Serial.println("%");
    
    // ... rest of logic
}
```

### State Transition Pattern

Always use `transitionToState()` - don't set state directly:

```cpp
// ❌ Bad
_currentState = SystemState::ALARM;

// ✅ Good
transitionToState(SystemState::ALARM);
```

This ensures logging and entry actions are executed.

### Constructor Injection Pattern

```cpp
// Header
class FallDetector {
private:
    IForceSensor* _sensor;  // Pointer to interface
    
public:
    FallDetector(IForceSensor* sensor);  // Inject in constructor
};

// Implementation
FallDetector::FallDetector(IForceSensor* sensor) 
    : _sensor(sensor) {
}
```

## Debugging Tips

### Serial Monitoring

```bash
# Monitor with filter
pio device monitor --filter=colorize

# Save log to file
pio device monitor > debug.log
```

### Common Issues

**Problem**: State transitions not logging  
**Solution**: Check that you're using `transitionToState()` not direct assignment

**Problem**: Sensor readings seem wrong  
**Solution**: Add debug prints in driver's `getPressurePercentage()`

**Problem**: Button presses not detected  
**Solution**: Verify `update()` is called every loop iteration

## Resources

- [PlatformIO Documentation](https://docs.platformio.org/)
- [Wokwi Simulator](https://wokwi.com/)
- [ESP32 Arduino Core](https://docs.espressif.com/projects/arduino-esp32/)
- [Clean Code Principles](https://www.amazon.com/Clean-Code-Handbook-Software-Craftsmanship/dp/0132350882)
- [SOLID Principles](https://en.wikipedia.org/wiki/SOLID)

## Questions?

If you're unsure about:
- **Architecture decisions**: Review the problem statement and this guide
- **Implementation details**: Check existing similar implementations
- **Testing**: See examples in `test/` directory
- **Hardware specifics**: Consult datasheets in project documentation

## License

This project is part of CSE 3.5 Capstone course.
