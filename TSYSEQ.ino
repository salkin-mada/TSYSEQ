// hallo
// at the moment this sequencer incorporates some bad math
// this gives some interesting counter intuitive results

// todo
// resetAmountOfTicks (how many ticks per reset)
// try responsive analog reading for pots


// std::random_shuffle
// PImpl
// tupedefs

#include <Wire.h>
#include <SerialFlash.h>
#include <Bounce2.h>



// #include <Arduino.h> // check om det hjælper på ledDelayTime print her?
// eller skal jeg bruge inline voids og static inline int eksempelvis
#include "Settings.h"
#include "Leds.h"

extern bool debug;

int calculation = 0; // for Serial monitor debugging, math helper

// time guard stuff for quick scope
unsigned long previousTimeGuard = 0;
const long timeGuardInterval = 200;

int flagAbuttonWasPressed = 0;

// seq stuff
//////////////////
//////////////////

// layout
unsigned int stepCount = 8;
unsigned int trackCount = 5;

const unsigned int muteButtons[] = {24, 25, 26, 27, 28, 29, 30, 31};
Bounce debouncer[8] = {Bounce()}; //deboucing

int buttonState[8] = {1};
int lastButtonState[8] = {1};
int BUTTON_PRESSED = 0;

// multi dimensional arrays for track logic (5 tracks)
bool doStep[5][8] = {
    {false, false, false, false, false, false, false, false},
    {false, false, false, false, false, false, false, false},
    {false, false, false, false, false, false, false, false},
    {false, false, false, false, false, false, false, false},
    {false, false, false, false, false, false, false, false}
};
bool flagHasHandledNoteOn[5][8] = {
    {false, false, false, false, false, false, false, false},
    {false, false, false, false, false, false, false, false},
    {false, false, false, false, false, false, false, false},
    {false, false, false, false, false, false, false, false},
    {false, false, false, false, false, false, false, false}
};


// tempo pins
const unsigned int tempoled = 13;
const unsigned int tempoOut = 23; // used for tempo input to the interrupt pin.

//switch for track selection, 5 tracks in all
int selectedTrack = 0; // init track is 0, not part of SD_writeSettings yet..
int lastSelectedtrack = 0;
const unsigned int switchUpPin = 22;
const unsigned int switchDownPin = 21;
Bounce debounceUpTrig = Bounce(); //deboucing
Bounce debounceDownTrig = Bounce(); //deboucing
int switchUpTrig;
int switchUpState = 1;
int lastSwitchUpState = 1;
int switchDownTrig;
int switchDownState = 1;
int lastSwitchDownState = 1;

// trigger outputs
const unsigned int pinsTrack[5] = {1,2,3,4,20}; // track outputs, going to voltage devider and jack out
unsigned int trackStepIterationtrigDevider[5] = {1,3,2,8,6};
unsigned int trackStepIterationClockCounter[5] = {1,1,1,1,1}; // they iterate from 1
bool madeTrigFlag[5] = {false,false,false,false,false};
unsigned int triggerHoldTime = 100;
unsigned int triggerHoldTimeCounter[5] = {triggerHoldTime}; 


int gateNr = 0; //sequencer inits
const unsigned int gateNrMap[] = {
    0,1,2,3,4,5,6,7,
    0,1,2,3,4,5,6,7,
    0,1,2,3,4,5,6,7,
    0,1,2,3,4,5,6,7 // max 4 times wrapping. 
    // maybe higher is needed due to interruptCountToStepReset
}; // translation map for maximum 16th devisions

// control declarations/definitions
// seq length and offset
int seqStartPotPin = A0;
int seqEndPotPin = A1;
int seqOffsetPin = A2;
int seqStartValue;
int seqEndValue;
int seqOffsetValue;

// fraction
int devisionPot = A3;
int devisionValue;

unsigned int dividendValue;
unsigned int divisorValue;

// experimental
int interruptCountToStepResetPot = A4;
int interruptCountToStepResetValue;

// internal clock
int tempoPin = A5;
float clockbpmtime;

// CV control
//int cvValuePotPin = A5;
//int cvAssignDestinationStepButtonPin; // any digital pin
//int cvOutValue;
//int cvAssignDestinationStep;

//interrupt stuff
const int pinClock = 0; // external clock in i dette tilfælde en hacket internal clock ud og så ind her ...
//const int debouncetime = 1000; //debouncetid i mikrosekunder
volatile bool flag = false;
int interruptCountToStepReset = 0;
float fraction = 1.0/8.0; // 1/8 node init devision

