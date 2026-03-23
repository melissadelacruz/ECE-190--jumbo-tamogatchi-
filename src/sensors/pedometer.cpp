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
    bufIndex = 0;
    stepCount = 0;
    happiness = 0;
    stepDetected = false;
    lastStepTime = 0;
    
    for (int i = 0; i < windowSize; i++) {
        buffer[i] = 0;
    }
}


// ---- needs to be more accurate (improved on accuracy) -----

bool Pedometer::update(float ax, float ay, float az) {
    float l1 = calculateL1Norm(ax, ay, az);
    
    // Add to buffer
    buffer[bufIndex] = l1;
    bufIndex = (bufIndex + 1) % windowSize;
    
    float avg = calculateMovingAverage();
    float dt = avg - baseline;
    
    // DEBUG: Print values every 1 second
    static unsigned long lastDebugPrint = 0;
    if (millis() - lastDebugPrint > 1000) {
        lastDebugPrint = millis();
        Serial.print("L1: ");
        Serial.print(l1);
        Serial.print(" | Avg: ");
        Serial.print(avg);
        Serial.print(" | Baseline: ");
        Serial.print(baseline);
        Serial.print(" | dt: ");
        Serial.print(dt);
        Serial.print(" | StepDetected: ");
        Serial.print(stepDetected ? "true" : "false");
        Serial.print(" | Steps: ");
        Serial.println(stepCount);
    }
    
    bool stepOccurred = false;
    
    if (detectStep(dt) && !stepDetected) {
        stepCount++;
        stepDetected = true;
        lastStepTime = millis();
        stepOccurred = true;
        
        Serial.print("*** STEP DETECTED! *** Total: ");
        Serial.println(stepCount);
        
        if (stepCount % 15 == 0 && stepCount != 0) {
            if (happiness < 5) {
                happiness++;
                Serial.print("Happiness increased to: ");
                Serial.println(happiness);
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

void Pedometer::setStepCount(int value) {
    if (value >= 0) {
        stepCount = value;
    }
}

void Pedometer::setWindowSize(int size) {
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
    return dt > 2.0;  // Try changing this to 1.0 or 1.5 if needed
}

void Pedometer::setHappiness(int value) {
    if (value >= 0 && value <= 5) {
        happiness = value;
    }
}
