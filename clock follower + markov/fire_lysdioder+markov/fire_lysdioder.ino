#include <Entropy.h>
#include "leds.h"
#include "markov.h"

LED led1(9), led2(10), led3(11), led4(12), ledstatus(13);

unsigned int tbase = 117500;

elapsedMicros microtime = tbase;

unsigned int counter = 0;

Markov<unsigned int> markov;

void setup() {
  //auto node = markov.next();
  //unsigned int beginner = node->data;
  Entropy.Initialize();

  for (int i = 0; i < 32; i++)
  {
    markov.setData(i, Entropy.random()%16);
    unsigned int cnum = 2 + (Entropy.random() % 3);
    for (int j = cnum; j >= 0; --j) {
      ledstatus = !ledstatus;
      markov.connect(i, Entropy.random()%32, Entropy.randomf());
    }
  }
  ledstatus = false;
}

void loop() {
  if (microtime >= tbase) {
    ledstatus = !ledstatus;
    //if (counter == 0) 
    //led1 = counter % 5;
    //if (!(counter % 5)) { led1 = !led1; }
    //if (!(counter % 7)) { led2 = !led2; }
    //if (!(counter % 11)) { led3 = !led3; }
    //if (!(counter % 13)) { led4 = !led4; }
    auto node = markov.next();
    unsigned int data = node->data;
    led1 = data & 1;
    led2 = data & 2;
    led3 = data & 4;
    led4 = data & 8;
    
    ++counter;
    microtime -= tbase;
  }
}
