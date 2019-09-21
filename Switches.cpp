#include "Switches.h"
#include "Leds.h"
//#include "Layout.h"
//#include "Arduino.h"
#include <Arduino.h>

Bounce commonPortDebouncers[numSensorGroups] = {Bounce()};

bool touchDebug = false;
extern bool debug;

const byte commonPort[numSensorGroups] = {24, 25};  // multiplexer common ports

// the multiplexer address select lines (A/B/C)
const byte inhibitAddressA = 26; // low-order bit
const byte inhibitAddressB = 27;
const byte inhibitAddressC = 28; // high-order bit


extern unsigned int stepCount;
extern unsigned int trackCount;

extern int selectedTrack;


// touch begin
const unsigned int numTouchPins = 5;
const unsigned int touchPins[numTouchPins] = {15,16,17,18,19};
bool trackMuteLatch[numTouchPins] = {false};

// calibration
unsigned int minimum = 2000; // keeping mininum kinda close to maximum.. touchRead is a button
unsigned int maximum = 3500; // what it takes to toggle

int current[numTouchPins]; 
unsigned int previous[numTouchPins];
// smoothing (very little is needed in this case, is any. touchRead is used as button)
const unsigned int numReadings = 5;
unsigned int readings[numTouchPins][numReadings];      // the readings
unsigned int readIndex[numTouchPins] = {0};              // the index of the current reading
unsigned int total[numTouchPins] = {0};                  // the running total
unsigned int average[numTouchPins] = {0};                // the average
const unsigned int smoothingInterval = 1; // very little is needed
unsigned long smoothingTime[numTouchPins];
unsigned long pastSmoothingTime[numTouchPins] = {0};
const unsigned int touchInterval = 20; // humans are slow
unsigned long touchTime;
unsigned long pastTouchTime = {0};
extern bool interruptCountButtonLatch;
// touch end

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

void Touch::setup() {
    if (touchDebug) {
        if (!debug) {
            Serial.begin(9600);
        }
    }
    for (unsigned int i = 0; i < numTouchPins; i++) {
		// initialize all readings to 0
		for (unsigned int y = 0; y < numReadings; y++) {
			readings[i][y] = 0;
		}
	}
}
void Touch::update() {

    touchTime = millis();

	if (touchTime - pastTouchTime >= touchInterval) {
        pastTouchTime = touchTime;

        for (unsigned int i = 0; i < numTouchPins; i++) {
            smoothingTime[i] = millis();

            current[i] = touchRead(touchPins[i]);
            current[i] = map(current[i], minimum, maximum, 0, 1); // makíng it a digital button
            current[i] = constrain(current[i], 0, 1);         

            if (smoothingTime[i] - pastSmoothingTime[i] >= smoothingInterval) {

                // subtract the last reading:
                total[i] = total[i] - readings[i][readIndex[i]];
                // read:
                readings[i][readIndex[i]] = current[i];
                // add to total:
                total[i] = total[i] + readings[i][readIndex[i]];
                // advance to the next position in the array:
                readIndex[i] = readIndex[i] + 1;

                // if at the end of the array
                if (readIndex[i] >= numReadings) {
                    readIndex[i] = 0; // wrap
                }

                // calculate the average:
                average[i] = total[i] / numReadings;

                // update time guard
                pastSmoothingTime[i] = smoothingTime[i];

                // if touch "button" state has changed
                // if(average[i] != previous[i]) {
                        // HACKKY this is used downstairs
                //     previous[i] = average[i];
                // }
            }
            if (touchDebug) {
                Serial.print("track: ");
                Serial.print(i+1);
                Serial.print("\t");
                Serial.print(touchRead(touchPins[i]));
                Serial.print("\t");
                Serial.print(current[i]);
                Serial.print("\t");
                Serial.println(average[i]);   
            }
        }
    }

}

void Touch::trackSelector() {
    if (!interruptCountButtonLatch) { // normal touch modus
        for (unsigned int i = 0; i < numTouchPins; i++) {
            if (average[i] == 1) { // it was touched, change the selected track
                selectedTrack = i;
                // clean up
                for (unsigned int i = 0; i < stepCount; ++i) {
                    LEDS_clear(i);            
                }
            }
	    }
    } else { // interruptCountButton latch is "high" aka "on" aka what not
        for (unsigned int i = 0; i < numTouchPins; i++) {
            if (average[i] == 1 && average[i] != previous[i]) {
                previous[i] = average[i];
                if (!trackMuteLatch[i]) {
                    // unmute track

                    trackMuteLatch[i] = true;

                } else {
                    trackMuteLatch[i] = false;
                }
            } else if (average[i] == 0 && average[i] != previous[i]) {
                previous[i] = average[i]; // make sure its updated when there is no touch
            }
	    }
    }
}
