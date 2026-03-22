#include "shake.h"

ShakeDetector::ShakeDetector() {
    threshold = 2.5;
    cooldown = 500;
    samplesRequired = 3;
    lastShakeTime = 0;
    shakeSampleCount = 0;
}

bool ShakeDetector::update(float ax, float ay, float az) {
    float magnitude = calculateMagnitude(ax, ay, az);
    
    if (millis() - lastShakeTime < cooldown) {
        shakeSampleCount = 0;
        return false;
    }
    
    if (magnitude > threshold) {
        shakeSampleCount++;
        
        if (shakeSampleCount >= samplesRequired) {
            lastShakeTime = millis();
            shakeSampleCount = 0;
            return true;
        }
    } else {
        shakeSampleCount = 0;
    }
    
    return false;
}

float ShakeDetector::calculateMagnitude(float ax, float ay, float az) const {
    return sqrt(ax*ax + ay*ay + az*az);
}