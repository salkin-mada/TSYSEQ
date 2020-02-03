#ifndef SETTINGS_H
#define SETTINGS_H

extern int flagAbuttonWasPressed;

extern bool doStep[5][16];
extern unsigned int trackStepIterationTrigDevider[5];
extern bool trackMuteLatch[5];
extern int selectedTrack;
extern int devisionValue;
extern int interruptCountToStepResetValue;
extern bool poolDirection;

void storeUISettings();
void recallUISettings();
void clearEEPROM();


//void WRITE2EEPROM(unsigned int i);

#endif