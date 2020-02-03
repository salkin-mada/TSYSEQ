
#include <Bounce2.h>

Bounce commonPortDebouncer = Bounce();

int readSensor (const byte which)
  {
  // select correct MUX channel
  digitalWrite (26, (which & 1) ? HIGH : LOW);
  digitalWrite (27, (which & 2) ? HIGH : LOW);
  digitalWrite (28, (which & 4) ? HIGH : LOW);

  //delay(1);
  delayMicroseconds(5);
  
  commonPortDebouncer.update();
  //return commonPortDebouncer.read(); // debounced not working with muxed switches..
  // now read the sensor
  return digitalRead(24);
  }  // end of readSensor
  
void setup() {
  Serial.begin(9600);

  delay(1000);
  pinMode(24, INPUT_PULLUP);
  pinMode(26, OUTPUT);
  pinMode(27, OUTPUT);
  pinMode(28, OUTPUT);

  commonPortDebouncer.attach(24);
  commonPortDebouncer.interval(50);

  Serial.println("test mux setup");
}

void loop (){
  // show all 8 sensor readings
  for (byte i = 0; i < 8; i++){
  if (readSensor(i) == HIGH) {
     //Button is not pressed...
  } else {
     //Button is pressed
     Serial.print(i);
     Serial.println(" Button  is pressed!!!");
     Serial.println(commonPortDebouncer.read());
  }
  }
  delay(50);
}