void irqClock() {
    /*     if (interruptCountToReset) {
        flag = true;
        interruptCountToReset = 0;
    } */
    flag = true;
}


void setup() {
    Serial.begin(9600);
    //Serial.begin(19200);
    while(!Serial && millis() < 5000) {
        // wait for Serial but if more than 5k millis passed run program
    };
    
    analogReference(EXTERNAL); 
    // used INTERNAL but now EXT, 
    // will be adding some noise but quick fix for proper analog readings
    
    // seq stuff
    pinMode(pinClock, INPUT);
    attachInterrupt(digitalPinToInterrupt(pinClock), irqClock, RISING);
    
    pinMode(tempoPin, INPUT);
    pinMode(tempoled, OUTPUT);
    pinMode(tempoOut, OUTPUT);

    for (unsigned int i = 0; i < trackCount; ++i) {
        pinMode(pinsTrack[i], OUTPUT);
    }
    
    for (unsigned int i = 0; i < stepCount; ++i) {
        pinMode(leds[i], OUTPUT);
    }
    
    for (unsigned int i = 0; i < stepCount; ++i) {
        pinMode(muteLeds[i], OUTPUT);
    }
    
    // Pullup for mute pins
    for (unsigned int i = 0; i < stepCount; ++i) {
        pinMode(muteButtons[i], INPUT_PULLUP);
    }

    //track page switch (5 tracks, one for each sds-8 drum)
    pinMode(switchUpPin, INPUT_PULLUP);
    pinMode(switchDownPin, INPUT_PULLUP);

    // debounce setup
    for (unsigned int i = 0; i < stepCount; ++i) {
        debouncer[i].attach(muteButtons[i]);
        debouncer[i].interval(5);
    }

    debounceUpTrig.attach(switchUpPin);
    debounceUpTrig.interval(5);
    debounceDownTrig.attach(switchDownPin);
    debounceDownTrig.interval(5);
    

    SD_init();
    SD_readSettings(); // and apply to program parameters/variables

    //apply loaded settings of selected track to UI (muteLeds)
    for (unsigned int i = 0; i < stepCount; ++i) {
        if (doStep[selectedTrack][i] == true) {
            digitalWriteFast(muteLeds[i], HIGH);
            if (debug) {
                Serial.print("init muteLeds");
                Serial.print(i+1);
                Serial.println(" ON");
            }
        }
        if (debug) {
            if (i == (stepCount-1) ) {
                Serial.println("stored steps have been loaded into program");
            }
        }
    }

    if (debug) {
        SD_readAllSettings2Monitor(); // debugging, dump all settings
    }

    LEDS_startUp(); // all set, ready to go
}

void setStepState(unsigned int i) {
    debouncer[i].update();
    //buttonState[i] = digitalRead(muteButtons[i]); // classic no debounce
    buttonState[i] = debouncer[i].read();
    
    if(buttonState[i] != lastButtonState[i]) {
        if(buttonState[i] == BUTTON_PRESSED) {
            flagAbuttonWasPressed = 1;
            if(doStep[selectedTrack][i] == false) {
                doStep[selectedTrack][i] = true;
                digitalWriteFast(muteLeds[i], HIGH);
                //Serial.println("true");
            } else if(doStep[selectedTrack][i] == true){
                doStep[selectedTrack][i] = false;
                digitalWriteFast(muteLeds[i], LOW);
                //Serial.println("false");
            }
            //SD_read(i); // read settings
            // check is something has changed
            //SD_write(i); // write settings
        }
        lastButtonState[i] = buttonState[i];
    }
}

void updateMuteLedsDueToTrackSelection() {
    //if (selectedTrack != lastSelectedtrack) { // breaks something
        for (int i = 0; i < 8; ++i) {
            if(doStep[selectedTrack][i] == true) {
                digitalWriteFast(muteLeds[i], HIGH);
            } else if(doStep[selectedTrack][i] == false){
                digitalWriteFast(muteLeds[i], LOW);
            }
        }
        lastSelectedtrack = selectedTrack;
    //}
}

