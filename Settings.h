#ifndef SETTINGS_H
#define SETTINGS_H

#include <SPI.h>
#include <SD.h>
// #include <EEPROM.h>

extern bool doStep[5][8]; // should be trackCount and stepCount instead of [5][8]
extern int flagAbuttonWasPressed;
extern unsigned int stepCount;
extern unsigned int trackCount;
extern int selectedTrack; // not used in settings yet, but here as a reminder
// would be nice to recall selectedTrack on power up

void SD_init();
void SD_checkTypeAndListRoot();
void SD_writeSettings(unsigned int i);
void SD_readSettings();
void SD_readAllSettings2Monitor();
void applySetting(String settingName, String settingValue);
//void SD_write(unsigned int i);
//void SD_read(unsigned int i);
//void WRITE2EEPROM(unsigned int i);

#endif