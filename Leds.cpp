#include "Leds.h"
#include <Arduino.h>

unsigned int ledDelayTime = 100;
unsigned int ledDelayCounter[8] = {100}; 


void LEDS_startUp() {
    for(int i = 0; i < 2; ++i) {
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

void LEDS_on(unsigned int i) {
    digitalWriteFast(leds[i], HIGH);
}

void LEDS_off(unsigned int i) {
    // handle led delay, checking all every cycle, humans are slow.
    if (flagHasHandledNoteOn[i] == true)
    {
        if(ledDelayCounter[i]-- == 0) {
            digitalWriteFast(leds[i], LOW);
            flagHasHandledNoteOn[i] = false;
            ledDelayCounter[i] = ledDelayTime;
        }
    }
}