void checkTriggers(unsigned int step) {
    // check all tracks
    for (unsigned int track = 0; track < 5; ++track) {
        // new, hhm counting what math, hard.. hmm
        if (doStep[track][step] == true) {
            
            if (trackStepIterationtrigDevider[track] == trackStepIterationClockCounter[track]) {
                makeTrigger(track);
                
            }
            
            // iterate
            trackStepIterationClockCounter[track]++;
            
            // if counter is bigger than set trigDevider 
            if (trackStepIterationClockCounter[track] > trackStepIterationtrigDevider[track]) {
                //reset counter for track
                trackStepIterationClockCounter[track] = 1;
            }
            
        }

        // old sheise
        // if (i == 0 && doStep[y][i] == true) {
        //     makeTrigger(y);
        // }
        // if (i == 1 && doStep[y][i] == true) {
        //     makeTrigger(y);
        // }
        // if (i == 2 && doStep[y][i] == true) {
        //     makeTrigger(y);
        // }
        // if (i == 3 && doStep[y][i] == true) {
        //     makeTrigger(y);
        // }
        // if (i == 4 && doStep[y][i] == true) {
        //     makeTrigger(y);
        // }
        // if (i == 5 && doStep[y][i] == true) {
        //     makeTrigger(y);
        // }
        // if (i == 6 && doStep[y][i] == true) {
        //     makeTrigger(y);
        // }
        // if (i == 7 && doStep[y][i] == true) {
        //     makeTrigger(y);
        // }
    }
}

void makeTrigger(unsigned int i) {
    digitalWriteFast(pinsTrack[i], HIGH);
    madeTrigFlag[i] = true;
}

void triggersOFF(unsigned int i) {
    if (madeTrigFlag[i] == true)
    {
        if(triggerHoldTimeCounter[i]-- == 0) {
            digitalWriteFast(pinsTrack[i], LOW);
            madeTrigFlag[i] = false;
            triggerHoldTimeCounter[i] = triggerHoldTime;
        }
    }
}

/* void handleLedDelay(unsigned int i) {
    // checking them all every cycle, humans are slow.
    if (flagHasHandledNoteOn[i] == true)
    {
        if(ledDelayCounter[i]-- == 0) {
            digitalWriteFast(leds[i], LOW);
            flagHasHandledNoteOn[i] = false;
            ledDelayCounter[i] = ledDelayTime;
        }
    }
}  */

// should it be switch case logic baby?
void handleNoteOn(unsigned int i) {
    if (i == 0 && doStep[selectedTrack][i] == true) {
        LEDS_on(i);
        // digitalWriteFast(leds[i], HIGH);
        flagHasHandledNoteOn[selectedTrack][i] = true;
    }
    if (i == 1 && doStep[selectedTrack][i] == true) {
        LEDS_on(i);
        // digitalWriteFast(leds[i], HIGH);
        flagHasHandledNoteOn[selectedTrack][i] = true;
    }
    if (i == 2 && doStep[selectedTrack][i] == true) {
        LEDS_on(i);
        // digitalWriteFast(leds[i], HIGH);
        flagHasHandledNoteOn[selectedTrack][i] = true;
    }
    if (i == 3 && doStep[selectedTrack][i] == true) {
        LEDS_on(i);
        // digitalWriteFast(leds[i], HIGH);
        flagHasHandledNoteOn[selectedTrack][i] = true;
    }
    if (i == 4 && doStep[selectedTrack][i] == true) {
        LEDS_on(i);
        // digitalWriteFast(leds[i], HIGH);
        flagHasHandledNoteOn[selectedTrack][i] = true;
    }
    if (i == 5 && doStep[selectedTrack][i] == true) {
        LEDS_on(i);
        // digitalWriteFast(leds[i], HIGH);
        flagHasHandledNoteOn[selectedTrack][i] = true;
    }
    if (i == 6 && doStep[selectedTrack][i] == true) {
        LEDS_on(i);
        // digitalWriteFast(leds[i], HIGH);
        flagHasHandledNoteOn[selectedTrack][i] = true;
    }
    if (i == 7 && doStep[selectedTrack][i] == true) {
        LEDS_on(i);
        // digitalWriteFast(leds[i], HIGH);
        flagHasHandledNoteOn[selectedTrack][i] = true;
    } 
}


