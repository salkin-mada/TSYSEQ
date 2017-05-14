#include <Arduino.h>

class LED {
private:
  unsigned int pin;
  bool state;
public:
  bool operator= (bool s)
  {
    state = s;
    digitalWrite(pin, state);
    return state;
  }
  inline operator bool()
  {
    return state;
  }
  LED(unsigned int pinNum) : pin(pinNum)
  {
    pinMode(pin, OUTPUT);
    *this = false;
    //state = false;
  }
};
