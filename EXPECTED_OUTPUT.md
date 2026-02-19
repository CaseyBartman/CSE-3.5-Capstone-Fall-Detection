# Expected Serial Monitor Output

When you start the Wokwi simulator, you should see this output in the Serial Monitor:

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

INSTRUCTIONS:
- Move potentiometer UP (>15%) to simulate patient sitting
- Move potentiometer DOWN (<5%) to simulate fall/stand-up
- CLICK button briefly for short press
- HOLD button 2+ seconds for long press
========================================
```

## Example: Testing Fall Detection

When you move the potentiometer UP then DOWN, you'll see:

```
Fall detected! Pressure dropped significantly.
State Transition: POLLING -> ALARM
ALARM! Fall detected - Triggering alert
========================================
!!!!! FALL ALARM TRIGGERED !!!!!
Alert sent to Connexxall (Simulated)
========================================
```

## Example: Clearing Alarm

When you click the button (short press):

```
Short press detected
Nurse pressed button - Clearing alarm
========================================
Alarm cleared by nurse
Connexxall alert cleared (Simulated)
========================================
State Transition: ALARM -> POLLING
System Armed - Monitoring for falls
```

## Example: Pausing System

When you hold the button for 2+ seconds:

```
Long press detected
Nurse pressed hold button - Pausing input
State Transition: POLLING -> INPUT_PAUSED
System Paused - Input monitoring suspended for 2 minutes
```

## All State Transitions Logged

Every state change is logged with this format:
```
State Transition: [CURRENT_STATE] -> [NEW_STATE]
[Description of new state]
```

This makes it easy to debug and understand what the system is doing at all times.
