#include "sequencer.h"
#include "Entropy.h"
#include "markov.h"

void setup();
void loop();
void irqClock();
void seqStep();

/*
interrupt pin der modtager pulses fra clock */
const int pinClock = 2;

/*
	en instans af klassen Sequencer
	fra "sequencer.h"
*/
Sequencer seq(seqStep);

/*
	interrupt rutine, der vha clock
	signalet "tapper" beatet til sequenceren
*/
void irqClock() { seq.tap(); }

void setup() {
  pinMode(pinClock, INPUT);
  attachInterrupt(digitalPinToInterrupt(pinClock), irqClock, RISING);

  //random til markov klassen
  Entropy.Initialize();
}

inline void seqStep(int sstep)
{
  markov.step(sstep);
}

void loop()
{
  while (1) { //tight loop for minimal overhead
    seq.tick();
  }
}
