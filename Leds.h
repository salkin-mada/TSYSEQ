#ifndef LEDS_H
#define LEDS_H

const unsigned int leds[] = {2, 5, 8, 11, 14, 17, 20, 23, 26, 29, 32, 35, 38, 41, 44, 47}; // blues
const unsigned int muteLeds[] = {1, 4, 7, 10, 13, 16, 19, 22, 25, 28, 31, 34, 37, 40, 43, 46}; // greens
const unsigned int stepLeds[] = {0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45}; // reds

extern bool flagHasHandledNoteOn[5][16];

void LEDS_setup();
void LEDS_startUp();
void LEDS_on(unsigned int i);
void LEDS_off(unsigned int y, unsigned int i);
void LEDS_clear(unsigned int i);
void LEDS_gateLength(unsigned int devisionFactor, unsigned int trackStepIterationDevider); 
// void -> gatelength is oviousllyyy just for visual feedback, not related to gate trigger length
void MUTELEDS_on(unsigned int i);
void MUTELEDS_off(unsigned int i);
void STEPLEDS_on(unsigned int i);
void STEPLEDS_off(unsigned int i);
void LEDS_sdCardInitFailed();
// class Leds
// {
// 	public:
// 		Leds();
// 		void init();
// };

#endif
