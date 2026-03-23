#include "shake.h"

ShakeDetector::ShakeDetector() {
    threshold = 8.0;  // Higher threshold for change in acceleration
    cooldown = 2000;  // 2 seconds cooldown between shakes
    samplesRequired = 3;
    lastShakeTime = 0;
    shakeSampleCount = 0;
    
    // Initialize previous values
    prevAx = 0;
    prevAy = 0;
    prevAz = 0;
    hasPrevValues = false;
}

bool ShakeDetector::update(float ax, float ay, float az) {
    // Calculate change in acceleration (derivative)
    float deltaX = 0;
    float deltaY = 0;
    float deltaZ = 0;
    
    if (hasPrevValues) {
        deltaX = abs(ax - prevAx);
        deltaY = abs(ay - prevAy);
        deltaZ = abs(az - prevAz);
    }
    
    // Store current values for next time
    prevAx = ax;
    prevAy = ay;
    prevAz = az;
    hasPrevValues = true;
    
    // Calculate total change magnitude
    float deltaMagnitude = sqrt(deltaX*deltaX + deltaY*deltaY + deltaZ*deltaZ);
    
    // Check cooldown period
    if (millis() - lastShakeTime < cooldown) {
        shakeSampleCount = 0;
        return false;
    }
    
    // Detect shake based on change in acceleration
    if (deltaMagnitude > threshold) {
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