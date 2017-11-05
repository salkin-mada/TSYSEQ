#ifndef SETTINGS_H
#define SETTINGS_H

#include <SPI.h>
#include <SD.h>
// #include <EEPROM.h>

// use do step to check if some settings has changes
extern bool doStep[];

//extern int buttonState[]; 
//extern int BUTTON_PRESSED;

void SD_init();
void SD_writeSettings(unsigned int i);
void SD_readSettings();
void SD_readAllSettings2Monitor();
void applySetting(String settingName, String settingValue);
//void SD_write(unsigned int i);
//void SD_read(unsigned int i);
//void WRITE2EEPROM(unsigned int i);

#endif