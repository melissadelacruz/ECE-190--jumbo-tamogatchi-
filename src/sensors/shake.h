#ifndef SHAKE_H
#define SHAKE_H

#include <Arduino.h>

class ShakeDetector {
public:
    ShakeDetector();
    
    bool update(float ax, float ay, float az);
    
    void setThreshold(float threshold) { this->threshold = threshold; }
    void setCooldown(unsigned long cooldown) { this->cooldown = cooldown; }
    void setSamplesRequired(int samples) { this->samplesRequired = samples; }
    
    float getThreshold() const { return threshold; }
    unsigned long getCooldown() const { return cooldown; }
    
private:
    float threshold;
    unsigned long cooldown;
    int samplesRequired;
    unsigned long lastShakeTime;
    int shakeSampleCount;
    
    float calculateMagnitude(float ax, float ay, float az) const;
};

#endif