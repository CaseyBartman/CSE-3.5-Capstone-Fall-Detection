# Fall Detection System - User Manual

## Table of Contents
- [1. Introduction](#1-introduction)
- [2. Getting Started: Alerts](#2-getting-started-alerts)
- [3. How the System Works: A Day in the Life](#3-how-the-system-works-a-day-in-the-life)
- [4. Frequently Asked Questions (FAQ)](#4-frequently-asked-questions-faq)
- [5. Troubleshooting](#5-troubleshooting)

Another version of this document exists for future handoff documentation editing here: https://docs.google.com/document/d/1zu8R34xOgtsTTHlpuhSVWi8CvuyDlSidG8npKi_QmLA/edit?tab=t.0

## 1. Introduction

### 1.1 Purpose
This manual is your guide to using the Fall Detection System. It is written for clinical staff who will use the system daily. This document will help you understand what the system does, how to use it, and what to do in different situations.

### 1.2 Documentation Guide
This **User Manual** provides a simplified view of how patients and nurses interact with the system. It explains the different use cases and how the system behaves based on your inputs.

For more technical information, please see the other handoff documents:
-   **`README.md`**: This file is for engineers. It explains the technical architecture and how to set up the development environment to add new features.
-   **Definition of Done**: This is a product requirements document. It should be used by engineers to validate the implementation and by clinical staff to understand the system's exact capabilities and limitations.

### 1.3 What is the Fall Detection System?
The Fall Detection System is a smart pressure-sensing mat that helps prevent patient falls. You place it in front of the toilet, and it continuously monitors their movement. If a patient tries to stand up without assistance, the system instantly sends an alert to the nursing staff.

You can also use a small, wireless button to pause the system when you need to assist a patient, recalibrate the sensor, or acknowledge an alarm.

## 2. Getting Started: Alerts

### 2.1 Where Do the Alarm Signals Go?
When the system detects a fall risk, it sends an alert to a specific mobile notification channel using a service called **ntfy.sh**. This service acts like a private radio station for our alerts.

### 2.2 How Do I Receive Alerts?
To get these alerts on your phone, you need to "tune in" to our channel.

1.  **Download the App**: Go to your phone's app store (Google Play for Android or the App Store for iOS) and download the **ntfy** app.
2.  **Subscribe to the Topic**: Open the app and subscribe to the topic (channel) name provided by your hospital's IT department (e.g., `fall-detection-alerts-floor-3`).

Once you subscribe, your phone will receive a notification every time an alarm is triggered, just like receiving a text message.

## 3. How the System Works: A Day in the Life

The system is designed to be simple and predictable. Here are the common workflows you will encounter.

### Workflow 1: A Patient Tries to Stand Up (Alarm)
1.  **The Situation**: A patient on the mat tries to get up unsupervised.
2.  **What the System Does**: The mat detects the change in pressure and instantly sends an alert to the ntfy channel. Your phone receives a notification.
3.  **Your Action**: You go to the patient's room. After ensuring the patient is safe, you **press the wireless button once (a short, quick press)**.
4.  **What Happens Next**: Pressing the button silences the alarm and tells the system to go back to actively monitoring the patient.

### Workflow 2: Helping a Patient (Pause, No Calibration)
1.  **The Situation**: You need to help a patient get on or off the toilet assist them in a way that would otherwise trigger an alarm.
2.  **Your Action**: Before you move the patient, **press the wireless button once (a short, quick press)**.
3.  **What Happens Next**: The system is now **paused for 2 minutes**. The green light on the device may blink to show it's paused. You can now help the patient without setting off an alarm. After 2 minutes, the system will automatically return to active monitoring. You don't need to do anything else.

### Workflow 3: Calibrating the System
1.  **The Situation**: The patient's weight has shifted significantly during their stay, or you are getting false alarms. The system's baseline "empty" weight is off.
2.  **How Calibration Works**: Calibration tells the system, "This is the amount of pressure the patient exerts on the mat when they are in the process of standing up. Only alert me if the weight changes from *this* point, since that signifies the patient is attempting to use the toilet or get up form the toilet without supervision."
3.  **Your Action**:
    *   **Step 1: Pause the System.** Press the button once to enter the **PAUSED** state.
    *   **Step 2: Calibrate.** While the system is paused, press the button **again**.
4.  **What Happens Next**: The system will take a few seconds to measure the new baseline weight. Once it's done, it will automatically return to active monitoring.

## 4. Frequently Asked Questions (FAQ)

**Q: What happens if I press the button multiple times quickly?**
A: The system will follow a sequence. The first press will **pause** it. The second press will start **calibration**. Any more presses will be ignored until the current action is finished.

**Q: What if I press and hold the button down?**
A: The system only responds to short, quick presses. Holding the button down will have no effect. This prevents accidental commands.

**Q: How do I know if the system is paused or monitoring?**
A: The device will have an indicator light. A solid green light typically means it's actively monitoring, while a blinking light means it is paused.

**Q: What if I forget to un-pause the system?**
A: You don't have to! The system will automatically resume monitoring after the 2-minute pause timer is up.

**Q: The alarm went off, but the patient is fine. What do I do?**
A: This is a false alarm. Simply press the button once to acknowledge the alert and return the system to monitoring mode. If false alarms happen often, the system may need to be recalibrated.

## 5. Troubleshooting
| Issue | Action |
|---|---|
| System is not turning on | Check the power supply and connections. |
| False alarms are frequent | The system may need recalibration. Follow the calibration workflow (Pause → Calibrate). |
| Button presses are not registering | Check the battery on the BLE button and ensure it is within range of the main unit. |
| I'm not receiving alerts | Make sure you are subscribed to the correct ntfy topic in the app and have an internet connection. |
