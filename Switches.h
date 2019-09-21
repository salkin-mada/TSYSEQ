#ifndef SWITCHES_H
#define SWITCHES_H

//#include "Arduino.h"
#include <Bounce2.h>
const unsigned int numSensorGroups = 2;

extern int BUTTON_PRESSED;
extern int flagAbuttonWasPressed;

extern int buttonState[];
extern int lastButtonState[];

/*class*/struct muteButtons
{
//    public:
    void setup();
    void update();
//    private:
    //int readSensor(const unsigned int, const byte);
};

struct Touch
{
    void setup();
    void update();
    void trackSelector();
};


// class EncoderButtons
// {
//     public:
//     unsigned int 
// }

#endif