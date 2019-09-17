// hallo
// at the moment this sequencer incorporates some bad math
// this gives some interesting counter intuitive results

// todo
// resetAmountOfTicks (how many ticks per reset)
// try responsive analog reading for pots

// ClickEncoder.h instead of Encoder.h?



// ****************************************
    // HUSK caps på pots !
// *****************************************


// std::random_shuffle
// PImpl
// tupedefs

#include <Wire.h>
#include <SerialFlash.h>

#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>

Encoder devisionEncoder(9, 10);
Encoder interruptCountToStepResetEncoder(11, 12);


// #include <Arduino.h> // check om det hjælper på ledDelayTime print her?
// eller skal jeg bruge inline voids og static inline int eksempelvis
//#include "Layout.h"
#include "Settings.h"
#include "Leds.h"
#include "Switches.h"
#include "Display.h"

muteButtons stepButtons;

Display seqDisplay;

extern bool debug;
extern unsigned int ledDelayTime;

//extern unsigned int stepCount;
//extern unsigned int trackCount;

int calculation = 0; // for Serial monitor debugging, math helper

// time guard stuff for quick scope
unsigned long previousTimeGuard = 0;
const long timeGuardInterval = 1000;

// layout
unsigned int stepCount = 16;
unsigned int trackCount = 5;

// seq stuff
//////////////////
//////////////////

int BUTTON_PRESSED = 0;
int flagAbuttonWasPressed = 0;

int buttonState[16] = {1};
int lastButtonState[16] = {1};

const unsigned int numEncoders = 2;
const unsigned int encoderButtonPins[] = {3,17};
Bounce encoderButtons[2] = {Bounce()};
int encoderButtonState[2] = {1};
int lastEncoderButtonState[2] = {1};
int devisionButtonLatch = false;

