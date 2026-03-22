#pragma once
#include <Arduino.h>

// Variable "Promises" (Externs)
// This allows main.cpp to see these values without re-defining them (need these??)

void display_init();
void display_Home();
void updatePetAnimation(bool isWalking);
void setup_buttons();
void check_buttons();
void display_setTime(int h, int m, bool isPM);
void display_setDate(int month, int day);
void display_setTemp(int temp);
void display_Feed();
void display_Play();
bool detectShake(float ax, float ay, float az);

//keep??
extern int hungerLevel;
extern bool showHeart;
extern unsigned long heartTimer;

//keep
extern int stepCount;
extern int happiness;
extern int fullnessLevel;
extern int energyLevel;
extern bool playMode;
extern bool feedMode;
extern bool isSleeping;
extern bool isWalking;
extern unsigned long lastShakeTime;