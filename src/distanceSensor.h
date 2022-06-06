//
// Created by Török Álmos on 2022. 06. 06..
//

#ifndef SMARTPLANT_DISTANCESENSOR_H
#define SMARTPLANT_DISTANCESENSOR_H

#include <Arduino.h>

//Ultra sonic distance sensor
const int trigger = D0;
const int echo = D1;
//#define WATERPROOFSENSOR

#ifdef WATERPROOFSENSOR
const int triggerMicSec = 20;
#else
const int triggerMicSec = 10;
#endif

float measureDistance() {
    delay(100);
    digitalWrite(trigger, HIGH);
    delayMicroseconds(triggerMicSec);
    digitalWrite(trigger, LOW);
    long time = (long)pulseIn(echo, HIGH);
    float distanceCM = (float)time * 0.034f;
    return distanceCM / 2.0f;
}

#endif //SMARTPLANT_DISTANCESENSOR_H
