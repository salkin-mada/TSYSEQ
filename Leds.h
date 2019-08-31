#ifndef LEDS_H
#define LEDS_H

const unsigned int leds[] = {5, 6, 7, 8, 9, 10, 11, 12};
const unsigned int muteLeds[] = {32, 33, 34, 35, 36, 37, 38, 39};

extern bool flagHasHandledNoteOn[5][8];

void LEDS_startUp();
void LEDS_on(unsigned int i);
void LEDS_off(unsigned int y, unsigned int i);
void LEDS_sdCardInitFailed();
// class Leds
// {
// 	public:
// 		Leds();
// 		void init();
// };

#endif
