
    const unsigned int leds[] = {5, 6, 7, 8, 9, 10, 11, 12}; // leds & output
    const unsigned int tempoled = 4;
    const unsigned int tempoOut = 3;


    //switch
    const unsigned int switchUp = 0;
    const unsigned int switchDown = 1;
    int switchDownTrig;
    int switchUpTrig;

    //
    int cvOutPin = A14; // 12 bit dac

    int gateNr = 0; //sequencer inits
    const unsigned int gateNrMap[] = {0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7}; // translation map

    // control definitions
    // seq length and offset
    int seqStartPotPin = A0;
    int seqEndPotPin = A1;
    int seqOffsetPin = A2;
    int seqStartValue;
    int seqEndValue;
    int seqOffsetValue;

    // fraction
    int devisionPot = A3;
    int devisionValue;

    // internal clock
    int tempoPin = A4;
    int clockbpmtime;

    // CV control
    int cvValuePotPin = A5;
    int cvAssignDestinationStepButtonPin; // any digital pin
    int cvOutValue;
    int cvAssignDestinationStep;

    //interrupt stuff
    const int pinClock = 23;
    //const int debouncetime = 1000; //debouncetid i mikrosekunder
    volatile bool flag = false;
    float fraction = 1.0/8.0; // 1/8 node  init devision

    void irqClock(){
      flag = true;
    }

    void setup() {
        //Serial.begin(9600);

        pinMode(pinClock, INPUT);
        attachInterrupt(digitalPinToInterrupt(pinClock), irqClock, RISING);

        pinMode(tempoPin, INPUT);
        pinMode(tempoled, OUTPUT);
        pinMode(tempoOut, OUTPUT);

        for (int i = 0; i < 8; ++i) {
            pinMode(leds[i], OUTPUT);
    }



    }

    void loop() {
      //tracker inits
      unsigned int t1 = 250000;
      elapsedMicros microtime;
      elapsedMicros microbeattime = 10000;
      //elapsedMicros debouncer = 0;
      bool resettracker = true;

      // internal clock inits
      elapsedMicros clocktime;

      //awesome scope
      while (1) {
          clockbpmtime = map(analogRead(tempoPin), 0 ,1023, 10000, 1000000);
          //internal clock
          if (clocktime >= clockbpmtime) {
            digitalWrite(tempoled, HIGH);
            digitalWrite(tempoOut, HIGH);
            delay(1);
            digitalWrite(tempoled, LOW);
            digitalWrite(tempoOut, LOW);
            clocktime = 0;
          }


          //seq controls
          seqStartValue = analogRead(seqStartPotPin);
          seqEndValue = analogRead(seqEndPotPin);
          seqOffsetValue = analogRead(seqOffsetPin);
          devisionValue = analogRead(devisionPot);
          //cv control
          cvOutValue = analogRead(cvValuePotPin);
          cvAssignDestinationStep = digitalRead(cvAssignDestinationStepButtonPin);
          //control scaling
          seqStartValue = map(seqStartValue, 0, 1010, 0, 7);
          seqEndValue = map(seqEndValue, 0, 850, 0, 7);
          seqOffsetValue = map(seqOffsetValue, 0, 1000, 0, 7);
          devisionValue = map(devisionValue, 0, 1000, 1, 16); // max 16-dele , kan være højere.
          //multiplicValue = map(multiplicValue 0, 1000, 1, 16); // implementer en form for mult for fraction (float)
          cvOutValue = map(cvOutValue, 0, 1023, 0, 1000); // value til cvOut - dac A14 - skal repræsenterer DC 0-3.3V (0-10V)?

          // få offset regnet med i sequence længden (seqStartValue+seqEndValue) ?
          seqStartValue = seqStartValue+seqOffsetValue;
          seqEndValue = seqEndValue+seqOffsetValue;

          // kontrol af fraction . det funker men der kunne godt lige optimeres
          // på matematikken så den arbejder efter at det er en 8 step seq
          float fraction = 1.0/devisionValue;


        //tracker
        if (flag /*&& (debouncer > debouncetime)*/) {

          t1 = (float(microtime) * fraction);
          microtime = 0;
          microbeattime = 0;
          flag = false;
          resettracker = true;

          //extended "sync" part of tracking
          gateNr = seqStartValue;

          // fraction skal bruges i en funktion sådan at det kan
          // kontrolleres hvor ofte sequenceren sætter seqStartValue til gateNr
        }

        //sequencer
        if (microbeattime >= t1 || resettracker) {
            //length of seq
            if(gateNr >= seqEndValue) {
                gateNr = seqStartValue;
            }

            digitalWrite(leds[gateNrMap[gateNr]], HIGH);
            delay(1);
            digitalWrite(leds[gateNrMap[gateNr]], LOW);

            ++gateNr;
            microbeattime = 0;
            resettracker = false;
        }
      }
    }
