#include "Leds.h"
#include <Arduino.h>
//Leds::startUp

void LEDS_startUp() {
    for(int i = 0; i < 6; ++i) {
        for (int i = 0; i < 8; ++i) {
            digitalWriteFast(muteLeds[i], HIGH);
            delay(20);
            digitalWriteFast(muteLeds[i], LOW);
            delay(5);
        }
        for (int i = 0; i < 8; ++i) {
            digitalWriteFast(leds[i], HIGH);
            delay(20);
            digitalWriteFast(leds[i], LOW);   
            delay(5);     
        }
    }
}