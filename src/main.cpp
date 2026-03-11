#include <Arduino.h>
#include "display/display.h"

void setup() {
    display_init();
    setup_buttons();
}

void loop() {
    check_buttons();
    display_drawHello();
    delay(10);
}
