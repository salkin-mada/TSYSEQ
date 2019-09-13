const int ShiftPWM_latchPin=8;
#define SHIFTPWM_NOSPI
const int ShiftPWM_dataPin = 1;
const int ShiftPWM_clockPin = 2;

const bool ShiftPWM_invertOutputs = false;

// You can enable the option below to shift the PWM phase of each shift register by 8 compared to the previous.
// This will slightly increase the interrupt load, but will prevent all PWM signals from becoming high at the same time.
// This will be a bit easier on your power supply, because the current peaks are distributed.
const bool ShiftPWM_balanceLoad = false;

#include "Leds.h"
#include <Arduino.h>
#include <ShiftPWM.h>

// program iteration based timing, not millis()
unsigned int ledDelayTime = 600; // init, will vary with seq devision
unsigned int ledDelayTimeFactor = 4000; // hence 4e3 is actually not five seconds
unsigned int ledDelayMaxTime = 8000;
unsigned int ledDelayCounter[16] = {ledDelayTime}; 

unsigned char maxBrightness = 255;
unsigned char pwmFrequency = 75;
unsigned int numRegisters = 6;
unsigned int numOutputs = numRegisters*8;
unsigned int numRGBLeds = numRegisters*8/3;

void LEDS_setup() {
    ShiftPWM.SetAmountOfRegisters(numRegisters);
    ShiftPWM.Start(pwmFrequency, maxBrightness);
}

void LEDS_startUp() {
    int stropeLength = 20;
    ShiftPWM.SetAll(0);

    for(int i = 0; i < stropeLength; i++){
        ShiftPWM.SetAll(8);
        delay(25);
        ShiftPWM.SetAll(0);
        delay(25);
    }

    for(int i = 0; i < 16; i++){
        //ShiftPWM.SetOne(i,150);
        ShiftPWM.SetHSV(i,50,255,255);
        delay(25);
        //ShiftPWM.SetOne(i,0);
    }
    //delay(10);
    for(int i = 16; i > 0; --i){
        //ShiftPWM.SetOne(i,150);
        ShiftPWM.SetHSV(i-1,50,255,0);
        delay(25);
        //ShiftPWM.SetOne(i,0);
    }

    //ShiftPWM.SetAll(0);

}

void LEDS_sdCardInitFailed() {
    for(int i = 0; i < 20; ++i) {
        for (int i = 0; i < 16; ++i) {
            ShiftPWM.SetHSV(i,150,255,255);
            //digitalWriteFast(muteLeds[i], HIGH);
        }
        delay(100);
        for (int i = 0; i < 16; ++i) {
            ShiftPWM.SetHSV(i,150,255,0);
            //digitalWriteFast(muteLeds[i], LOW);
        }
        delay(100);
    }
}

void LEDS_on(unsigned int i) {
    //digitalWriteFast(leds[i], HIGH);
        ShiftPWM.SetOne(leds[i],100);
        //ShiftPWM.SetHSV(i,50,255,255);
}

void LEDS_off(unsigned int y, unsigned int i) {
    // handle led delay, checking all every cycle, humans are slow.
    if (flagHasHandledNoteOn[y][i] == true)
    {
        if(ledDelayCounter[i]-- == 0) {
            //digitalWriteFast(leds[i], LOW);
            //ShiftPWM.SetHSV(i,50,255,0);
            
            ShiftPWM.SetOne(leds[i],0);
            flagHasHandledNoteOn[y][i] = false;
            ledDelayCounter[i] = ledDelayTime;
        }
    }
}

void LEDS_clear(unsigned int i) {
    ShiftPWM.SetOne(leds[i],0);
}

void LEDS_gateLength(unsigned int devisionFactor, unsigned int trackStepIterationDevider) {
    unsigned int result;
    result = (ledDelayTimeFactor/devisionFactor)*trackStepIterationDevider;
    // last *mul is here to make longer leds blinks dur to trackStepIterationDevider amount
    if (result > ledDelayMaxTime) {
        result = ledDelayMaxTime;
    }
    ledDelayTime = result; 
    
}

void MUTELEDS_on(unsigned int i) {
    //digitalWriteFast(leds[i], HIGH);
        ShiftPWM.SetOne(muteLeds[i],5);
        //ShiftPWM.SetHSV(i,50,255,255);
}

void MUTELEDS_off(unsigned int i) {
        ShiftPWM.SetOne(muteLeds[i],0);
}

void STEPLEDS_on(unsigned int i) {
    //digitalWriteFast(leds[i], HIGH);
        ShiftPWM.SetOne(stepLeds[i],25);
        //ShiftPWM.SetHSV(i,50,255,255);
}

void STEPLEDS_off(unsigned int i) {
        ShiftPWM.SetOne(stepLeds[i],0);
}