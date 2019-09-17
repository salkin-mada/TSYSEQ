#include "Switches.h"
//#include "Layout.h"
//#include "Arduino.h"
#include <Arduino.h>

Bounce commonPortDebouncers[numSensorGroups] = {Bounce()};

const byte commonPort[numSensorGroups] = {24, 25};  // multiplexer common ports

// the multiplexer address select lines (A/B/C)
const byte inhibitAddressA = 26; // low-order bit
const byte inhibitAddressB = 27;
const byte inhibitAddressC = 28; // high-order bit


extern unsigned int stepCount;
extern unsigned int trackCount;

int /*muteButtons::*/readSensor(const unsigned int sensorGroup, const byte which) {
    // select MUX channel
    digitalWrite(inhibitAddressA, (which & 1) ? HIGH : LOW);
    digitalWrite(inhibitAddressB, (which & 2) ? HIGH : LOW);
    digitalWrite(inhibitAddressC, (which & 4) ? HIGH : LOW);

    delayMicroseconds(5); //// delay needed for bounced digital pins? or just analogread() ?
    
    // return correct common
    if (sensorGroup == 1) {
        //commonPortDebouncers[0].update();
        //return commonPortDebouncers[0].read();
        return digitalRead(commonPort[0]);
    } else if (sensorGroup == 2) {
        //commonPortDebouncers[1].update();
        //return commonPortDebouncers[1].read();
        return digitalRead(commonPort[1]);
    } else {
        // if querry is outside sensorGroups
        return HIGH; // return sensor as "HIGH"
    }
}


void muteButtons::setup() {
    // internal pullup for common ports
    for (unsigned int i = 0; i < numSensorGroups; ++i) {
        pinMode(commonPort[i], INPUT_PULLUP);
    }
    
    for (unsigned int i = 0; i < numSensorGroups; i++) {
        commonPortDebouncers[i].attach(commonPort[i]);
        commonPortDebouncers[i].interval(5);
    }

    pinMode(inhibitAddressA, OUTPUT); // low-order bit
    pinMode(inhibitAddressB, OUTPUT);
    pinMode(inhibitAddressC, OUTPUT); // high-order bit
}
    
  
void muteButtons::update() {
    byte sensorGroup = 1; // iterating from 1
    // get all 16 sensor readings
    for (byte i = 0; i < stepCount; i++) {
        if (i >= (stepCount/2)) {
            unsigned int y = i-8;
            sensorGroup = 2;
            buttonState[i] = /*muteButtons::*/readSensor(sensorGroup, y);
        } else {
        buttonState[i] = /*muteButtons::*/readSensor(sensorGroup, i);
        }
    }
}

