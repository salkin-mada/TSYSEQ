#include "Display.h"
#include <LedDisplay.h>

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
const long displayWriteInterval = 30;

void Display::setup() {
    display.begin();
    display.setBrightness(brightness);

    display.clear();
    display.setCursor(1);
    display.print("TSYSEQ");
    for (int i = 0; i < 3; i++) {
        //fade up
        for (brightness = 0; brightness < 16; brightness++) {
            display.setBrightness(brightness);
            delay(30);
        }
        delay(50);
        // fade down
        for (brightness = 15; brightness >= 0; brightness--) {
            display.setBrightness(brightness);
            delay(30);
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
        display.home();
        display.print("t:");
        display.print(round(displayWriteTime/1000));
    
        previousDisplayWriteTime = displayWriteTime;
    }
}