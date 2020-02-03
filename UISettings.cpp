#include "UISettings.h"
#include "Leds.h"
#include <Arduino.h>
#include <EEPROM.h>
//#include "Layout.h"

bool flagSettingsHaveBeenRead = false;

bool debug = false; // serial monitor debug modus, should be dependant on serial modus (which yet is to be implemented)

extern unsigned int stepCount;
extern unsigned int trackCount;
int eeAddress = 0;

// timeguard for writing
unsigned long previousWriteTime = 0;
const long writeTimeGuardInterval = 5000;

struct UISettings
{
    bool savedDoStep11;
    bool savedDoStep12;
    bool savedDoStep13;
    bool savedDoStep14;
    bool savedDoStep15;
    bool savedDoStep16;
    bool savedDoStep17;
    bool savedDoStep18;
    bool savedDoStep19;
    bool savedDoStep110;
    bool savedDoStep111;
    bool savedDoStep112;
    bool savedDoStep113;
    bool savedDoStep114;
    bool savedDoStep115;
    bool savedDoStep116;
    bool savedDoStep21;
    bool savedDoStep22;
    bool savedDoStep23;
    bool savedDoStep24;
    bool savedDoStep25;
    bool savedDoStep26;
    bool savedDoStep27;
    bool savedDoStep28;
    bool savedDoStep29;
    bool savedDoStep210;
    bool savedDoStep211;
    bool savedDoStep212;
    bool savedDoStep213;
    bool savedDoStep214;
    bool savedDoStep215;
    bool savedDoStep216;
    bool savedDoStep31;
    bool savedDoStep32;
    bool savedDoStep33;
    bool savedDoStep34;
    bool savedDoStep35;
    bool savedDoStep36;
    bool savedDoStep37;
    bool savedDoStep38;
    bool savedDoStep39;
    bool savedDoStep310;
    bool savedDoStep311;
    bool savedDoStep312;
    bool savedDoStep313;
    bool savedDoStep314;
    bool savedDoStep315;
    bool savedDoStep316;
    bool savedDoStep41;
    bool savedDoStep42;
    bool savedDoStep43;
    bool savedDoStep44;
    bool savedDoStep45;
    bool savedDoStep46;
    bool savedDoStep47;
    bool savedDoStep48;
    bool savedDoStep49;
    bool savedDoStep410;
    bool savedDoStep411;
    bool savedDoStep412;
    bool savedDoStep413;
    bool savedDoStep414;
    bool savedDoStep415;
    bool savedDoStep416;
    bool savedDoStep51;
    bool savedDoStep52;
    bool savedDoStep53;
    bool savedDoStep54;
    bool savedDoStep55;
    bool savedDoStep56;
    bool savedDoStep57;
    bool savedDoStep58;
    bool savedDoStep59;
    bool savedDoStep510;
    bool savedDoStep511;
    bool savedDoStep512;
    bool savedDoStep513;
    bool savedDoStep514;
    bool savedDoStep515;
    bool savedDoStep516;
    bool savedTrackMuteLatch1;
    bool savedTrackMuteLatch2;
    bool savedTrackMuteLatch3;
    bool savedTrackMuteLatch4;
    bool savedTrackMuteLatch5;
    unsigned int savedTrackStepIterationTrigDevider1;
    unsigned int savedTrackStepIterationTrigDevider2;
    unsigned int savedTrackStepIterationTrigDevider3;
    unsigned int savedTrackStepIterationTrigDevider4;
    unsigned int savedTrackStepIterationTrigDevider5;
    bool savedPoolDirection;
    int savedInterruptCountToStepResetValue;
    int savedDevisionValue;
    int savedSelectedTrack;
};

