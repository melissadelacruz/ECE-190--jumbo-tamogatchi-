#pragma once
#include <Arduino.h>

void display_init();
void display_startup(const char* status);
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