void trackSelect() {
    debounceUpTrig.update();
    debounceDownTrig.update();
    switchUpState = debounceUpTrig.read();
    switchDownState = debounceDownTrig.read();
    
    if(switchUpState != lastSwitchUpState) {
        if(switchUpState == BUTTON_PRESSED) {
            if (debug) {
                Serial.println("track UP");
            }
            switch(selectedTrack) {
                case 0:
                selectedTrack = 1;
                break;
                case 1:
                selectedTrack = 2;
                break;
                case 2:
                selectedTrack = 3;
                break;
                case 3:
                selectedTrack = 4;
                break;
                case 4:
                selectedTrack = 0;
                break;
                default:
                selectedTrack = 0; //back to init if needed
                break;
            }
            
            if (debug) {
                Serial.print("selectedTrack: ");
                Serial.println(selectedTrack);
            }
            
        }
        lastSwitchUpState = switchUpState;
    }

    if(switchDownState != lastSwitchDownState) {
        if(switchDownState == BUTTON_PRESSED) {
            if (debug) {
                Serial.println("track DOWN");
            }
            switch(selectedTrack) {
                case 0:
                selectedTrack = 4;
                break;
                case 1:
                selectedTrack = 0;
                break;
                case 2:
                selectedTrack = 1;
                break;
                case 3:
                selectedTrack = 2;
                break;
                case 4:
                selectedTrack = 3;
                break;
                default:
                selectedTrack = 0; //back to init if needed, aka wrapper
                break;
            }
            if (debug) {
                Serial.print("selectedTrack: ");
                Serial.println(selectedTrack);
            }
        }
        // note: used for wrapper?
        lastSwitchDownState = switchDownState;
    }
}



