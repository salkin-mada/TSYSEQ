// #include "Settings.h"
// #include "Leds.h"
// #include <Arduino.h>
// //#include "Layout.h"

// bool flagSettingsHaveBeenRead = false;

// bool debug = false; // serial monitor debug modus, should be dependant on serial modus (which yet is to be implemented)
// bool deleteSettingsFile = false; // set to true if settings.txt is "corrupted"

// extern unsigned int stepCount;
// extern unsigned int trackCount;

// // placeholder for doStep data
// String settingValueArray[5][16];

// // things not implemented, begin
// bool savedPoolDirection;
// int savedInterruptCountToStepResetValue;
// int savedDevisionValue;
// int savedSelectedTrack;
// bool savedTrackMuteLatch[5];
// unsigned int savedTrackStepIterationTrigDevider[5];
// // end

// // converting string to Float
// float toFloat(String settingValue){
//     char floatbuf[settingValue.length()+1];
//     settingValue.toCharArray(floatbuf, sizeof(floatbuf));
//     float f = atof(floatbuf);
//     return f;
// }
// // converting string to Long
// long toLong(String settingValue){
//     char longbuf[settingValue.length()+1];
//     settingValue.toCharArray(longbuf, sizeof(longbuf));
//     long l = atol(longbuf);
//     return l;
// }

// // Converting String to integer and then to boolean
// boolean toBoolean(String settingValue) {
//     if(settingValue.toInt()==1){
//         return true; // 1 = true
//     } else {
//         return false; // 0 = false
//     }
// }

// const int chipSelect = BUILTIN_SDCARD; // card slot on teensy 3.6
// File recallFile;

// /* ################################## INIT ################################## */
// /* ################################### SD ################################### */

// void SD_init() {
//     if (debug) {
//         Serial.print("Initializing SD card...");
//     }
//     if (!SD.begin(chipSelect)) {
//         Serial.println("initialization failed!");
//         LEDS_sdCardInitFailed();
//         return;
//     }
//     if (debug) {
//         Serial.println("initialization done.");
//     }
    
//     if (deleteSettingsFile) {
//         if (SD.exists("SETTINGS.TXT") == true) {
//             if (SD.remove("SETTINGS.TXT") == true); { // remove file
//                 if (debug) {
//                     Serial.println("removed SETTINGS.TXT");
//                 }
//             }
//         }    
//     } else {
//         if (debug) {
//             Serial.println("dont delete file");
//         }
//     }
    
//     // make sure settingValueArray is doStep in case no settingsfile existed and it was freshly created
//     // this is a pre sd read thingy
//     for (unsigned int y = 0; y < trackCount; ++y) {
//         for (unsigned int i = 0; i < stepCount; ++i) {
//             settingValueArray[y][i] = doStep[y][i];    
//         }
//     }
    
//     delay(100); // sd chill
// }

// /* ################################## READ ################################### */
// /* ################################ SETTINGS ################################# */

// void SD_readSettings(){

//     // if no setting file exists create it
//     if (!SD.exists("SETTINGS.TXT")) {
//         if (debug) {
//             Serial.println("Creating SETTINGS.TXT...");
//         }
//         recallFile = SD.open("SETTINGS.TXT", FILE_WRITE); // Write File
//         recallFile.close();
//     }

//     if (SD.exists("settings.txt") == true) {
//         char character;
//         String settingName;
//         String settingValue;
        
//         if (debug) {
//             Serial.println("opening settings");
//         }

//         recallFile = SD.open("settings.txt", FILE_READ); // Read File
//         if (recallFile) {int i = 0; int y = 0;
//             if (debug) {
//                 Serial.println("reading settings");
//             }

//             while (recallFile.available()) {
//                 character = recallFile.read();
//                 while((recallFile.available()) && (character != '[')){
//                     character = recallFile.read();
//                     if (debug) {
//                         Serial.println("data before [ begin marker -> ");
//                         Serial.println(character);
//                     }
//                 }
//                 character = recallFile.read();
//                 while((recallFile.available()) && (character != '=')){
//                     settingName = settingName + character;
//                     character = recallFile.read();
//                     if (debug) {
//                     Serial.println(character);
//                     }
//                 }
//                 character = recallFile.read();
//                 while( (recallFile.available()) && (character != ']') && (character != '{') ){
//                     settingValue = settingValue + character;
//                     character = recallFile.read();
//                     if (debug) {
//                         Serial.println(character);                    
//                     }
//                 }
                
//                 if (debug) {
//                     Serial.print("character to parser: ");
//                     Serial.println(character);
//                     Serial.print("setting name= ");
//                     Serial.println(settingName);
//                 }

//                 if (settingName.length() == 0) {
//                     if (debug) {
//                         Serial.println("--> halla --> String settingName is empty !");
//                     }
//                 } // should break , goto close file ??
                
//                 if (character == '{') {
//                     character = recallFile.read();
                    
//                     if (debug) {
//                         Serial.println("string parser found: {");
//                     }
                    
