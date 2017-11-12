
// at the moment this sequencer incorporates some bad math
// this gives some interesting counter intuitive results

// todo
// 1K Resistor on all muteleds
// resetAmountOfTicks (how many ticks per reset)
// try responsive analog reading for pots

#include <Audio.h>
#include <Wire.h>
#include <SerialFlash.h>
#include <Bounce2.h>



// #include <Arduino.h> // check om det hjælper på ledDelayTime print her?
// eller skal jeg bruge inline voids og static inline int eksempelvis
#include "Settings.h"
#include "Leds.h"



// GUItool: begin automatically generated code
AudioSynthSimpleDrum     drum2;          //xy=152.99999618530273,173.0000228881836
AudioSynthSimpleDrum     drum1;          //xy=153.22221755981445,138.2222194671631
AudioSynthSimpleDrum     drum3;          //xy=153.66666412353516,211.00000190734863
AudioSynthSimpleDrum     drum5;          //xy=155,294
AudioSynthSimpleDrum     drum4;          //xy=159,254
AudioSynthSimpleDrum     drum7;          //xy=163,375
AudioSynthSimpleDrum     drum6;          //xy=165,336
AudioSynthSimpleDrum     drum8;          //xy=168,431
AudioMixer4              mixer1;         //xy=442.99998474121094,197.33333206176758
AudioMixer4              mixer2;         //xy=488.77782440185547,384.2222442626953
AudioEffectReverb        reverb1;        //xy=579.2222442626953,197.22226333618164
AudioEffectReverb        reverb2;        //xy=629.1111373901367,383.1111030578613
AudioMixer4              mixer6;         //xy=957.555606842041,438.3333578109741
AudioMixer4              mixer5;         //xy=959.5555953979492,366.3333501815796
AudioMixer4              mixer3;         //xy=964.6666984558105,186.88892364501953
AudioMixer4              mixer4;         //xy=966.5556221008301,275.33334827423096
AudioMixer4              mixer7;         //xy=1200.7777862548828,222.2222385406494
AudioMixer4              mixer8;         //xy=1211.8889656066895,333.55559158325195
AudioOutputAnalogStereo  dacs1;          //xy=1340.5557174682617,269.3333787918091
AudioConnection          patchCord1(drum2, 0, mixer2, 0);
AudioConnection          patchCord2(drum2, 0, mixer5, 1);
AudioConnection          patchCord3(drum1, 0, mixer1, 0);
AudioConnection          patchCord4(drum1, 0, mixer3, 1);
AudioConnection          patchCord5(drum3, 0, mixer1, 1);
AudioConnection          patchCord6(drum3, 0, mixer3, 2);
AudioConnection          patchCord7(drum5, 0, mixer1, 2);
AudioConnection          patchCord8(drum5, 0, mixer3, 3);
AudioConnection          patchCord9(drum4, 0, mixer2, 1);
AudioConnection          patchCord10(drum4, 0, mixer5, 2);
AudioConnection          patchCord11(drum7, 0, mixer1, 3);
AudioConnection          patchCord12(drum7, 0, mixer4, 0);
AudioConnection          patchCord13(drum6, 0, mixer2, 2);
AudioConnection          patchCord14(drum6, 0, mixer5, 3);
AudioConnection          patchCord15(drum8, 0, mixer2, 3);
AudioConnection          patchCord16(drum8, 0, mixer6, 0);
AudioConnection          patchCord17(mixer1, reverb1);
AudioConnection          patchCord18(mixer2, reverb2);
AudioConnection          patchCord19(reverb1, 0, mixer3, 0);
AudioConnection          patchCord20(reverb2, 0, mixer5, 0);
AudioConnection          patchCord21(mixer6, 0, mixer8, 1);
AudioConnection          patchCord22(mixer5, 0, mixer8, 0);
AudioConnection          patchCord23(mixer3, 0, mixer7, 0);
AudioConnection          patchCord24(mixer4, 0, mixer7, 1);
AudioConnection          patchCord25(mixer7, 0, dacs1, 0);
AudioConnection          patchCord26(mixer8, 0, dacs1, 1);
// GUItool: end automatically generated code

int calculation = 0; // for Serial monitor debugging

// time guard stuff for quick scope
unsigned long previousTimeGuard = 0;
const long timeGuardInterval = 200;

