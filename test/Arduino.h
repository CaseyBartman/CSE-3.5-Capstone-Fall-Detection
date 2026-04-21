#pragma once

/**
 * Mock Arduino.h for Unit Testing
 * 
 * This provides stub implementations of Arduino functions for testing
 * on a native C++ platform (not an actual Arduino board).
 * 
 * When compiled for testing, millis() is redirected to TimeSimulator
 * to provide deterministic timing for tests.
 */

#include <cstdio>
#include <iostream>
#include <cstring>
#include <functional>
#include <cstdint>

// Redirect millis() to TimeSimulator for deterministic testing
#ifdef TESTING_MODE
    #include "test/helpers/TimeSimulator.h"
    inline unsigned long millis() {
        return TimeSimulator::getInstance()->getCurrentTime();
    }
#else
    // For non-testing platforms, use actual millis() if available
    // This is a fallback that simulates millis behavior
    inline unsigned long millis() {
        static unsigned long start = 0;
        if (start == 0) start = 1;  // Avoid division by zero
        // Return a simulated time counter
        static unsigned long counter = 0;
        return counter++;
    }
#endif

// Serial object for printing (stub implementation)
class SerialClass {
public:
    void begin(unsigned long baud = 9600) {}
    
    void print(const char* str) {
        if (str) std::cout << str;
    }
    
    void print(int value) {
        std::cout << value;
    }
    
    void print(float value) {
        std::cout << value;
    }
    
    void print(unsigned long value) {
        std::cout << value;
    }
    
    void print(bool value) {
        std::cout << (value ? "true" : "false");
    }
    
    void println(const char* str = "") {
        if (str) std::cout << str;
        std::cout << "\n";
    }
    
    void println(int value) {
        std::cout << value << "\n";
    }
    
    void println(float value) {
        std::cout << value << "\n";
    }
    
    void println(unsigned long value) {
        std::cout << value << "\n";
    }
    
    void println(bool value) {
        std::cout << (value ? "true" : "false") << "\n";
    }
    
    int available() { return 0; }
    int read() { return -1; }
    void flush() {}
    
    template<typename T>
    SerialClass& operator<<(const T& value) {
        print(value);
        return *this;
    }
};

inline void delay(unsigned long ms) {}

extern SerialClass Serial;

#define A0 0
#define INPUT 0x0
#define OUTPUT 0x1