//                     while(recallFile.available() && (character != '}')) {
//                         settingValue = settingValue + character;
//                         character = recallFile.read();
//                         if (character == ',') {
//                             character = recallFile.read();
//                             if (debug) {
//                                 Serial.println("string parser found: ,");
//                                 Serial.println("new step value");
//                                 Serial.print("value: ");
//                                 Serial.println(settingValue);
//                             }
//                             // update setting from parser and "stage" for applySetting
//                             settingValueArray[y][i%stepCount] = settingValue;
                            
//                             if (debug) {
//                                 Serial.print("value step iteration num: ");
//                                 Serial.println(i);
//                                 Serial.print("value track iteration num: ");
//                                 Serial.println(y);
//                             }
//                             //reset string, prepare for next iteration
//                             settingValue = "";
                            
//                             // iterate
//                             ++i;
//                             if (debug) {
//                                 Serial.print("after iter ++i = ");
//                                 Serial.println(i);
//                             }
                            
//                             // here follows super hack for iterating correctly to settingValueArray[>-<][]
//                             // five independant tracks but stored in .txt as one long list

//                             if(i < 16) {} // no need to update y (track#)
//                             if(i == 16) {y = 1;}
//                             if(i == 16*2) {y = 2;}
//                             if(i == 16*3) {y = 3;}
//                             if(i == 16*4) {y = 4;}
//                             // now y (track#) is updated for next round
//                         }
//                     }

//                     if (character == '}') {
//                         character = recallFile.read();
//                         if (debug) {
//                             Serial.println("string parser found: }");
//                             Serial.println(settingValue);
//                         }
//                         // read last setting value
//                         settingValueArray[y][i%stepCount] = settingValue;
//                     }
//                 }
                
//                 if(character == ']'){ // means end of settings
//                     if (debug) {
//                         Serial.println("string parser found: ]");
//                         Serial.println("end of settings, applying settings");
//                     }
//                     // Apply the value to the parameter
//                     flagSettingsHaveBeenRead = true;
//                     applySetting(settingName,settingValue);
//                     // reset strings, redundant..
//                     settingName = "";
//                     settingValue = "";
//                 }
//             }
//             // close the file:
//             recallFile.close();
//             if (debug) {
//                 Serial.println("closed file, read done");
//             }
//         } else {
//             // if the file didn't open, print an error:
//             if (debug) {
//                 Serial.println("error opening settings.txt");
//             }
//         }
//     } else {
//         if (debug) {
//             Serial.println("SD_read could not find settings.txt");
//         }
//     }
// }

// /* ################################# APPLY ################################### */
// /* ################################ SETTINGS ################################# */


// void applySetting(String settingName, String settingValue) {
//     if (flagSettingsHaveBeenRead == true) {
//         // examples
//         /*if(settingName == "exINT") {
//             exINT=settingValue.toInt();
//         }
//         if(settingName == "exFloat") {
//             exFloat=toFloat(settingValue);
//         }
//         if(settingName == "exBoolean") {
//             exBoolean=toBoolean(settingValue);
//         }
//         if(settingName == "exLong") {
//             exLong=toLong(settingValue);
//         }*/
//         //examples end
//         if(settingName == "doStep") {
//             for (unsigned int y = 0; y < trackCount; ++y) {
//                 for (unsigned int i = 0; i < stepCount; ++i) {
//                     doStep[y][i] = toBoolean(settingValueArray[y][i]);
//                 }
//             }
//             /*Serial.println("after apllying toBoolean(settingValueArray[][]) --> doStep[][]");
//             for (int k = 0; k < 5; ++k) {
//                 for (int j = 0; j < 8; ++j) {
//                     Serial.print("settingValueArray: ");
//                     Serial.println(settingValueArray[k][j]);
//                     Serial.print("doStep: ");
//                     Serial.println(doStep[k][j]);
//                 }
//             }*/
//             /* Apply the value to the parameter by searching for the parameter name
//             Using String.toInt(); for Integers
//             toFloat(string); for Float
//             toBoolean(string); for Boolean
//             toLong(string); for Long
//             */
//             if (debug) {
//                 Serial.print("applied ");
//                 Serial.print(settingName);
//                 Serial.print(" ");
//                 Serial.println("setting");
//             }
//         }
        
//         if(settingName == "selectedTrack") {
//             selectedTrack = settingValue.toInt();
//             if (debug) {
//                 Serial.print("applied ");
//                 Serial.print(settingName);
//                 Serial.print(" ");
//                 Serial.println("setting");
//             }
//         }

//         if(settingName == "devisionValue") {
//             devisionValue = settingValue.toInt();
//             if (debug) {
//                 Serial.print("applied ");
//                 Serial.print(settingName);
//                 Serial.print(" ");
//                 Serial.println("setting");
//             }
//         }

//         if(settingName == "interruptCountToStepResetValue") {
//             if (debug) {
//                 Serial.print("applied ");
//                 Serial.print(settingName);
//                 Serial.print(" ");
//                 Serial.println("setting");
//             }
//         }

//         if(settingName == "trackStepIterationTrigDevider") {
//             if (debug) {
//                 Serial.print("applied ");
//                 Serial.print(settingName);
//                 Serial.print(" ");
//                 Serial.println("setting");
//             }
//         }
//     }
// }

