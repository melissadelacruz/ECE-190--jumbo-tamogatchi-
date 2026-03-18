#pragma once
#include <Arduino.h>

// Variable "Promises" (Externs)
// This allows main.cpp to see these values without re-defining them (need these??)
extern int hungerLevel;
extern bool showHeart;
extern unsigned long heartTimer;

void display_init();
void display_Homepage();
void updatePetAnimation(bool isWalking);
void setup_buttons();
void check_buttons();