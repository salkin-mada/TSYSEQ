#include "Display.h"
#include <LedDisplay.h>
#include <Arduino.h>

// led display pins:
#define dataPin 33            // the display's data in
#define registerSelect 34     // the display's register select pin
#define clockPin 35           // the display's clock pin
#define enable 36             // the display's chip enable pin
#define reset 37              // the display's reset pin
// led display settings
#define displayLength 8       // number of characters
// #define TRUE 1
// #define FALSE 0
int brightness = 15;        // screen brightness
// start an instance of the LED display library:
LedDisplay display = LedDisplay(dataPin, registerSelect, clockPin, enable, reset, displayLength);

unsigned long previousDisplayWriteTime = 0;
const long displayWriteInterval = 30; // human eyes, no real need of higher refresh rate

extern int selectedTrack;
int lastPrintedTrack = 99; //initialize to something that can not be the selectedTrack after boot up, this is a print hack..
extern int interruptCountToStepResetValue;
int lastPrintedInterruptCountToStepResetValue;
extern int devisionValue;
int lastPrintedDevisionValue;
extern unsigned int trackStepIterationTrigDevider[5];
extern unsigned int trackCount;
unsigned int lastPrintedTrackStepIterationTrigDevider;
bool trackWasMuted = false;

extern bool trackMuteLatch[5];

void Display::setup() {
    display.begin();
    display.setBrightness(brightness);

    display.clear();
    display.setCursor(1);
    display.print("TSYSEQ");
    for (int i = 0; i < 2; i++) {
        //fade up
        for (brightness = 0; brightness < 16; brightness++) {
            display.setBrightness(brightness);
            delay(20);
        }
        delay(50);
        // fade down
        for (brightness = 15; brightness >= 0; brightness--) {
            display.setBrightness(brightness);
            delay(20);
        }
    }
    display.clear();
    display.setCursor(2);
    display.print("SDS8");
    for (int i = 0; i < 2; i++) {
        //fade up
        for (brightness = 0; brightness < 16; brightness++) {
            display.setBrightness(brightness);
            delay(20);
        }
        delay(50);
        // fade down
        for (brightness = 15; brightness >= 0; brightness--) {
            display.setBrightness(brightness);
            delay(20);
        }
    }

    // clear display
    display.clear();
    brightness = 15;
    display.setBrightness(brightness);
}

void Display::update() {
    unsigned long displayWriteTime = millis();

    if (displayWriteTime - previousDisplayWriteTime >= displayWriteInterval) {
        // should only print if something has changed..

        if (selectedTrack != lastPrintedTrack) {
            display.home();
            display.print("t"); // t for track
            display.setCursor(1);
            display.print(selectedTrack+1); // + for human attitude

            lastPrintedTrack = selectedTrack;
        }
        if (!trackMuteLatch[selectedTrack]) {
            if (trackStepIterationTrigDevider[selectedTrack] != lastPrintedTrackStepIterationTrigDevider || trackWasMuted) {
                if (trackStepIterationTrigDevider[selectedTrack] < 10) {
                    display.setCursor(2);
                    display.print(".");
                    display.setCursor(3);
                    display.print(trackStepIterationTrigDevider[selectedTrack]);
                } else {
                    display.setCursor(2);
                    display.print(trackStepIterationTrigDevider[selectedTrack]);
                }
                trackWasMuted = false;
                lastPrintedTrackStepIterationTrigDevider = trackStepIterationTrigDevider[selectedTrack];
            }
        } else {
            display.setCursor(2);
            display.print("<>");
            trackWasMuted = true;
        }
        
        

        if (devisionValue != lastPrintedDevisionValue) {
            if (devisionValue < 10) {
                display.setCursor(4);
                display.print("%");
                display.setCursor(5);
                display.print(devisionValue);
            } else {
                display.setCursor(4);
                display.print(devisionValue);
            }
            lastPrintedDevisionValue = devisionValue;
        }

        if (interruptCountToStepResetValue != lastPrintedInterruptCountToStepResetValue) {
            if (interruptCountToStepResetValue < 10) {
                display.setCursor(6);
                display.print("*");
                display.setCursor(7);
                display.print(interruptCountToStepResetValue);
            } else {
                display.setCursor(6);
                display.print(interruptCountToStepResetValue);
            }

            lastPrintedInterruptCountToStepResetValue = interruptCountToStepResetValue;
        }

        previousDisplayWriteTime = displayWriteTime;
    }
}