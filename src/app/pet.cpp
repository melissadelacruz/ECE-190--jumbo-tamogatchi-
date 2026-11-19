#include "pet.h"
#include "state.h"
#include "../sensors/pedometer.h"
#include <Preferences.h>

extern Pedometer pedometer;
static Preferences preferences;
static unsigned long lastFullnessDecay = 0;
static unsigned long lastEnergyDecay = 0;
static unsigned long lastPlayDecay = 0;
static constexpr unsigned long FULLNESS_DECAY_INTERVAL_MS = 10UL * 60UL * 1000UL;
static constexpr unsigned long ENERGY_DECAY_INTERVAL_MS = 15UL * 60UL * 1000UL;
static constexpr unsigned long PLAY_DECAY_INTERVAL_MS = 20UL * 60UL * 1000UL;

// persistence section
static int clampMeter(int value, int minValue, int maxValue) {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

static int decayMeter(int value, int amount = 1) {
    value -= amount;
    return value < 0 ? 0 : value;
}

void loadPetState() {
    preferences.begin("tama-pet", false);

    petState.stepCount = preferences.getInt("steps", 0);
    petState.playHearts = clampMeter(preferences.getInt("play", 0), 0, MAX_PLAY_HEARTS);
    petState.fullnessLevel = clampMeter(preferences.getInt("full", 0), 0, MAX_FULLNESS);
    petState.energyLevel = clampMeter(preferences.getInt("energy", 0), 0, MAX_ENERGY);

    pedometer.setStepCount(petState.stepCount);
    pedometer.setHappiness(petState.playHearts);

    unsigned long now = millis();
    lastFullnessDecay = now;
    lastEnergyDecay = now;
    lastPlayDecay = now;
}

void savePetStateIfChanged() {
    static int savedStepCount = -1;
    static int savedPlayHearts = -1;
    static int savedFullness = -1;
    static int savedEnergy = -1;

    if (petState.stepCount != savedStepCount) {
        preferences.putInt("steps", petState.stepCount);
        savedStepCount = petState.stepCount;
    }
    if (petState.playHearts != savedPlayHearts) {
        preferences.putInt("play", petState.playHearts);
        savedPlayHearts = petState.playHearts;
    }
    if (petState.fullnessLevel != savedFullness) {
        preferences.putInt("full", petState.fullnessLevel);
        savedFullness = petState.fullnessLevel;
    }
    if (petState.energyLevel != savedEnergy) {
        preferences.putInt("energy", petState.energyLevel);
        savedEnergy = petState.energyLevel;
    }
}

void updateOverallHappiness() {
    int fullMeters = 0;
    if (petState.playHearts >= MAX_PLAY_HEARTS) fullMeters++;
    if (petState.fullnessLevel >= MAX_FULLNESS) fullMeters++;
    if (petState.energyLevel >= MAX_ENERGY) fullMeters++;
    petState.happiness = fullMeters;
}

void resetMoodBars() {
    petState.playHearts = 0;
    petState.fullnessLevel = 0;
    petState.energyLevel = 0;
    petState.happiness = 0;
    pedometer.setHappiness(0);

    unsigned long now = millis();
    lastFullnessDecay = now;
    lastEnergyDecay = now;
    lastPlayDecay = now;

    Serial.println("Mood bars reset");
}

void decayPetMetersIfNeeded() {
    unsigned long now = millis();

    while (now - lastFullnessDecay >= FULLNESS_DECAY_INTERVAL_MS) {
        petState.fullnessLevel = decayMeter(petState.fullnessLevel);
        lastFullnessDecay += FULLNESS_DECAY_INTERVAL_MS;
    }

    while (now - lastPlayDecay >= PLAY_DECAY_INTERVAL_MS) {
        petState.playHearts = decayMeter(petState.playHearts);
        pedometer.setHappiness(petState.playHearts);
        lastPlayDecay += PLAY_DECAY_INTERVAL_MS;
    }

    if (!petState.isSleeping) {
        while (now - lastEnergyDecay >= ENERGY_DECAY_INTERVAL_MS) {
            petState.energyLevel = decayMeter(petState.energyLevel);
            lastEnergyDecay += ENERGY_DECAY_INTERVAL_MS;
        }
    } else {
        lastEnergyDecay = now;
    }
}

