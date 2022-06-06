//
// Created by Török Álmos on 2022. 06. 06..
//

#ifndef SMARTPLANT_SEVENSEGMENT_H
#define SMARTPLANT_SEVENSEGMENT_H

#include <Arduino.h>

//8 digit 7 segment display
const int MAX7219_Data_IN = 2;
const int MAX7219_Chip_Select = 0;
const int MAX7219_Clock = 4;
bool displayOn = true;

void shift(byte send_to_address, byte send_this_data) {
    digitalWrite(MAX7219_Chip_Select, LOW);
    shiftOut(MAX7219_Data_IN, MAX7219_Clock, MSBFIRST, send_to_address);
    shiftOut(MAX7219_Data_IN, MAX7219_Clock, MSBFIRST, send_this_data);
    digitalWrite(MAX7219_Chip_Select, HIGH);
}

void writeSevenSegment(int num, int fromDigit = 0, int toDigit = 7) {
    if (fromDigit < 0 || toDigit > 7 || fromDigit > toDigit) return;
    int decRem = 0;
    if (num < 0) {
        shift(0x01 + toDigit, 0x0a);
        toDigit--;
        num *= -1;
    }
    for(int i = fromDigit; i <= toDigit; i++) {
        if (num >= 0) {
            decRem = num % 10;
            shift(0x01 + i, 0x00 + decRem);
            num -= decRem;
            if (num == 0) num = -1;
            else num = num / 10;
        }
        else {
            shift(0x01 + i, 0x0f);
        }
    }
}

void writeSevenSegment(float numf, int fromDigit, int toDigit) {
    if (fromDigit < 0 || toDigit > 7 || fromDigit > toDigit) return;

    int decimals = floor((toDigit - fromDigit + 1) / 3);
    int num = numf * pow(10, decimals);
    int decRem = 0;
    if (num < 0) {
        shift(0x01 + toDigit, 0x0a);
        toDigit--;
        num *= -1;
    }
    for(int i = fromDigit; i <= toDigit; i++) {
        if (num >= 0) {
            decRem = num % 10;
            if (decimals > 0 && i == fromDigit + decimals) {
                shift(0x01 + i, 0x80 + decRem);
            }
            else {
                shift(0x01 + i, 0x00 + decRem);
            }
            num -= decRem;
            if (num == 0) num = -1;
            else num = num / 10;
        }
        else {
            shift(0x01 + i, 0x0f);
        }
    }
}

void setDiplay(bool newStatus) {
    for(int i = 0; i < 7; i++) {
        shift(0x01 + i, 0x0f);
    }
    displayOn = newStatus;
}

#endif //SMARTPLANT_SEVENSEGMENT_H
