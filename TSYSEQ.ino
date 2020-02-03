
/*___               __      _                __            ___              ___//
|                                                                               |
    hallo
    at the moment this sequencer incorporates some bad math and timing
    this gives some interesting counter intuitive results

    a proper non-accurate robot feel "offness" (swing? no!) 
|                                                    is naturally implemented
|                                                                               |
//___          _     _             __                ___        __          ___*/


// newest discovery bug -> if resetcount is 2 and div is 8 and clock is slow (just to be able to see) 
// seq freezes for one step when the clock that does not resetcounter comes in... shitty?? or what happens here
// it does not happen repeatedly.. its not universal truth...? hmmm


#include <Wire.h>
#include <SerialFlash.h>

#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>

Encoder devisionEncoder(9, 10);
Encoder interruptCountToStepResetEncoder(11, 12);

#define FORWARDS 1
#define BACKWARDS 0
bool poolDirection = 1; // init forwards

bool trackerDebug = false;

// #include <Arduino.h> // check om det hjælper på ledDelayTime print her?
// eller skal jeg bruge inline voids og static inline int eksempelvis
//#include "Layout.h"
// #include "Settings.h"
#include "UISettings.h"
#include "Leds.h"
#include "Switches.h"
#include "Display.h"

muteButtons stepButtons; // instance of switches.h buttons
Touch trackButtons;      // instance of Touch class
Display seqDisplay;      // instance of Display class

bool hang = false; // hang/pause all operations, for ex. pin I/O work
extern bool debug;
extern unsigned int ledDelayTime;
extern bool trackMuteLatch[5];

int calculation = 0; // for Serial monitor debugging, math helper

// time guard stuff for quick scope
unsigned long previousTimeGuard = 0;
const long timeGuardInterval = 1000;

// layout
unsigned int stepCount = 16;
unsigned int trackCount = 5;

//////////////////
//////////////////

int BUTTON_PRESSED = 0;
int flagAbuttonWasPressed = 0;

int buttonState[16] = {1};
int lastButtonState[16] = {1};

const unsigned int numEncoders = 2;
const unsigned int encoderButtonPins[] = {3, 29};
Bounce encoderButtons[2] = {Bounce()};
int encoderButtonState[2] = {1};
int lastEncoderButtonState[2] = {1};
int devisionButtonLatch = false; // why these not bool? common!
int interruptCountButtonLatch = false;

// multi dimensional arrays for track logic (5 tracks)
bool doStep[5][16] = {
    {true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true},
    {true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true},
    {true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true},
    {true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true},
    {true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true}};
