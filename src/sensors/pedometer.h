#ifndef PEDOMETER_H
#define PEDOMETER_H

#include <Arduino.h>

class Pedometer {
public:
    Pedometer();
    
    void begin();
    bool update(float ax, float ay, float az);
    
    int getStepCount() { return stepCount; }
    void resetSteps();
    
    int getHappiness() { return happiness; }
    void setHappiness(int value);
    
    void setBaseline(float baseline) { this->baseline = baseline; }
    void setWindowSize(int size);
    
private:
    static const int DEFAULT_WINDOW = 10;
    
    float* buffer;
    int bufIndex;
    int windowSize;
    
    int stepCount;
    int happiness;
    bool stepDetected;
    
    float baseline;
    unsigned long lastStepTime;
    
    // ADD THESE NEW VARIABLES HERE ↓↓↓
    float dynamicBaseline;  // For adaptive baseline
    int stepCounter;         // For step counting logic
    
    // Helper functions
    float calculateL1Norm(float ax, float ay, float az);
    float calculateMovingAverage();
    bool detectStep(float value);
};

#endif