void storeUISettings()
{
    unsigned long currentWriteTime = millis();

    UISettings list = {
        doStep[0][0],
        doStep[0][1],
        doStep[0][2],
        doStep[0][3],
        doStep[0][4],
        doStep[0][5],
        doStep[0][6],
        doStep[0][7],
        doStep[0][8],
        doStep[0][9],
        doStep[0][10],
        doStep[0][11],
        doStep[0][12],
        doStep[0][13],
        doStep[0][14],
        doStep[0][15],
        doStep[1][0],
        doStep[1][1],
        doStep[1][2],
        doStep[1][3],
        doStep[1][4],
        doStep[1][5],
        doStep[1][6],
        doStep[1][7],
        doStep[1][8],
        doStep[1][9],
        doStep[1][10],
        doStep[1][11],
        doStep[1][12],
        doStep[1][13],
        doStep[1][14],
        doStep[1][15],
        doStep[2][0],
        doStep[2][1],
        doStep[2][2],
        doStep[2][3],
        doStep[2][4],
        doStep[2][5],
        doStep[2][6],
        doStep[2][7],
        doStep[2][8],
        doStep[2][9],
        doStep[2][10],
        doStep[2][11],
        doStep[2][12],
        doStep[2][13],
        doStep[2][14],
        doStep[2][15],
        doStep[3][0],
        doStep[3][1],
        doStep[3][2],
        doStep[3][3],
        doStep[3][4],
        doStep[3][5],
        doStep[3][6],
        doStep[3][7],
        doStep[3][8],
        doStep[3][9],
        doStep[3][10],
        doStep[3][11],
        doStep[3][12],
        doStep[3][13],
        doStep[3][14],
        doStep[3][15],
        doStep[4][0],
        doStep[4][1],
        doStep[4][2],
        doStep[4][3],
        doStep[4][4],
        doStep[4][5],
        doStep[4][6],
        doStep[4][7],
        doStep[4][8],
        doStep[4][9],
        doStep[4][10],
        doStep[4][11],
        doStep[4][12],
        doStep[4][13],
        doStep[4][14],
        doStep[4][15],
        trackMuteLatch[0],
        trackMuteLatch[1],
        trackMuteLatch[2],
        trackMuteLatch[3],
        trackMuteLatch[4],
        trackStepIterationTrigDevider[0],
        trackStepIterationTrigDevider[1],
        trackStepIterationTrigDevider[2],
        trackStepIterationTrigDevider[3],
        trackStepIterationTrigDevider[4],
        poolDirection,
        interruptCountToStepResetValue,
        devisionValue,
        selectedTrack};

    if (currentWriteTime - previousWriteTime > writeTimeGuardInterval)
    {
        //Serial.println("saving UIdata if something changed");

        EEPROM.put(eeAddress, list);

        previousWriteTime = currentWriteTime;
    }
};

