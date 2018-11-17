//
// Created by morrigan on 11/17/18.
//

/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
  Example uses a static library to show off generate_arduino_library().

  This example code is in the public domain.
 */
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

void blink(unsigned long duration, uint8_t pin=13){
    digitalWrite(pin, HIGH);   // set the LED on
    delay(duration);           // wait for a second
    digitalWrite(pin, LOW);    // set the LED off
    delay(duration);           // wait for a second
}

void setup() {
    pinMode(13, OUTPUT);
}

void loop() {
    digitalWrite(13, HIGH);
    delay(200);
    digitalWrite(13, LOW);
    delay(200);
}