// /* ################################## WRITE ################################## */
// /* ################################ SETTINGS ################################# */
// void SD_writeSettings(unsigned int i) {
    
//     // save steps
//     if (flagAbuttonWasPressed == 1) {int y = 0;
//         //Serial.println("__________ something changed");
        
//         // Delete the old one
//         if (SD.exists("settings.txt")) {
//             SD.remove("settings.txt");
//             if (debug) {
//                 Serial.println("settings removed by SD_writeSettings");       ;     
//             }
//         }
//         // Create a fresh empty one
//         recallFile = SD.open("settings.txt", FILE_WRITE);
        
//         if (debug) {
//             Serial.println("SD.open");
//         }
//         // write to recall file          
//         recallFile.print("["); // beginning of a setting type
//         recallFile.print("doStep=");
//         recallFile.print("{"); // begin list
//             for (unsigned int k = 0; k < (stepCount*trackCount); ++k) {
//                 //track 1
//                 if (k < 16) {
//                     y = 0;
//                 }
//                 //track 2
//                 if ((k > 15) && (k < 16*2)) {
//                     y = 1;
//                 }
//                 //track 3
//                 if ((k > 31) && (k < 16*3)) {
//                     y = 2;
//                 }
//                 //track 4
//                 if ((k > 47) && (k < 16*4)) {
//                     y = 3;
//                 }
//                 //track 5
//                 if ((k > 63) && (k < 16*5)) {
//                     y = 4;
//                 }
                
//                 recallFile.print(doStep[y][k%stepCount]); // write all 5 chans as one long string

//                 if (k < 16*5) {
//                     recallFile.print(",");
//                 }
                
//                 if (k == 79) { // end of list
//                     //Serial.println("end list write");
//                     recallFile.print("}");
//                 }
//             }
        
//         recallFile.println("]"); // end of a setting type
        
//         //Serial.println("settings created");
//         //
//         //SD_readSettings(); // read new settings into flaskepost
//         // close the file:
//         recallFile.close();
        
//         if (debug) {
//             Serial.println("############# Writing done and file closed.");
//         }
        
//         settingValueArray[y][i] = doStep[y][i];
//     }
// }


// void SD_readAllSettings2Monitor() {
//     if (SD.exists("settings.txt")) {
//         recallFile = SD.open("settings.txt");
//         if (recallFile) {
//             // read from the file until there's nothing else in it: (and print it to monitor)
//             Serial.println("settings.txt:");
//             while (recallFile.available()) {
//                 Serial.write(recallFile.read());
//             }
//             // close the file:
//             recallFile.close();
//         } else {
//             // if the file didn't open, print an error:
//             Serial.println("read all error opening settings.txt");
//         }
//     }
// }

// /* ################################## CHECK ################################## */
// /* ################################### ROOT ################################## */

// // denne er farlig; bliver ikke lukket ordentligt, og jeg kan derfor ikke skrive til kortet...
// void SD_checkTypeAndListRoot() {
//     Sd2Card card;
//     SdVolume volume;
//     SdFile root;
//     if (!card.init(SPI_HALF_SPEED, chipSelect)) {
//         Serial.println("initialization failed. Things to check:");
//         Serial.println("* is a card inserted?");
//         Serial.println("* is your wiring correct?");
//         Serial.println("* did you change the chipSelect pin to match your shield or module?");
//         return;
//     } else {
//         Serial.println("Wiring is correct and a card is present.");
//     }
//     delay(200);
    
//     // print the type of card
//     Serial.print("\nCard type: ");
//     switch (card.type()) {
//         case SD_CARD_TYPE_SD1:
//         Serial.println("SD1");
//         break;
//         case SD_CARD_TYPE_SD2:
//         Serial.println("SD2");
//         break;
//         case SD_CARD_TYPE_SDHC:
//         Serial.println("SDHC");
//         break;
//         default:
//         Serial.println("Unknown");
//     }
//     delay(200);
    
//     // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
//     if (!volume.init(card)) {
//         Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
//         return;
//     }
    
    
//     // print the type and size of the first FAT-type volume
//     uint32_t volumesize;
//     Serial.print("\nVolume type is FAT");
//     Serial.println(volume.fatType(), DEC);
//     Serial.println();
    
//     volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
//     volumesize *= volume.clusterCount();       // we'll have a lot of clusters
//     volumesize *= 512;                            // SD card blocks are always 512 bytes
//     Serial.print("Volume size (bytes): ");
//     Serial.println(volumesize);
//     Serial.print("Volume size (Kbytes): ");
//     volumesize /= 1024;
//     Serial.println(volumesize);
//     Serial.print("Volume size (Mbytes): ");
//     volumesize /= 1024;
//     Serial.println(volumesize);
    
    
//     Serial.println("\nFiles found on the card (name, date and size in bytes): ");
//     root.openRoot(volume);
    
//     // list all files in the card with date and size
//     root.ls(LS_R | LS_DATE | LS_SIZE);
    
// }


