//
// Created by Török Álmos on 2022. 06. 06..
//

#ifndef SMARTPLANT_SEVENSEGMENT_H
#define SMARTPLANT_SEVENSEGMENT_H

#include <Arduino.h>

//8 digit 7 segment display
const int MAX7219_Data_IN = D4;
const int MAX7219_Chip_Select = D2;
const int MAX7219_Clock = D3;
bool displayOn = true;

void shift(byte send_to_address, byte send_this_data) {
    digitalWrite(MAX7219_Chip_Select, LOW);
    shiftOut(MAX7219_Data_IN, 0, MSBFIRST, send_to_address);
    shiftOut(MAX7219_Data_IN, 0, MSBFIRST, send_this_data);
    digitalWrite(MAX7219_Chip_Select, HIGH);
}

void initSevenSegment() {
    pinMode(MAX7219_Data_IN, OUTPUT);
    pinMode(MAX7219_Chip_Select, OUTPUT);
    pinMode(MAX7219_Clock, OUTPUT);

    digitalWrite(MAX7219_Clock, HIGH);
    delay(200);

    //Setup of MAX7219 chip
    shift(0x0f, 0x00); //display test register - test mode off
    shift(0x0c, 0x01); //shutdown register - normal operation
    shift(0x0b, 0x07); //scan limit register - display digits 0 thru 7
    shift(0x0a, 0x0f); //intensity register - max brightness
    shift(0x09, 0xff); //decode mode register - CodeB decode all digits
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

void writeSevenSegment(float numf, int fromDigit = 0, int toDigit = 7) {
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