void loop() {
    //tracker inits
    unsigned int t1 = 250000;
    elapsedMicros microtime;
    elapsedMicros microbeattime = 10000;
    // elapsedMicros debouncer = 0;
    bool resettracker = true;
    
    // internal clock
    elapsedMicros clocktime;
    
    //quick scope
    while (1) {
        // time guard for quick scope
        unsigned long currentTimeGuard = millis();   
        
        clockbpmtime = analogRead(tempoPin); // try responsive version?
        clockbpmtime = map(clockbpmtime, 0, 1023, 6000000, 100000); // det var 4e6, 2e5
        
        //internal clock hack
        if (clocktime >= clockbpmtime) {
            digitalWriteFast(tempoled, HIGH);
            digitalWriteFast(tempoOut, HIGH);
            delay(1); // remove delay with nice counter delay, like on the step leds
            digitalWriteFast(tempoled, LOW);
            digitalWriteFast(tempoOut, LOW);
            clocktime = 0;
        }
        
        //seq controls
        // read knobs
        seqStartValue = analogRead(seqStartPotPin);
        seqEndValue = analogRead(seqEndPotPin);
        seqOffsetValue = analogRead(seqOffsetPin);
        devisionValue = analogRead(devisionPot);
        interruptCountToStepResetValue = analogRead(interruptCountToStepResetPot);
        
        seqStartValue = constrain(map(seqStartValue, 0, 1010, 0, 7), 0, 7);
        seqEndValue = constrain(map(seqEndValue, 0, 850, 0, 7), 0, 7);
        seqOffsetValue = map(seqOffsetValue, 0, 1000, 0, 7);
        devisionValue = map(devisionValue, 90, 1023, 1, 16);
        interruptCountToStepResetValue = map(interruptCountToStepResetValue, 90, 1023, 1, 16);
        
        // Offset into ->> seqStart and seqEnd
        seqStartValue = seqStartValue+seqOffsetValue;
        seqEndValue = seqEndValue+seqOffsetValue;


        /* ############################################################# */
        /* ######################## FRACTION ########################## */
        /* ############################################################# */

        // control af fraction
        float fraction = 1.0/devisionValue;
        // also interesting devident devider attitude
       // float fraction = 4.0/devisionValue;
        
        // oldest one, first
        //float fraction = 1.0/devisionValue;

        // second attempt
        // dividend (faster)
        // dividendValue = analogRead(devisionPot);
        // dividendValue = constrain(map(dividendValue, 550, 1023, 1, 16), 1, 16);

        // // devisor (slower)
        // divisorValue = analogRead(devisionPot);
        // divisorValue = constrain(map(divisorValue, 90, 450, 16, 1), 1, 16);

        // float fraction = 1.0 / (dividendValue / divisorValue);

        /* ############################################################# */
        /* ############################################################# */
        /* ############################################################# */
        
        //track selector
        trackSelect();
        updateMuteLedsDueToTrackSelection();
        
        // check if steps are on or off
        for (int i = 0; i < 8; ++i) {
            setStepState(i);
            SD_writeSettings(i);
            flagAbuttonWasPressed = 0; // burde denne være inde i SD_writeSettings?
            // uanset så er det safety measures at sætte den til 0 hele tiden.
            // så skrives der kun én gang per knaptryk
        }
        
        //tracker
        if (flag /*&& (debouncer > debouncetime)*/) {
            
            t1 = (float(microtime) * fraction);
            microtime = 0;
            microbeattime = 0;
            flag = false;
            
            //extended "sync" part of tracking
            // hvor mange clicks der skal til før at sekvensen resettes
            if (interruptCountToStepReset >= interruptCountToStepResetValue) {
                resettracker = true;
                gateNr = seqStartValue;
                //Serial.println("¤_STEP_RESET_¤");
                interruptCountToStepReset = 0;
            }
            //gateNr = seqStartValue;
            
            // fraction kan bruges i en funktion sådan at det kan
            // kontrolleres hvor ofte sequenceren sætter seqStartValue til gateNr
            // men der bliver jo tænkt lidt anderldes om dette i seq delen nedenunder .
            // if gate er større end end value set gate til seqstartvalue

            interruptCountToStepReset++;
        }
        
        //sequencer
        if (microbeattime >= t1 || resettracker) {
            
            /* length of seq, old
            if(gateNr >= seqEndValue) {
                gateNr = seqStartValue;
            }*/
            // length of seq, new turn
            if(gateNr >= 1 + seqEndValue + constrain((devisionValue - 8), 0, 8) ) {
                gateNr = seqStartValue; // trying to control wierd fold-back/over behavior
            }
            
            // debugging
            /*Serial.print("###########");
            Serial.println("___________");
            Serial.print("seqStartValue: ");
            Serial.println(seqStartValue);
            Serial.print("seqEndValue: ");
            Serial.println(seqEndValue);
            Serial.print("seqOffsetValue: ");
            Serial.println(seqOffsetValue);
            Serial.print("devisionValue: ");
            Serial.println(devisionValue);
            Serial.print("interruptCountToStepResetValue: ");
            Serial.println(interruptCountToStepResetValue);
            Serial.print("gateNrMap: ");
            Serial.println(gateNrMap[gateNr]);
            Serial.print("gateNr: ");
            Serial.println(gateNr);


            Serial.print("dividendValue (faster): ");
            Serial.println(dividendValue);
            Serial.print("divisorValue (slower): ");
            Serial.println(divisorValue);
            */

            
            // noteOn
            // make step --> also here to implement probability, if (bla bla)
            handleNoteOn(gateNrMap[gateNr]); // led handling and flag
            checkTriggers(gateNrMap[gateNr]); // send trigs out

            // seq
            ++gateNr;
            microbeattime = 0;
            resettracker = false;
        }

        // checking the state of all triggers every cycle, humans are slow
        // handling TRIGGER "on" time
        for (int i = 0; i < 5; ++i){
            triggersOFF(i);
        }
        
        // checking the state of all leds every cycle, humans are slow
        // handling LED "on" time
        for (int i = 0; i < 8; ++i){
            LEDS_off(selectedTrack, i);   
        }
        
        // debugg serial print delay
        if (currentTimeGuard - previousTimeGuard >= timeGuardInterval) {
            //SD_readAllSettings2Monitor();
            
            /* Serial.print("seqStartValue: ");
            Serial.println(seqStartValue);
            Serial.print("seqEndValue: ");
            Serial.println(seqEndValue);
            Serial.print("seqOffsetValue: ");
            Serial.println(seqOffsetValue);
            Serial.print("devisionValue: ");
            Serial.println(devisionValue);
            Serial.print("interruptCountToStepResetValue: ");
            Serial.println(interruptCountToStepResetValue); */
            
            
            
            previousTimeGuard = currentTimeGuard; 
            /*    not stable enough - use 0.1uf cap - argh lazy 
            Serial.print("direct reading tempoPin: ");
            Serial.println(analogRead(tempoPin)); // and show a responsive version!
            Serial.print("clockbpmtime: ");
            Serial.println(clockbpmtime);
            Serial.print("BPM: ");
            Serial.println(60000/(clockbpmtime/1000));
            Serial.print("devision: ");
            Serial.println(devisionValue);
            Serial.print("fraction: ");
            Serial.println(fraction);
            
            Serial.println("seqEndValue + constrain((devisionValue - 8), 0, 8) = ");
            calculation = seqEndValue + constrain((devisionValue - 8), 0, 8);
            Serial.println(calculation);
            
            counterr = 3000; */
        }
    } 
    
}

