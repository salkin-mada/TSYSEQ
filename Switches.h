#ifndef SWITCHES_H
#define SWITCHES_H

//#include "Arduino.h"
#include <Bounce2.h>
const unsigned int numSensorGroups = 2;



extern int BUTTON_PRESSED;
extern int flagAbuttonWasPressed;

extern int buttonState[];
extern int lastButtonState[];



//const unsigned int muteButtons[] = {24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39};

/*class*/struct muteButtons
{
//    public:
    void setup();
    void update();
//    private:
    //int readSensor(const unsigned int, const byte);
};

// struct encoderButtons
// {
    
// };


// class EncoderButtons
// {
//     public:
//     unsigned int 
// }

#endif