int randomFreq; // ahh so lazy
int randomLength; // doubleble lazyzy
int randomSecondMix; //muhaha
int randomPitchMod; //aargahg lazlazlazzy

// const char *drums[8] = {"drum1", "drum2", "drum3", "drum4", "drum5", "drum6", "drum7", "drum8"};
//auto drum = AudioSynthSimpleDrum();

int flagAbuttonHaveBeenPressed = 0;

// seq stuff
//////////////////
//////////////////
//const unsigned int leds[] = {5, 6, 7, 8, 9, 10, 11, 12}; // leds indicating seq activity
//const unsigned int muteLeds[] = {32, 33, 34, 35, 36, 37, 38, 39}; //leds indicating wether a step is on or off
const unsigned int muteButtons[] = {31, 30, 29, 28, 27, 26, 25, 24};
Bounce debouncer[8] = {Bounce()}; //deboucing
// flipped order - convenient for breadboarding
int buttonState[8] = {1};
int lastButtonState[8] = {1};
int BUTTON_PRESSED = 0;
bool doStep[8] = {true, true, true, true, true, true, true, true};
bool flagHasHandledNoteOn[8] = {false, false, false, false, false, false, false, false};
const unsigned int tempoled = 13;
const unsigned int tempoOut = 23;

int exINT = 15;
float exFloat = 1.12345;
boolean exBoolean = true;
long exLong = 2123456789;
//boolean exBooleanArray[8] = {true, true, true, true, true, true, true, true};


//switch
//const unsigned int switchUp = 0;
//const unsigned int switchDown = 1;
//int switchDownTrig;
//int switchUpTrig;

int gateNr = 0; //sequencer inits
const unsigned int gateNrMap[] = {
    0,1,2,3,4,5,6,7,
    0,1,2,3,4,5,6,7,
    0,1,2,3,4,5,6,7,
    0,1,2,3,4,5,6,7 // max 4 times wrapping. maybe higher is needed.
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
int interruptCountToStepResetPot = A5;
int interruptCountToStepResetValue;

// internal clock
int tempoPin = A4;
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
    while(!Serial && millis() > 10000) {
        // wait for Serial but if more than 10k millis passed run program
    };
    
    AudioMemory(120); // check bench! maybe only 20 needed
    analogReference(EXTERNAL); // used INTERNAL but now EXT, will be adding some noise but quick fix for proper analog readings
    
    // reverb in
    mixer1.gain(0, 0.3);
    mixer1.gain(1, 0.3);
    mixer1.gain(2, 0.3);
    mixer1.gain(3, 0.3);
    mixer2.gain(0, 0.3);
    mixer2.gain(1, 0.3);
    mixer2.gain(2, 0.3);
    mixer2.gain(3, 0.3);
    //reverb mix l og r - wet
    mixer3.gain(0, 0.5);
    mixer5.gain(0, 0.5);
    
    // dry mix
    mixer3.gain(1, 2.0);
    mixer5.gain(1, 0.3);
    mixer3.gain(2, 0.2);
    mixer5.gain(2, 0.1);
    mixer3.gain(3, 1.7);
    mixer5.gain(3, 0.6);
    mixer4.gain(0, 0.1);
    mixer6.gain(0, 0.3);
    
    // end l-r mix (direct)
    mixer7.gain(0, 1);
    mixer7.gain(1, 1);
    mixer8.gain(0, 1);
    mixer8.gain(1, 1);
    
    reverb1.reverbTime(1);
    reverb2.reverbTime(1);
    
    // seq stuff
    pinMode(pinClock, INPUT);
    attachInterrupt(digitalPinToInterrupt(pinClock), irqClock, RISING);
    
    pinMode(tempoPin, INPUT);
    pinMode(tempoled, OUTPUT);
    pinMode(tempoOut, OUTPUT);
    
    for (int i = 0; i < 8; ++i) {
        pinMode(leds[i], OUTPUT);
    }
    
    for (int i = 0; i < 8; ++i) {
        pinMode(muteLeds[i], OUTPUT);
    }
    
    // Pullup for mute pins
    for (int i = 0; i < 8; ++i) {
        pinMode(muteButtons[i], INPUT_PULLUP);
    }
    // debounce setup
    for (int i = 0; i < 8; ++i) {
        debouncer[i].attach(muteButtons[i]);
        debouncer[i].interval(5);
    }
    
    
    //Serial.println("iam awake");
    LEDS_startUp();
    
    SD_init();
    //SD_checkTypeAndListRoot(); // DANGER DANGER
    //delay(2000);
    SD_readSettings(); // and apply to program parameters/variables
    //Serial.println("-----> I MANAGED TO READ SETTINGS AFTER INIT <-----");
    //apply loaded settings to UI (muteLeds)
    for (int i = 0; i < 8; ++i) {
        //doStep[i] = exBooleanArray[i];
        if (doStep[i] == true) {
            digitalWriteFast(muteLeds[i], HIGH);
            Serial.print("init muteLeds");
            Serial.print(i+1);
            Serial.println(" ON");
        }
        if (i == 7) {
            Serial.println("stored steps have been loaded into program");
        }
    }
    
    //SD_readAllSettings2Monitor();
}

