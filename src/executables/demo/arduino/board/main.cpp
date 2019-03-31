//
// Created by morrigan on 11/17/18.
//


#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// 800 is nearly the largest array size that is supported on arduino uno
const int16_t N = 200;

int16_t val[N];
int16_t idx;

//int16_t cnt;
//boolean inc;

void setup() {
    // this is the largest speed supported
    Serial.begin(9600);
    idx = 0;
    //cnt = 1;
    //inc = true;

    // pin 13
    //pinMode(LED_BUILTIN, OUTPUT);

    // pin 12
    //pinMode(MISO, OUTPUT);
}

void loop() {
    if(idx < N){
        val[idx] = analogRead(A0);
        idx++;
    }else{
        Serial.print("BEGIN");
        Serial.write((byte*)val, N*sizeof(int16_t));
        Serial.print("END");
        idx = 0;
    }

    // TODO: Create practicum based on Arduino and include it there
    // temporary code to work with digital potentiometer
    /*if(cnt == 100 || cnt == 0)
        inc = !inc;
    cnt = inc ? cnt + 1 : cnt - 1;

    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);

    digitalWrite(MISO, inc);*/
}