void recallUISettings()
{
    UISettings list;
    EEPROM.get(eeAddress, list);

    Serial.println("Read data from EEPROM: ");

    doStep[0][0] = list.savedDoStep11;
    doStep[0][1] = list.savedDoStep12;
    doStep[0][2] = list.savedDoStep13;
    doStep[0][3] = list.savedDoStep14;
    doStep[0][4] = list.savedDoStep15;
    doStep[0][5] = list.savedDoStep16;
    doStep[0][6] = list.savedDoStep17;
    doStep[0][7] = list.savedDoStep18;
    doStep[0][8] = list.savedDoStep19;
    doStep[0][9] = list.savedDoStep110;
    doStep[0][10] = list.savedDoStep111;
    doStep[0][11] = list.savedDoStep112;
    doStep[0][12] = list.savedDoStep113;
    doStep[0][13] = list.savedDoStep114;
    doStep[0][14] = list.savedDoStep115;
    doStep[0][15] = list.savedDoStep116;
    doStep[1][0] = list.savedDoStep21;
    doStep[1][1] = list.savedDoStep22;
    doStep[1][2] = list.savedDoStep23;
    doStep[1][3] = list.savedDoStep24;
    doStep[1][4] = list.savedDoStep25;
    doStep[1][5] = list.savedDoStep26;
    doStep[1][6] = list.savedDoStep27;
    doStep[1][7] = list.savedDoStep28;
    doStep[1][8] = list.savedDoStep29;
    doStep[1][9] = list.savedDoStep210;
    doStep[1][10] = list.savedDoStep211;
    doStep[1][11] = list.savedDoStep212;
    doStep[1][12] = list.savedDoStep213;
    doStep[1][13] = list.savedDoStep214;
    doStep[1][14] = list.savedDoStep215;
    doStep[1][15] = list.savedDoStep216;
    doStep[2][0] = list.savedDoStep31;
    doStep[2][1] = list.savedDoStep32;
    doStep[2][2] = list.savedDoStep33;
    doStep[2][3] = list.savedDoStep34;
    doStep[2][4] = list.savedDoStep35;
    doStep[2][5] = list.savedDoStep36;
    doStep[2][6] = list.savedDoStep37;
    doStep[2][7] = list.savedDoStep38;
    doStep[2][8] = list.savedDoStep39;
    doStep[2][9] = list.savedDoStep310;
    doStep[2][10] = list.savedDoStep311;
    doStep[2][11] = list.savedDoStep312;
    doStep[2][12] = list.savedDoStep313;
    doStep[2][13] = list.savedDoStep314;
    doStep[2][14] = list.savedDoStep315;
    doStep[2][15] = list.savedDoStep316;
    doStep[3][0] = list.savedDoStep41;
    doStep[3][1] = list.savedDoStep42;
    doStep[3][2] = list.savedDoStep43;
    doStep[3][3] = list.savedDoStep44;
    doStep[3][4] = list.savedDoStep45;
    doStep[3][5] = list.savedDoStep46;
    doStep[3][6] = list.savedDoStep47;
    doStep[3][7] = list.savedDoStep48;
    doStep[3][8] = list.savedDoStep49;
    doStep[3][9] = list.savedDoStep410;
    doStep[3][10] = list.savedDoStep411;
    doStep[3][11] = list.savedDoStep412;
    doStep[3][12] = list.savedDoStep413;
    doStep[3][13] = list.savedDoStep414;
    doStep[3][14] = list.savedDoStep415;
    doStep[3][15] = list.savedDoStep416;
    doStep[4][0] = list.savedDoStep51;
    doStep[4][1] = list.savedDoStep52;
    doStep[4][2] = list.savedDoStep53;
    doStep[4][3] = list.savedDoStep54;
    doStep[4][4] = list.savedDoStep55;
    doStep[4][5] = list.savedDoStep56;
    doStep[4][6] = list.savedDoStep57;
    doStep[4][7] = list.savedDoStep58;
    doStep[4][8] = list.savedDoStep59;
    doStep[4][9] = list.savedDoStep510;
    doStep[4][10] = list.savedDoStep511;
    doStep[4][11] = list.savedDoStep512;
    doStep[4][12] = list.savedDoStep513;
    doStep[4][13] = list.savedDoStep514;
    doStep[4][14] = list.savedDoStep515;
    doStep[4][15] = list.savedDoStep516;

    Serial.print("trackMuteLatch1:");
    Serial.println(list.savedTrackMuteLatch1);
    trackMuteLatch[0] = list.savedTrackMuteLatch1;
    Serial.print("trackMuteLatch2:");
    Serial.println(list.savedTrackMuteLatch2);
    trackMuteLatch[1] = list.savedTrackMuteLatch2;
    Serial.print("trackMuteLatch3:");
    Serial.println(list.savedTrackMuteLatch3);
    trackMuteLatch[2] = list.savedTrackMuteLatch3;
    Serial.print("trackMuteLatch4:");
    Serial.println(list.savedTrackMuteLatch4);
    trackMuteLatch[3] = list.savedTrackMuteLatch4;
    Serial.print("trackMuteLatch5:");
    Serial.println(list.savedTrackMuteLatch5);
    trackMuteLatch[4] = list.savedTrackMuteLatch5;

    Serial.print("trackStepIterationTrigDevider1: ");
    Serial.println(list.savedTrackStepIterationTrigDevider1);
    trackStepIterationTrigDevider[0] = list.savedTrackStepIterationTrigDevider1;
    Serial.print("trackStepIterationTrigDevider2: ");
    Serial.println(list.savedTrackStepIterationTrigDevider2);
    trackStepIterationTrigDevider[1] = list.savedTrackStepIterationTrigDevider2;
    Serial.print("trackStepIterationTrigDevider3: ");
    Serial.println(list.savedTrackStepIterationTrigDevider3);
    trackStepIterationTrigDevider[2] = list.savedTrackStepIterationTrigDevider3;
    Serial.print("trackStepIterationTrigDevider4: ");
    Serial.println(list.savedTrackStepIterationTrigDevider4);
    trackStepIterationTrigDevider[3] = list.savedTrackStepIterationTrigDevider4;
    Serial.print("trackStepIterationTrigDevider5: ");
    Serial.println(list.savedTrackStepIterationTrigDevider5);
    trackStepIterationTrigDevider[4] = list.savedTrackStepIterationTrigDevider5;

    Serial.print("poolDirection: ");
    Serial.println(list.savedPoolDirection);
    poolDirection = list.savedPoolDirection;
    Serial.print("interruptCountToStepResetValue: ");
    Serial.println(list.savedInterruptCountToStepResetValue);
    interruptCountToStepResetValue = list.savedInterruptCountToStepResetValue;
    Serial.print("devisionValue: ");
    Serial.println(list.savedDevisionValue);
    devisionValue = list.savedDevisionValue;
    Serial.print("selectedTrack: ");
    Serial.println(list.savedSelectedTrack + 1);
    selectedTrack = list.savedSelectedTrack;
};

void clearEEPROM()
{
    for (unsigned int i = 0; i < EEPROM.length(); i++)
        EEPROM.write(i, 0);
}

/* only works with max 120MHz teensy 3.6
void WRITE2EEPROM(unsigned int i) { //only write if it is different
    if (doStep[i] != EEPROM.read(i)) {
        EEPROM.write(i, doStep[i]);
    }
} */
