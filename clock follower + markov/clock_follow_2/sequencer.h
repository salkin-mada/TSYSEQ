#include <Arduino.h>
#include "emicros.h"

/* hack metode til at finde ud af om det er arduino eller teensy
   og hvis det er teensy, så undlade at inkludere klassen fordi den
   allerede er lavet  */



class Sequencer {
private:
  elapsedMicros usec;
  elapsedMicros usec2;
  void (&callback)(int);
  volatile bool flag;
  unsigned long clockInterval;
  unsigned long counter;
public:
  inline void tick()
  {
    if (usec >= clockInterval || flag) {
      callback(counter++);
      flag = false;
      usec -= clockInterval;
    }
  }

  inline void tap()
  {
    if (usec2 < 25000) { return; } //debounce
    clockInterval = usec2 + 2;
    clockInterval >>= 1;
    
    usec2 = 0;
    counter = 0;
    flag = true;
  }

  Sequencer(void (&cb)(int)) : callback(cb), flag(false), clockInterval(1000000) {}
};

