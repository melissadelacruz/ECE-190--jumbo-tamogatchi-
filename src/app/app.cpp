#include <Arduino.h>
#include "../display/display.h"
#include "../sensors/imu.h"
#include "../sensors/pedometer.h"
#include "../sensors/shake.h"
#include "state.h"
#include "pet.h"
#include "network.h"
#include "app.h"

// The pet state is shared with the display and pet modules.
PetState petState;
Pedometer pedometer;
static ShakeDetector shakeDetector;
static unsigned long lastEnergyUpdate = 0;

void appSetup() {
    Serial.begin(115200);
    display_init();
    display_startup("Starting IMU...");
    Serial.println("Starting Tamagotchi");
    if (!imuSetup()) {
        Serial.println("IMU not found; check wiring and restart.");
        display_startup("IMU error: check wiring");
        while (true) delay(100);
    }
    Serial.println("IMU ready");
    
    // Initialize sensors

    // Calibrate pedometer baseline
    display_startup("Hold still...");
    Serial.println("Calibrating pedometer; hold still.");
    delay(3000);
    
    float sum = 0;
    for (int i = 0; i < 100; i++) {
        float ax, ay, az, gx, gy, gz;
        imuRead(ax, ay, az, gx, gy, gz);
        float l1 = abs(ax) + abs(ay) + abs(az);
        sum += l1;
        delay(10);
    }
    float calibratedBaseline = sum / 100;
    Serial.print("Calibrated baseline: ");
    Serial.println(calibratedBaseline);
    
    pedometer.begin();
    pedometer.setBaseline(calibratedBaseline);
    loadPetState();

    // shake detector
    shakeDetector.setThreshold(25.0); //delta value set from data recorded based on hand shake
    shakeDetector.setCooldown(2000);
    shakeDetector.setSamplesRequired(3);

    setup_buttons();
    display_startup("Connecting WiFi...");
    
    networkSetup();
    Serial.println("Startup complete");
}

static void updateSensors() {
    // IMU Read
    float ax, ay, az, gx, gy, gz;
    imuRead(ax, ay, az, gx, gy, gz);


    // shake feature for FEED MODE (printing delta for debugging)
    if (petState.feedMode) {
    // Calculate delta for debugging
    static float prevAx, prevAy, prevAz;
    static bool first = true;
    if (!first) {
        float delta = sqrt(pow(ax - prevAx, 2) + pow(ay - prevAy, 2) + pow(az - prevAz, 2));
        if (delta > 2.0) {  // Only print when something happens
            Serial.print("Delta: ");
            Serial.println(delta);
        }
    }
    prevAx = ax; prevAy = ay; prevAz = az;
    first = false;
}

    // Add this to see raw acceleration when in play mode (for debugging pedometer)
    if (petState.playMode) {
        static unsigned long lastPrint = 0;
        if (millis() - lastPrint > 2000) {
            lastPrint = millis();
            Serial.printf("Raw - ax: %.2f, ay: %.2f, az: %.2f\n", ax, ay, az);
        }
    }
    
    // PEDOMETER - Only in PLAY mode
    if (petState.playMode) {
        if (pedometer.update(ax, ay, az)) {
            petState.stepCount = pedometer.getStepCount();
            petState.playHearts = pedometer.getHappiness();
            petState.isWalking = true;
        }
    } else {
        petState.isWalking = false;
    }
    
    // SHAKE DETECTOR - Only in FEED mode
    if (petState.feedMode) {
        if (shakeDetector.update(ax, ay, az)) {
            if (petState.fullnessLevel < MAX_FULLNESS) {
                petState.fullnessLevel++;
            }
            Serial.printf("Fed pet; fullness: %d/%d\n", petState.fullnessLevel, MAX_FULLNESS);
        }
    }
    
    // ENERGY UPDATE - Only in BED mode
    if (petState.isSleeping) {
        if (millis() - lastEnergyUpdate > 30000) {
            lastEnergyUpdate = millis();
            if (petState.energyLevel < MAX_ENERGY) {
                petState.energyLevel++;
            }
        }
    }

}

void appLoop() {
    networkUpdate();
    updateSensors();

    // UI
    check_buttons();
    if (consumeMoodResetRequest()) {
        resetMoodBars();
    }

    decayPetMetersIfNeeded();
    updateOverallHappiness();
    savePetStateIfChanged();

    display_Home();
    delay(50);
}
