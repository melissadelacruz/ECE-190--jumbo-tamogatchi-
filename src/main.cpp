#include <Arduino.h>
#include "display/display.h"

void setup()
{
    Serial.begin(115200);
    delay(500);

    display_init();
    display_drawHello();

    Serial.println("System Initialized");
}

void loop()
{
}