#pragma once

// Runtime values shared by the application, pet logic, and display.
// Only the meters and step count are saved across restarts.
struct PetState {
    int stepCount = 0;
    int playHearts = 0;
    int happiness = 0;
    int fullnessLevel = 0;
    int energyLevel = 0;
    bool playMode = false;
    bool feedMode = false;
    bool isSleeping = false;
    bool isWalking = false;
};

extern PetState petState;

constexpr int MAX_PLAY_HEARTS = 5;
constexpr int MAX_FULLNESS = 6;
constexpr int MAX_ENERGY = 5;
