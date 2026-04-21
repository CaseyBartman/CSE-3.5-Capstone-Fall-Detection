#include "TimeSimulator.h"

// Initialize static member
TimeSimulator* TimeSimulator::_instance = nullptr;

// Implementation of the getInstance method
TimeSimulator* TimeSimulator::getInstance() {
    if (_instance == nullptr) {
        _instance = new TimeSimulator();
    }
    return _instance;
}