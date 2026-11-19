#pragma once
#include <WiFi.h>
const char* wifiStatusName(wl_status_t status);
void fetchTemperature();
void networkSetup();
void networkUpdate();
