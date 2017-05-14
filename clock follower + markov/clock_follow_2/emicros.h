#include <Arduino.h>


#ifndef WProgram_h

#ifndef __CF2__ELAPSEDMICROSHACK__
#define __CF2__ELAPSEDMICROSHACK__

class elapsedMicros
{
private:
  unsigned long us;
public:
  inline elapsedMicros(void) { us = micros(); }
  //inline void reset() { us = micros(); }
  inline operator unsigned long () const { return micros() - us; }
  inline elapsedMicros & operator -= (unsigned long val)      { us += val ; return *this; }
  inline elapsedMicros & operator = (unsigned long val) { us = micros() - val; return *this; }
};
#endif

#endif
