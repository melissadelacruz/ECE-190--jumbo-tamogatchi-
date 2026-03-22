#include "pedometer.h"

Pedometer::Pedometer() {
    windowSize = DEFAULT_WINDOW;
    buffer = new float[windowSize];
    bufIndex = 0;
    stepCount = 0;
    happiness = 0;
    stepDetected = false;
    baseline = 29.0;
    lastStepTime = 0;
    
    // Initialize buffer
    for (int i = 0; i < windowSize; i++) {
        buffer[i] = 0;
    }
}

void Pedometer::begin() {
    // Reset all values
    bufIndex = 0;
    stepCount = 0;
    happiness = 0;
    stepDetected = false;
    lastStepTime = 0;
    
    for (int i = 0; i < windowSize; i++) {
        buffer[i] = 0;
    }
}

bool Pedometer::update(float ax, float ay, float az) {
    float l1 = calculateL1Norm(ax, ay, az);
    
    // Add to buffer
    buffer[bufIndex] = l1;
    bufIndex = (bufIndex + 1) % windowSize;
    
    float avg = calculateMovingAverage();
    float dt = avg - baseline;
    
    bool stepOccurred = false;
    
    if (detectStep(dt) && !stepDetected) {
        stepCount++;
        stepDetected = true;
        lastStepTime = millis();
        stepOccurred = true;
        
        // Increase happiness every 15 steps
        if (stepCount % 15 == 0 && stepCount != 0) {
            if (happiness < 5) {
                happiness++;
            }
        }
    }
    
    if (dt < 1.0) {
        stepDetected = false;
    }
    
    return stepOccurred;
}

void Pedometer::resetSteps() {
    stepCount = 0;
}

void Pedometer::setWindowSize(int size) {
    // Reallocate buffer with new size
    delete[] buffer;
    windowSize = size;
    buffer = new float[windowSize];
    bufIndex = 0;
    
    for (int i = 0; i < windowSize; i++) {
        buffer[i] = 0;
    }
}

float Pedometer::calculateL1Norm(float ax, float ay, float az) {
    return abs(ax) + abs(ay) + abs(az);
}

float Pedometer::calculateMovingAverage() {
    float avg = 0;
    for (int i = 0; i < windowSize; i++) {
        avg += buffer[i];
    }
    return avg / windowSize;
}

bool Pedometer::detectStep(float dt) {
    return dt > 2.0;
}

void Pedometer::setHappiness(int value) {
    if (value >= 0 && value <= 5) {
        happiness = value;
    }
}