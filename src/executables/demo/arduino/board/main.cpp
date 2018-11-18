//
// Created by morrigan on 11/17/18.
//


#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// 800 is nearly the largest array size that is supported on arduino uno
const int16_t N = 800;

int16_t val[N];
int16_t idx;

void setup() {
    // this is the largest speed supported
    Serial.begin(9600);
    idx = 0;
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
}