bool flagHasHandledNoteOn[5][16] = {
    {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
    {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
    {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
    {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
    {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false}};

// internal clock/ tempo pin
const unsigned int tempoOut = 23; // used for tempo input to the interrupt pin.
unsigned long previousTempoOutTime = 0;
unsigned int tempoOutHoldTime = 1;
bool tempoOutFlag = false;

//switch for track selection, 5 tracks in all
int selectedTrack = 0; // init track is 0, not part of SD_writeSettings yet..
int lastSelectedtrack = 0;

const unsigned int switchUpPin = 22;
const unsigned int switchDownPin = 21;
Bounce debounceUpTrig = Bounce();   //deboucing
Bounce debounceDownTrig = Bounce(); //deboucing
int switchUpTrig;
int switchUpState = 1;
int lastSwitchUpState = 1;
int switchDownTrig;
int switchDownState = 1;
int lastSwitchDownState = 1;

// trigger outputs
const unsigned int pinsTrack[5] = {4, 5, 6, 7, 20}; // track outputs, going to voltage devider -> protection diode -> jack out
unsigned int trackStepIterationTrigDevider[5] = {1, 3, 2, 8, 6};
unsigned int trackStepIterationClockCounter[5] = {1, 1, 1, 1, 1}; // they iterate from 1
bool madeTrigFlag[5] = {false, false, false, false, false};
unsigned int triggerHoldTime = 100; // the actual gate time of outputs
unsigned int triggerHoldTimeCounter[5] = {triggerHoldTime};

int gateNr = 0; //sequencer inits
const unsigned int gateNrMap[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 // max 4 times wrapping.
    // maybe higher is needed due to interruptCountToStepReset
}; // translation map for maximum 16th devisions

int lastGateNr; // for playhead stepLeds.

// control declarations/definitions
// seq length and offset
int seqStartPotPin = A0;
int seqEndPotPin = A19;
int seqOffsetPin = A20;
int seqStartValue;
int seqEndValue;
int seqOffsetValue;

// fraction
int devisionValue;

//( not used in this version
//unsigned int dividendValue;
//unsigned int divisorValue;

// experimental
//int interruptCountToStepResetPot = A4;
int interruptCountToStepResetValue;

// internal clock
int tempoPin = A13;
float clocktimeGuard;

// CV control
//int cvValuePotPin = A5;
//int cvAssignDestinationStepButtonPin; // any digital pin
//int cvOutValue;
//int cvAssignDestinationStep;

//interrupt stuff
const int pinClock = 0;               // external clock in i dette tilfælde en hacket internal clock ud og så ind her ...
const int trackerDebounceTime = 1000; //debouncetid i mikrosekunder
volatile bool flag = false;
int interruptCountToStepReset = 0;
float fraction = 1.0 / 8.0; // 1/8 node init devision

void irqClock()
{
    /*     if (interruptCountToReset) {
        flag = true;
        interruptCountToReset = 0;
    } */
    flag = true;
}

void setup()
{

    if (hang)
    {
        while (1)
        {
        }; // pauser for pin I/O work
    }

    if (debug)
    {
        Serial.begin(9600);
        //Serial.begin(19200);
        while (!Serial && millis() < 4000)
        {
        };
    }

    LEDS_setup();

    analogReference(EXTERNAL);
    // used INTERNAL but now EXT,
    // will be adding some noise but quick fix for proper analog readings

    // seq stuff
    pinMode(pinClock, INPUT);
    attachInterrupt(digitalPinToInterrupt(pinClock), irqClock, RISING);

    pinMode(tempoPin, INPUT);
    pinMode(tempoOut, OUTPUT);

    for (unsigned int i = 0; i < trackCount; ++i)
    {
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
    for (unsigned int i = 0; i < numEncoders; ++i)
    {
        pinMode(encoderButtonPins[i], INPUT_PULLUP);
    }

    //track page switch (5 tracks, one for each sds-8 drum)
    pinMode(switchUpPin, INPUT_PULLUP);
    pinMode(switchDownPin, INPUT_PULLUP);

    debounceUpTrig.attach(switchUpPin);
    debounceUpTrig.interval(5);
    debounceDownTrig.attach(switchDownPin);
    debounceDownTrig.interval(5);

    for (unsigned int i = 0; i < numEncoders; ++i)
    {
        encoderButtons[i].attach(encoderButtonPins[i]);
        encoderButtons[i].interval(5);
    }

    devisionEncoder.write(16);                 // init, remember these are X4
    interruptCountToStepResetEncoder.write(4); // init

    //SD_init();
    // SD_readSettings(); // and apply to program parameters/variables

    // recall settings
    recallUISettings();
    // -> important wite data to encoders updates after recall of settings
    // because they update the values of divion and resetcounttostep in loop
    devisionEncoder.write(devisionValue * 4);                                   // init
    interruptCountToStepResetEncoder.write(interruptCountToStepResetValue * 4); // init

    // clear rom
    //clearEEPROM();

    //apply loaded settings of selected track to UI (muteLeds)
    for (unsigned int i = 0; i < stepCount; ++i)
    {
        if (doStep[selectedTrack][i] == true)
        {
            //digitalWriteFast(muteLeds[i], HIGH);
            MUTELEDS_on(i);
            if (debug)
            {
                Serial.print("init muteLeds");
                Serial.print(i + 1);
                Serial.println(" ON");
            }
        }
        if (debug)
        {
            if (i == (stepCount - 1))
            {
                Serial.println("stored steps have been loaded into program");
            }
        }
    }

    /* if (debug) {
        SD_readAllSettings2Monitor(); // debugging, dump all settings
    } */

    LEDS_startUp(); // all set, ready to go, flash some
}

void updateEncoderButtonStates()
{
    for (unsigned int i = 0; i < numEncoders; ++i)
    {
        encoderButtons[i].update();
        encoderButtonState[i] = encoderButtons[i].read();
    }
}

void updateMuteLedsDueToTrackSelection()
{
    //if (selectedTrack != lastSelectedtrack) { // breaks something
    for (int i = 0; i < 16; ++i)
    {
        if (doStep[selectedTrack][i] == true)
        {
            MUTELEDS_on(i);
        }
        else if (doStep[selectedTrack][i] == false)
        {
            MUTELEDS_off(i);
        }
    }
    lastSelectedtrack = selectedTrack;
    //}
}

void checkTriggers(unsigned int step)
{
    // check all tracks
    for (unsigned int track = 0; track < 5; ++track)
    {
        // new, hhm counting what math, hard.. hmm
        if (doStep[track][step] == true)
        {

            // track trigger devider
            if (trackStepIterationTrigDevider[track] == trackStepIterationClockCounter[track])
            {
                if (!trackMuteLatch[track])
                {
                    makeTrigger(track);
                }
            }

            // iterate
            trackStepIterationClockCounter[track]++;

            // if counter is bigger than set trigDevider
            if (trackStepIterationClockCounter[track] > trackStepIterationTrigDevider[track])
            {
                //reset counter for track
                trackStepIterationClockCounter[track] = 1;
            }
        }
    }
}

void makeTrigger(unsigned int i)
{
    digitalWriteFast(pinsTrack[i], HIGH);
    madeTrigFlag[i] = true;
}

void triggersOFF(unsigned int i)
{
    if (madeTrigFlag[i] == true)
    {
        if (triggerHoldTimeCounter[i]-- == 0)
        {
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

void setStepState(unsigned int i)
{
    //debouncer[i].update();
    //buttonState[i] = debouncer[i].read();

    if (buttonState[i] != lastButtonState[i])
    {
        if (buttonState[i] == BUTTON_PRESSED)
        {
            flagAbuttonWasPressed = 1;
            if (doStep[selectedTrack][i] == false)
            {
                doStep[selectedTrack][i] = true;
                MUTELEDS_on(i);
            }
            else if (doStep[selectedTrack][i] == true)
            {
                doStep[selectedTrack][i] = false;
                MUTELEDS_off(i);
            }
        }
        lastButtonState[i] = buttonState[i];
    }
}

// should it be switch case logic baby?
void handleSeqTrigLedsOn(unsigned int gateNr)
{
    if (gateNr == 0 && doStep[selectedTrack][gateNr] == true)
    {
        if (madeTrigFlag[selectedTrack])
        {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 1 && doStep[selectedTrack][gateNr] == true)
    {
        if (madeTrigFlag[selectedTrack])
        {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 2 && doStep[selectedTrack][gateNr] == true)
    {
        if (madeTrigFlag[selectedTrack])
        {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 3 && doStep[selectedTrack][gateNr] == true)
    {
        if (madeTrigFlag[selectedTrack])
        {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 4 && doStep[selectedTrack][gateNr] == true)
    {
        if (madeTrigFlag[selectedTrack])
        {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 5 && doStep[selectedTrack][gateNr] == true)
    {
        if (madeTrigFlag[selectedTrack])
        {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 6 && doStep[selectedTrack][gateNr] == true)
    {
        if (madeTrigFlag[selectedTrack])
        {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 7 && doStep[selectedTrack][gateNr] == true)
    {
        if (madeTrigFlag[selectedTrack])
        {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 8 && doStep[selectedTrack][gateNr] == true)
    {
        if (madeTrigFlag[selectedTrack])
        {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 9 && doStep[selectedTrack][gateNr] == true)
    {
        if (madeTrigFlag[selectedTrack])
        {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 10 && doStep[selectedTrack][gateNr] == true)
    {
        if (madeTrigFlag[selectedTrack])
        {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 11 && doStep[selectedTrack][gateNr] == true)
    {
        if (madeTrigFlag[selectedTrack])
        {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 12 && doStep[selectedTrack][gateNr] == true)
    {
        if (madeTrigFlag[selectedTrack])
        {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 13 && doStep[selectedTrack][gateNr] == true)
    {
        if (madeTrigFlag[selectedTrack])
        {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 14 && doStep[selectedTrack][gateNr] == true)
    {
        if (madeTrigFlag[selectedTrack])
        {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
    if (gateNr == 15 && doStep[selectedTrack][gateNr] == true)
    {
        if (madeTrigFlag[selectedTrack])
        {
            LEDS_on(gateNr);
            flagHasHandledNoteOn[selectedTrack][gateNr] = true;
        }
    }
}

void setDirection()
{
    debounceUpTrig.update();
    debounceDownTrig.update();
    switchUpState = debounceUpTrig.read();
    switchDownState = debounceDownTrig.read();

    if (switchUpState != lastSwitchUpState)
    {
        if (switchUpState == BUTTON_PRESSED)
        {
            if (debug)
            {
                Serial.println("FORWARDS");
            }
            poolDirection = FORWARDS;
        }
        lastSwitchUpState = switchUpState;
    }

    if (switchDownState != lastSwitchDownState)
    {
        if (switchDownState == BUTTON_PRESSED)
        {
            if (debug)
            {
                Serial.println("BACKWARDS");
            }
            poolDirection = BACKWARDS;
        }
        lastSwitchDownState = switchDownState;
    }
}

void updateDevisionReadings()
{
    unsigned int pointer = 0; // devisionEncoder is encoder button 0 aka first in array.. messy

    // latching logic
    if ((encoderButtonState[pointer] != lastEncoderButtonState[pointer]) && (encoderButtonState[pointer] == BUTTON_PRESSED))
    {
        if (!devisionButtonLatch)
        {
            devisionButtonLatch = true;
            devisionEncoder.write(trackStepIterationTrigDevider[selectedTrack] * 4); //set encoder to val for state, X4 count
            if (debug)
            {
                Serial.println("division encoder is in track gate counter set modus");
            }
        }
        else
        {
            devisionButtonLatch = false;
            devisionEncoder.write(devisionValue * 4); // get division, set encoder to val for state, X4 count

            if (debug)
            {
                Serial.println("division encoder is in devision modus");
            }
        }
    }
    lastEncoderButtonState[pointer] = encoderButtonState[pointer];

    switch (devisionButtonLatch)
    {
    case true: // pressed
        // track devision of selected track
        trackStepIterationTrigDevider[selectedTrack] = devisionEncoder.read() >> 2; // shift to counter X4
        if (trackStepIterationTrigDevider[selectedTrack] < 1)
        {
            trackStepIterationTrigDevider[selectedTrack] = 1;
            devisionEncoder.write(1 * 4); // mult up to X4
        }
        if (trackStepIterationTrigDevider[selectedTrack] > 99)
        {
            trackStepIterationTrigDevider[selectedTrack] = 99;
            devisionEncoder.write(99 * 4); // mult up to X4
        }
        break;

    default:
        // global devision
        devisionValue = devisionEncoder.read() >> 2; // shift to counter X4

        // devisionValue borders
        if (devisionValue < 1)
        {
            devisionValue = 1;
            devisionEncoder.write(1 * 4); // mult up to X4
        }
        if (devisionValue > 32)
        {
            devisionValue = 32;
            devisionEncoder.write(32 * 4); // mult up to X4
        };
        break;
    }
}

void updateResetCountReading()
{

    /////////////////////////// BUTTON STUFF HERE FOR NOW. WHAA SHOULD BE IN Switches.h which should be called Interaction.h
    unsigned int pointer = 1; // interruptCount Encoder is encoder button 1 aka second in array.. messy

    // latching logic
    if ((encoderButtonState[pointer] != lastEncoderButtonState[pointer]) && (encoderButtonState[pointer] == BUTTON_PRESSED))
    {
        if (!interruptCountButtonLatch)
        {
            interruptCountButtonLatch = true;

            if (debug)
            {
                Serial.println("interruptCount encoder is setting trackselectors to mute latch modus");
            }
        }
        else
        {
            interruptCountButtonLatch = false;

            if (debug)
            {
                Serial.println("interruptCount encoder is setting trackselectors to normal modus");
            }
        }
    }
    lastEncoderButtonState[pointer] = encoderButtonState[pointer];
    ///////////////////////////////////////// end of something that should be its own void

    interruptCountToStepResetValue = interruptCountToStepResetEncoder.read() >> 2; // shift to counter X4

    if (interruptCountToStepResetValue < 1)
    {
        interruptCountToStepResetValue = 1;
        interruptCountToStepResetEncoder.write(1 * 4); // mult up to X4
    }
    if (interruptCountToStepResetValue > 99)
    {
        interruptCountToStepResetValue = 99;
        interruptCountToStepResetEncoder.write(99 * 4); // mult up to X4
    }
}

void loop()
{
    //tracker inits
    unsigned int t1 = 250000;
    elapsedMicros microtime;
    elapsedMicros microbeattime = 10000;
    elapsedMicros trackerDebouncer = 0;
    bool resettracker = true;

    // internal clock
    elapsedMicros clocktime;

    //quick scope
    while (1)
    {
        // time guard for quick scope
        unsigned long time = millis();

        seqDisplay.update(); // user feedback first

        clocktimeGuard = analogRead(tempoPin);                          // try responsive version?
        clocktimeGuard = map(clocktimeGuard, 0, 1023, 6000000, 100000); // det var 4e6, 2e5

        //internal clock hack
        ////// BEGIN
        if (clocktime >= clocktimeGuard)
        {
            digitalWriteFast(tempoOut, HIGH);
            tempoLedOn();
            clocktime = 0;
            tempoOutFlag = true;
        }

        if (tempoOutFlag)
        {
            if (time - previousTempoOutTime >= tempoOutHoldTime)
            {
                digitalWriteFast(tempoOut, LOW);
                tempoOutFlag = false;
                previousTempoOutTime = time;
            }
        }
        tempoLedOff();
        ////// END

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
        updateResetCountReading();

        // Offset into ->> seqStart and seqEnd
        seqStartValue = seqStartValue + seqOffsetValue;
        seqEndValue = seqEndValue + seqOffsetValue;

        // set length of blinks made by seq trig leds
        LEDS_gateLength(devisionValue, trackStepIterationTrigDevider[selectedTrack]);

        /* ############################################################# */
        /* ######################## FRACTION ########################## */
        /* ############################################################# */

        // control af fraction
        float fraction = 1.0 / devisionValue;
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
        trackButtons.update();
        trackButtons.trackSelector();

        updateMuteLedsDueToTrackSelection();

        stepButtons.update();

        // check if steps are on or off
        for (int i = 0; i < 16; ++i)
        {
            setStepState(i);
            //SD_writeSettings(i);
            flagAbuttonWasPressed = 0; // burde denne være inde i SD_writeSettings?
            // uanset så er det safety measures at sætte den til 0 hele tiden.
            // så skrives der kun én gang per knaptryk
        }

        storeUISettings();

        setDirection(); // upt direction for the gate iterator

        //tracker
        if (flag && (trackerDebouncer > trackerDebounceTime))
        {

            trackerDebouncer = 0;

            t1 = (float(microtime) * fraction);
            microtime = 0;
            microbeattime = 0;
            flag = false;

            //extended "sync" part of tracking
            // hvor mange clicks der skal til før at sekvensen resettes og retning på hovedet
            if (interruptCountToStepReset >= interruptCountToStepResetValue)
            {
                resettracker = true;
                if (poolDirection == FORWARDS)
                {
                    ++gateNr;
                }
                else if (poolDirection == BACKWARDS)
                {
                    --gateNr;
                    if (gateNr < 0)
                    {
                        gateNr = 15;
                    }
                }
                gateNr = seqStartValue;
                if (trackerDebug)
                {
                    Serial.println("¤_STEP_RESET_¤");
                }
                interruptCountToStepReset = 0;
            }

            interruptCountToStepReset++;
        }

        //sequencer
        if (microbeattime >= t1 || resettracker)
        {

            /* length of seq, old
            if(gateNr >= seqEndValue) {
                gateNr = seqStartValue;
            }*/
            // length of seq, new turn
            if (gateNr >= 1 + seqEndValue + constrain((devisionValue - 16), 0, 16))
            {
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

            // direction control
            if (poolDirection == FORWARDS)
            {
                ++gateNr;
            }
            else if (poolDirection == BACKWARDS)
            {
                --gateNr;
                if (gateNr < 0)
                {
                    gateNr = 15;
                }
            }

            microbeattime = 0;
            resettracker = false;
        }

        // checking the state of all triggers every cycle, humans are slow
        // handling TRIGGER "on" time
        for (int i = 0; i < 5; ++i)
        {
            triggersOFF(i);
        }

        // checking the state of all leds every cycle
        // this is internally handling LED "on" time
        for (int i = 0; i < 16; ++i)
        {
            // humans are slow
            LEDS_off(selectedTrack, i);
        }

        if (debug)
        {
            // debugg serial print delay
            if (time - previousTimeGuard >= timeGuardInterval)
            {
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
                for (unsigned int i = 0; i < trackCount; ++i)
                {
                    Serial.print(trackStepIterationTrigDevider[i]);
                    Serial.print(" ");
                }
                Serial.println();

                previousTimeGuard = time;

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