void setStepState(unsigned int i) {
    debouncer[i].update();
    //buttonState[i] = digitalRead(muteButtons[i]); // classic no debounce
    buttonState[i] = debouncer[i].read();
    
    if(buttonState[i] != lastButtonState[i]) {
        if(buttonState[i] == BUTTON_PRESSED) {
            flagAbuttonHaveBeenPressed = 1;
            if(doStep[i] == false) {
                doStep[i] = true;
                digitalWriteFast(muteLeds[i], HIGH);
                //Serial.println("true");
            } else if(doStep[i] == true){
                doStep[i] = false;
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

void handleNoteOn(unsigned int i) {
    if (i == 0 && doStep[i] == true) {
        LEDS_on(i);
        // digitalWriteFast(leds[i], HIGH);
        flagHasHandledNoteOn[i] = true;
        drum1.noteOn();
    }
    if (i == 1 && doStep[i] == true) {
        LEDS_on(i);
        // digitalWriteFast(leds[i], HIGH);
        flagHasHandledNoteOn[i] = true;
        drum2.noteOn();
    }
    if (i == 2 && doStep[i] == true) {
        LEDS_on(i);
        // digitalWriteFast(leds[i], HIGH);
        flagHasHandledNoteOn[i] = true;
        drum3.noteOn();
    }
    if (i == 3 && doStep[i] == true) {
        LEDS_on(i);
        // digitalWriteFast(leds[i], HIGH);
        flagHasHandledNoteOn[i] = true;
        drum4.noteOn();
    }
    if (i == 4 && doStep[i] == true) {
        LEDS_on(i);
        // digitalWriteFast(leds[i], HIGH);
        flagHasHandledNoteOn[i] = true;
        drum5.noteOn();
    }
    if (i == 5 && doStep[i] == true) {
        LEDS_on(i);
        // digitalWriteFast(leds[i], HIGH);
        flagHasHandledNoteOn[i] = true;
        drum6.noteOn();
    }
    if (i == 6 && doStep[i] == true) {
        LEDS_on(i);
        // digitalWriteFast(leds[i], HIGH);
        flagHasHandledNoteOn[i] = true;
        drum7.noteOn();
    }
    if (i == 7 && doStep[i] == true) {
        LEDS_on(i);
        // digitalWriteFast(leds[i], HIGH);
        flagHasHandledNoteOn[i] = true;
        drum8.noteOn();
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
        clockbpmtime = map(clockbpmtime, 22, 1023, 4000000, 200000);
        
        //internal clock hack
        if (clocktime >= clockbpmtime) {
            digitalWriteFast(tempoled, HIGH);
            digitalWriteFast(tempoOut, HIGH);
            delay(1); // remove delay with nice counter delay
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
        interruptCountToStepResetValue = map(interruptCountToStepResetValue, 90, 1023, 1, 8);
        
        devisionValue = map(devisionValue, 90, 1023, 1, 16);
        
        // Offset into ->> seqStart and seqEnd
        seqStartValue = seqStartValue+seqOffsetValue;
        seqEndValue = seqEndValue+seqOffsetValue;
        
        // control af fraction
        // old one
        //float fraction = 1.0/devisionValue;
        
        // new one
        // dividend (faster)
        dividendValue = analogRead(devisionPot);
        dividendValue = constrain(map(dividendValue, 550, 1023, 1, 16), 1, 16);

        // devisor (slower)
        divisorValue = analogRead(devisionPot);
        divisorValue = constrain(map(divisorValue, 90, 450, 16, 1), 1, 16);

        float fraction = 1.0 / (dividendValue / divisorValue);
        
        
        // check if steps are on or off
        for (int i = 0; i < 8; ++i) {
            setStepState(i);
            SD_writeSettings(i);
            flagAbuttonHaveBeenPressed = 0; // burde denne være inde i SD_writeSettings
        }
        
        //tracker
        if (flag /*&& (debouncer > debouncetime)*/) {
            
            t1 = (float(microtime) * fraction);
            microtime = 0;
            microbeattime = 0;
            flag = false;

            
            
            //extended "sync" part of tracking
            if (interruptCountToStepReset >= interruptCountToStepResetValue) {
                resettracker = true;
                gateNr = seqStartValue;
                Serial.println("¤¤¤¤¤¤¤¤¤¤");
                Serial.println("¤¤¤¤¤¤¤¤¤¤");
                Serial.println("step reset");
                interruptCountToStepReset = 0;
            }
            //gateNr = seqStartValue;
            
            // fraction kan bruges i en funktion sådan at det kan
            // kontrolleres hvor ofte sequenceren sætter seqStartValue til gateNr
            // men der bliver jo tænkt lidt anderldes om dette i seq delen nedenunder .
            // if gate er større end endvalue set gate til seqstartvalue

            interruptCountToStepReset++;
        }
        
        //sequencer
        if (microbeattime >= t1 || resettracker) {
            
            /* length of seq
            if(gateNr >= seqEndValue) {
                gateNr = seqStartValue;
            } */
            
            if(gateNr >= 1 + seqEndValue + constrain((devisionValue - 8), 0, 8) ) {
                gateNr = seqStartValue; // trying to control wierd foldback/over behavior
            }
            
            Serial.print("###########");
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

            
            // sounds      
            randomFreq = random(0, 20);
            randomLength = random(20, 100);
            randomSecondMix = random(0, 100);
            randomPitchMod = random(0, 100);
            
            // settings for drums
            if(gateNrMap[gateNr] == 0){
                drum1.frequency(70);
                drum1.length((randomLength/2)*10);
                drum1.secondMix(randomSecondMix/100);
                drum1.pitchMod(0.6);
            }
            if(gateNrMap[gateNr] == 1){
                drum2.frequency(1000+randomFreq);
                drum2.length(randomLength/6);
                drum2.secondMix(0);
                drum2.pitchMod(0.9);
            }
            if(gateNrMap[gateNr] == 2){
                drum3.frequency(randomFreq*200);
                drum3.length(randomLength/2);
                drum3.secondMix(0);
                drum3.pitchMod(0.5);
            }
            if(gateNrMap[gateNr] == 3){
                drum4.frequency(5000);
                drum4.length(randomLength/3);
                drum4.secondMix(0);
                drum4.pitchMod(0.45);
            }
            if(gateNrMap[gateNr] == 4){
                drum5.frequency(150+randomFreq);
                drum5.length(randomLength*2);
                drum5.secondMix(0);
                drum5.pitchMod(0.5);
            }
            if(gateNrMap[gateNr] == 5){
                drum6.frequency(120);
                drum6.length(randomLength/2);
                drum6.secondMix(randomSecondMix/100);
                drum6.pitchMod(0.7);
            }
            if(gateNrMap[gateNr] == 6){
                drum7.frequency(9000+randomFreq);
                drum7.length(randomLength/2);
                drum7.secondMix(randomSecondMix/100);
                drum7.pitchMod(0.2);
            }
            if(gateNrMap[gateNr] == 7){
                drum8.frequency(1900);
                drum8.length(randomLength/4);
                drum8.secondMix(randomSecondMix/100);
                drum8.pitchMod(0.3);
            }
            
            // noteOn
            // make step
            handleNoteOn(gateNrMap[gateNr]);
            
            // seq
            ++gateNr;
            microbeattime = 0;
            resettracker = false;
        }
        
        // checking all every cycle, humans are slow
        for (int i = 0; i < 8; ++i){
            //handleLedDelay(i);
            LEDS_off(i);   
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

