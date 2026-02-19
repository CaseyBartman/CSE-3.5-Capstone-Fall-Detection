# Wokwi Quick Start Guide

## Problem Solved

The Wokwi VS Code extension doesn't work well with PlatformIO builds. This guide shows how to use the standalone Arduino sketch instead.

## Solution: Use sketch.ino

We've created `sketch.ino` - a self-contained Arduino sketch that works directly with Wokwi without any build step.

## How to Use

### Step 1: Open in Wokwi

1. Open this project folder in VS Code
2. Make sure you have the Wokwi extension installed
3. Press **F1** (or Ctrl+Shift+P / Cmd+Shift+P)
4. Type and select: **"Wokwi: Start Simulator"**

### Step 2: View Logs

Once the simulator starts:

1. **Serial Monitor opens automatically** in the bottom panel
2. You'll see output like:

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

### Step 3: Interact with Hardware

**Potentiometer (Force Sensor):**
- Click and drag the slider UP → Simulates patient sitting (high pressure)
- Click and drag the slider DOWN → Simulates patient standing/fall (low pressure)

**Button (Nurse Input):**
- Click once briefly → Short press
- Click and hold for 2+ seconds → Long press

### Step 4: Test Fall Detection

Follow this sequence:

1. **Move potentiometer UP** (above 15%) - Patient sits down
2. **Wait 2-3 seconds** - System recognizes occupied state
3. **Quickly move potentiometer DOWN** (below 5%) - Simulates fall
4. **Watch Serial Monitor** - You'll see:
   ```
   Fall detected! Pressure dropped significantly.
   State Transition: POLLING -> ALARM
   ========================================
   !!!!! FALL ALARM TRIGGERED !!!!!
   Alert sent to Connexxall (Simulated)
   ========================================
   ```
5. **Click button** to clear the alarm

## Architecture Maintained

Even though `sketch.ino` is a single file, it still maintains the clean dependency injection architecture:

- **Interfaces**: IForceSensor, INurseInput, IAlertSystem
- **Implementations**: WokwiPotentiometer, WokwiButton, SerialConsoleAlert
- **State Machine**: FallDetector with all 6 states
- **Dependency Injection**: Constructor injection pattern preserved

## Why This Works Better

✅ **No build step** - Wokwi compiles automatically  
✅ **Instant start** - Just press F1 and go  
✅ **Logs visible** - Serial Monitor shows all output  
✅ **Same architecture** - All design patterns preserved  
✅ **Easy to test** - Interact directly with components  

## For Production Builds

The modular structure (`src/`, `include/`, `platformio.ini`) remains available for:
- Production hardware builds
- Advanced PlatformIO features
- Team development with separate files
- Unit testing

Just use PlatformIO commands for production:
```bash
pio run -e production-esp32 -t upload
```

## Troubleshooting

**Serial Monitor not showing?**
- Look for the terminal panel at the bottom of VS Code
- It should say "Serial Monitor" in the tab

**Simulator not starting?**
- Make sure Wokwi extension is installed
- Check that `diagram.json` and `wokwi.toml` exist in the root
- Try closing and reopening VS Code

**No circuit showing?**
- The circuit is defined in `diagram.json`
- Wokwi should load it automatically
- You should see ESP32, potentiometer, and button

## Summary

Use `sketch.ino` for Wokwi simulation - it's the easiest way to test the system with full logging support!
