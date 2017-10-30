#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

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


int randomFreq; // ahh so lazy
int randomLength; // doubleble lazyzy
int randomSecondMix; //muhaha
int randomPitchMod; //aargahg lazlazlazzy

//const unsigned int drum[] = {1,2,3,4,5,6,7,8}; // drums
/// hvordadn gør man dette egentlig?

// seq stuff
//////////////////
//////////////////
const unsigned int leds[] = {5, 6, 7, 8, 9, 10, 11, 12}; // leds indicating seq activity
const unsigned int tempoled = 13;
const unsigned int tempoOut = 23;


//switch
const unsigned int switchUp = 0;
const unsigned int switchDown = 1;
int switchDownTrig;
int switchUpTrig;

int gateNr = 0; //sequencer inits
const unsigned int gateNrMap[] = {0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7}; // translation map

// control definitions
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

// internal clock
int tempoPin = A4;
int clockbpmtime;

// CV control
int cvValuePotPin = A5;
int cvAssignDestinationStepButtonPin; // any digital pin
int cvOutValue;
int cvAssignDestinationStep;

//interrupt stuff
const int pinClock = 0; // external clock in i dette tilfælde en hacket internal clock ud og så ind her ...
//const int debouncetime = 1000; //debouncetid i mikrosekunder
volatile bool flag = false;
float fraction = 1.0/8.0; // 1/8 node  init devision

void irqClock(){
    flag = true;
}

void setup() {
    //Serial.begin(9600);

    AudioMemory(120);
    analogReference(INTERNAL);

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
}

void loop() {

    //tracker inits
    unsigned int t1 = 250000;
    elapsedMicros microtime;
    elapsedMicros microbeattime = 10000;
    //elapsedMicros debouncer = 0;
    bool resettracker = true;

    // internal clock inits
    elapsedMicros clocktime;

    //awesome scope
    while (1) {
        clockbpmtime = map(analogRead(tempoPin), 0 ,1023, 10000, 1000000);
        //internal clock
        if (clocktime >= clockbpmtime) {
            digitalWrite(tempoled, HIGH);
            digitalWrite(tempoOut, HIGH);
            delay(1);
            digitalWrite(tempoled, LOW);
            digitalWrite(tempoOut, LOW);
            clocktime = 0;
        }


        //seq controls
        seqStartValue = analogRead(seqStartPotPin);
        seqEndValue = analogRead(seqEndPotPin);
        seqOffsetValue = analogRead(seqOffsetPin);
        devisionValue = analogRead(devisionPot);
        //cv control
        cvOutValue = analogRead(cvValuePotPin);
        cvAssignDestinationStep = digitalRead(cvAssignDestinationStepButtonPin);
        //control scaling
        seqStartValue = map(seqStartValue, 0, 1010, 0, 7);
        seqEndValue = map(seqEndValue, 0, 850, 0, 7);
        seqOffsetValue = map(seqOffsetValue, 0, 1000, 0, 7);
        devisionValue = map(devisionValue, 0, 1000, 1, 16); // max 16-dele , kan være højere.
        //- kan den laveste devisions værdi være 0.5 ?? aka der skal 2 ticks til et step i sequencen, etc .
        cvOutValue = map(cvOutValue, 0, 1023, 0, 1000); // value til cvOut - dac A14 - skal repræsenterer DC 0-3.3V (0-10V)?

        // få offset regnet med i sequence længden (seqStartValue+seqEndValue) ?
        seqStartValue = seqStartValue+seqOffsetValue;
        seqEndValue = seqEndValue+seqOffsetValue;

        // control af fraction . det funker men der kunne godt lige optimeres
        //på matematikken så den arbejder efter at det er en 8 step seq
        float fraction = 1.0/devisionValue;


        //tracker
        if (flag /*&& (debouncer > debouncetime)*/) {

            t1 = (float(microtime) * fraction);
            microtime = 0;
            microbeattime = 0;
            flag = false;
            resettracker = true;

            //extended "sync" part of tracking
            gateNr = seqStartValue;

            // fraction skal bruges i en funktion sådan at det kan
            //kontrolleres hvor ofte sequenceren sætter seqStartValue til gateNr
        }

        //sequencer
        if (microbeattime >= t1 || resettracker) {
            //length of seq
            if(gateNr >= seqEndValue) {
                gateNr = seqStartValue;
            }

            randomFreq = random(10, 2000);
            randomLength = random(20, 100);
            randomSecondMix = random(0, 100);
            randomPitchMod = random(0, 100);

            if(gateNrMap[gateNr] == 0){
                drum1.frequency(70);
                drum1.length(randomLength*10);
                drum1.secondMix(randomSecondMix/100);
                drum1.pitchMod(0.6);
                drum1.noteOn();
            }
            if(gateNrMap[gateNr] == 1){
                drum2.frequency(randomFreq/2+1000);
                drum2.length(randomLength/6);
                drum2.secondMix(randomSecondMix/100);
                drum2.pitchMod(randomPitchMod/100);
                drum2.noteOn();
            }
            if(gateNrMap[gateNr] == 2){
                drum3.frequency(randomFreq);
                drum3.length(randomLength/2);
                drum3.secondMix(randomSecondMix/100);
                drum3.pitchMod(randomPitchMod/100);
                drum3.noteOn();

            }
            if(gateNrMap[gateNr] == 3){
                drum4.frequency(5000);
                drum4.length(randomLength/3);
                drum4.secondMix(randomSecondMix/100);
                drum4.pitchMod(randomPitchMod/100);
                drum4.noteOn();

            }
            if(gateNrMap[gateNr] == 4){
                drum5.frequency(150+random(20));
                drum5.length(randomLength*2);
                drum5.secondMix(0);
                drum5.pitchMod(0.5);
                drum5.noteOn();

            }
            if(gateNrMap[gateNr] == 5){
                drum6.frequency(120);
                drum6.length(randomLength/2);
                drum6.secondMix(randomSecondMix/100);
                drum6.pitchMod(0.7);
                drum6.noteOn();

            }
            if(gateNrMap[gateNr] == 6){
                drum7.frequency(randomFreq+9000);
                drum7.length(randomLength/2);
                drum7.secondMix(randomSecondMix/100);
                drum7.pitchMod(0.8);
                drum7.noteOn();

            }
            if(gateNrMap[gateNr] == 7){
                drum8.frequency(1900);
                drum8.length(randomLength/4);
                drum8.secondMix(randomSecondMix/100);
                drum8.pitchMod(0.3);
                drum8.noteOn();

            }

            digitalWrite(leds[gateNrMap[gateNr]], HIGH);
            delay(1);
            digitalWrite(leds[gateNrMap[gateNr]], LOW);

            ++gateNr;
            microbeattime = 0;
            resettracker = false;
        }
    }

}