// multi dimensional arrays for track logic (5 tracks)
bool doStep[5][16] = {
    {true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true},
    {true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true},
    {true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true},
    {true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true},
    {true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true}
};
bool flagHasHandledNoteOn[5][16] = {
    {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
    {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
    {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
    {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
    {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false}
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
const unsigned int pinsTrack[5] = {4,5,6,7,20}; // track outputs, going to voltage devider and jack out
unsigned int trackStepIterationTrigDevider[5] = {1,3,2,8,6};
unsigned int trackStepIterationClockCounter[5] = {1,1,1,1,1}; // they iterate from 1
bool madeTrigFlag[5] = {false,false,false,false,false};
unsigned int triggerHoldTime = 100; // the actual gate time of outputs
unsigned int triggerHoldTimeCounter[5] = {triggerHoldTime}; 


int gateNr = 0; //sequencer inits
const unsigned int gateNrMap[] = {
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 // max 4 times wrapping. 
    // maybe higher is needed due to interruptCountToStepReset
}; // translation map for maximum 16th devisions

int lastGateNr; // for playhead stepLeds.


// control declarations/definitions
// seq length and offset
int seqStartPotPin = A0;
int seqEndPotPin = A1;
int seqOffsetPin = A2;
int seqStartValue;
int seqEndValue;
int seqOffsetValue;

// fraction
//int devisionPot = A3;
int devisionReading;
int devisionValue;

unsigned int dividendValue;
unsigned int divisorValue;

// experimental
//int interruptCountToStepResetPot = A4;
int interruptCountToStepResetValue;

// internal clock
int tempoPin = A5;
float clocktimeGuard;

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
    while(!Serial && millis() < 2000) {};

    LEDS_setup();
    
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
    
    // for (unsigned int i = 0; i < stepCount; ++i) {
    //     pinMode(leds[i], OUTPUT);
    // }
    
    // for (unsigned int i = 0; i < stepCount; ++i) {
    //     pinMode(muteLeds[i], OUTPUT);
    // }
    
    stepButtons.setup();
    seqDisplay.setup();
   

    // Pullup for encoder button pins
    for (unsigned int i = 0; i < numEncoders; ++i) {
        pinMode(encoderButtonPins[i], INPUT_PULLUP);
    }

    //track page switch (5 tracks, one for each sds-8 drum)
    pinMode(switchUpPin, INPUT_PULLUP);
    pinMode(switchDownPin, INPUT_PULLUP);

    debounceUpTrig.attach(switchUpPin);
    debounceUpTrig.interval(5);
    debounceDownTrig.attach(switchDownPin);
    debounceDownTrig.interval(5);

    for (unsigned int i = 0; i < numEncoders; ++i) {
        encoderButtons[i].attach(encoderButtonPins[i]);
        encoderButtons[i].interval(5);
    }
    
    devisionEncoder.write(16); // init
    interruptCountToStepResetEncoder.write(4); // init

    SD_init();
    SD_readSettings(); // and apply to program parameters/variables

    //apply loaded settings of selected track to UI (muteLeds)
    for (unsigned int i = 0; i < stepCount; ++i) {
        if (doStep[selectedTrack][i] == true) {
            //digitalWriteFast(muteLeds[i], HIGH);
            MUTELEDS_on(i);
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

    LEDS_startUp(); // all set, ready to go, flash some
}

void updateEncoderButtonStates() {
    for (unsigned int i = 0; i < numEncoders; ++i) {
        encoderButtons[i].update();
        encoderButtonState[i] = encoderButtons[i].read();
    }
}

void updateMuteLedsDueToTrackSelection() {
    //if (selectedTrack != lastSelectedtrack) { // breaks something
        for (int i = 0; i < 16; ++i) {
            if(doStep[selectedTrack][i] == true) {
                MUTELEDS_on(i);
            } else if(doStep[selectedTrack][i] == false){
                MUTELEDS_off(i);
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
            
            // track trigger devider
            if (trackStepIterationTrigDevider[track] == trackStepIterationClockCounter[track]) {
                makeTrigger(track);
                
            }
            
            // iterate
            trackStepIterationClockCounter[track]++;
            
            // if counter is bigger than set trigDevider 
            if (trackStepIterationClockCounter[track] > trackStepIterationTrigDevider[track]) {
                //reset counter for track
                trackStepIterationClockCounter[track] = 1;
            }   
        }
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


void setStepState(unsigned int i) {
    //debouncer[i].update();
    //buttonState[i] = debouncer[i].read();
    
    if(buttonState[i] != lastButtonState[i]) {
        if(buttonState[i] == BUTTON_PRESSED) {
            flagAbuttonWasPressed = 1;
            if(doStep[selectedTrack][i] == false) {
                doStep[selectedTrack][i] = true;
                MUTELEDS_on(i);
            } else if(doStep[selectedTrack][i] == true){
                doStep[selectedTrack][i] = false;
                MUTELEDS_off(i);
            }
        }
        lastButtonState[i] = buttonState[i];
    }
}


// should it be switch case logic baby?
void handleSeqTrigLedsOn(unsigned int gateNr) {
    if (gateNr == 0 && doStep[selectedTrack][gateNr] == true) {
        if(madeTrigFlag[selectedTrack]) {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 1 && doStep[selectedTrack][gateNr] == true) {
        if(madeTrigFlag[selectedTrack]) {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 2 && doStep[selectedTrack][gateNr] == true) {
        if(madeTrigFlag[selectedTrack]) {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 3 && doStep[selectedTrack][gateNr] == true) {
        if(madeTrigFlag[selectedTrack]) {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 4 && doStep[selectedTrack][gateNr] == true) {
        if(madeTrigFlag[selectedTrack]) {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 5 && doStep[selectedTrack][gateNr] == true) {
        if(madeTrigFlag[selectedTrack]) {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 6 && doStep[selectedTrack][gateNr] == true) {
        if(madeTrigFlag[selectedTrack]) {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 7 && doStep[selectedTrack][gateNr] == true) {
        if(madeTrigFlag[selectedTrack]) {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 8 && doStep[selectedTrack][gateNr] == true) {
        if(madeTrigFlag[selectedTrack]) {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 9 && doStep[selectedTrack][gateNr] == true) {
        if(madeTrigFlag[selectedTrack]) {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 10 && doStep[selectedTrack][gateNr] == true) {
        if(madeTrigFlag[selectedTrack]) {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 11 && doStep[selectedTrack][gateNr] == true) {
        if(madeTrigFlag[selectedTrack]) {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 12 && doStep[selectedTrack][gateNr] == true) {
        if(madeTrigFlag[selectedTrack]) {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 13 && doStep[selectedTrack][gateNr] == true) {
        if(madeTrigFlag[selectedTrack]) {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 14 && doStep[selectedTrack][gateNr] == true) {
        if(madeTrigFlag[selectedTrack]) {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 15 && doStep[selectedTrack][gateNr] == true) {
        if(madeTrigFlag[selectedTrack]) {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
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

        // clean up step leds, for now still just called LEDS_ ... messy head
        for (unsigned int i = 0; i < stepCount; ++i) {
            LEDS_clear(i);            
        }
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

        // zz. messy msss
        for (unsigned int i = 0; i < stepCount; ++i) {
            LEDS_clear(i);
        }
    }
}


void updateDevisionReadings() {
    unsigned int pointer = 0; // devisionEncoder is encoder button 0 aka first in array.. messy
    
    
        if ((encoderButtonState[pointer] != lastEncoderButtonState[pointer]) && (encoderButtonState[pointer] == BUTTON_PRESSED)) {
            if (!devisionButtonLatch) {
                devisionButtonLatch = true;
                if (debug) {
                    Serial.println("first encoder is in track gate counter set modus");
                    devisionEncoder.write(trackStepIterationTrigDevider[selectedTrack]); //is something
                }
            } else {
                devisionButtonLatch = false;
                if (debug) {
                    Serial.println("first encoder is in devision modus");
                    devisionEncoder.write(devisionReading); // get division
                }

            }
            
        }
        lastEncoderButtonState[pointer] = encoderButtonState[pointer];

    
    
    
    switch (devisionButtonLatch)
    {
    case true: // pressed
        // track devision of selected track
        trackStepIterationTrigDevider[selectedTrack] = devisionEncoder.read();
        break;
    
    default:
        // global devision
        devisionReading = devisionEncoder.read();
        break;
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

        seqDisplay.update();
        
        clocktimeGuard = analogRead(tempoPin); // try responsive version?
        clocktimeGuard = map(clocktimeGuard, 0, 1023, 6000000, 100000); // det var 4e6, 2e5
        
        //internal clock hack
        if (clocktime >= clocktimeGuard) {
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

        seqStartValue = constrain(map(seqStartValue, 0, 1010, 0, 15), 0, 15);
        seqEndValue = constrain(map(seqEndValue, 0, 850, 0, 15), 0, 15);
        seqOffsetValue = map(seqOffsetValue, 0, 1000, 0, 15);

        updateEncoderButtonStates();
        updateDevisionReadings();
        devisionValue = floor(devisionReading/4)+1;

        if (devisionValue < 1){
            devisionValue = 1;
            devisionEncoder.write(1);
        }
        if (devisionValue > 16){
            devisionValue = 16;
            devisionEncoder.write(16);
        }
        //devisionValue = ceil(devisionValue);


        interruptCountToStepResetValue = interruptCountToStepResetEncoder.read();
        
        if (interruptCountToStepResetValue < 1){
            interruptCountToStepResetValue = 1;
            interruptCountToStepResetEncoder.write(1);
        }
        if (interruptCountToStepResetValue > 32){
            interruptCountToStepResetValue = 32;
            interruptCountToStepResetEncoder.write(32);
        }

        
        
        // Offset into ->> seqStart and seqEnd
        seqStartValue = seqStartValue+seqOffsetValue;
        seqEndValue = seqEndValue+seqOffsetValue;

        // set length of blinks made by seq trig leds
        LEDS_gateLength(devisionValue, trackStepIterationTrigDevider[selectedTrack]);


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

        stepButtons.update();
        
        // check if steps are on or off
        for (int i = 0; i < 16; ++i) {
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
            if(gateNr >= 1 + seqEndValue + constrain((devisionValue - 16), 0, 16) ) {
                gateNr = seqStartValue; // trying to control wierd fold-back/over behavior
            }
            
            // "seq head" aka the reds
            STEPLEDS_on(gateNrMap[gateNr]);
            STEPLEDS_off(lastGateNr);
            
            // debugging
            /*Serial.print("###########");
            Serial.println("seq debuggin");
            Serial.println("___________");
            Serial.print("gateNrMap: ");
            Serial.println(gateNrMap[gateNr]);
            Serial.print("gateNr: ");
            Serial.println(gateNr);
            */

            // noteOn
            // make step --> also here to implement probability, if (bla bla)
            
            checkTriggers(gateNrMap[gateNr]); // check and possibly send trigs out

            handleSeqTrigLedsOn(gateNrMap[gateNr]); // led handling and flag

            // seq
            lastGateNr = gateNrMap[gateNr];
            ++gateNr;
            microbeattime = 0;
            resettracker = false;
        }

        // checking the state of all triggers every cycle, humans are slow
        // handling TRIGGER "on" time
        for (int i = 0; i < 5; ++i){
            triggersOFF(i);
        }
        
        // checking the state of all leds every cycle
        // this is internally handling LED "on" time
        for (int i = 0; i < 16; ++i){
            // humans are slow
            LEDS_off(selectedTrack, i);   
        }
        
        if (debug) {
            // debugg serial print delay
            if (currentTimeGuard - previousTimeGuard >= timeGuardInterval) {
                //SD_readAllSettings2Monitor();
                
                Serial.println();
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

                Serial.println();
                Serial.print("seq pool trigger led delay: ");
                Serial.println(ledDelayTime);

                Serial.println();
                Serial.print("trackStepIterationTrigDeviders: ");
                for (unsigned int i = 0; i < trackCount; ++i) {
                    Serial.print(trackStepIterationTrigDevider[i]);
                    Serial.print(" ");
                }
                Serial.println();
                
                previousTimeGuard = currentTimeGuard;

                /*    not stable enough - use 0.1uf cap - argh lazy 
                Serial.print("direct reading tempoPin: ");
                Serial.println(analogRead(tempoPin)); // and show a responsive version!
                Serial.print("clocktimeGuard: ");
                Serial.println(clocktimeGuard);
                Serial.print("BPM: ");
                Serial.println(60000/(clocktimeGuard/1000));
                */
 
            }
        }
        
    } 
    
}
