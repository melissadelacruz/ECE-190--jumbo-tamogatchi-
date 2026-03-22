#ifndef PEDOMETER_H
#define PEDOMETER_H

#include <Arduino.h>

class Pedometer {
public:
    Pedometer();
    
    // Initialize pedometer
    void begin();
    
    // Update with accelerometer data - returns true if step detected
    bool update(float ax, float ay, float az);
    
    // Get current step count
    int getStepCount() { return stepCount; }
    
    // Reset step count
    void resetSteps();
    
    // Get current happiness (could be tied to steps)
    int getHappiness() { return happiness; }
    void setHappiness(int value);
    
    // Configuration
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
    
    // Helper functions
    float calculateL1Norm(float ax, float ay, float az);
    float calculateMovingAverage();
    bool detectStep(float value);
};

#endif

// // ==== Pedometer ====
//     // 1. L1 norm
//     float l1 = abs(ax) + abs(ay) + abs(az);

//     // 2. Moving average
//     buffer[bufIndex] = l1;
//     bufIndex = (bufIndex + 1) % WINDOW;

//     float avg = 0;
//     for (int i = 0; i < WINDOW; i++) {
//         avg += buffer[i];
//     }
//     avg /= WINDOW;

//     // 3. Detrend
//     float dt = avg - baseline;

//     // 4. Step detection
//     if (dt > 2.0 && !stepDetected) {
//         stepCount++;
//         stepDetected = true;
//         lastStepTime = millis();

//         Serial.print("Steps: ");
//         Serial.println(stepCount);

//         if (stepCount % 15 == 0 && stepCount != 0) {
//             happiness++;
//             Serial.println("Pet is happier!");
//         }
//     }

//     if (dt < 1.0) {
//         stepDetected = false;
//     }