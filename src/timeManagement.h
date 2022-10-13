//
// Created by Török Álmos on 2022. 10. 13..
//

#ifndef SMARTPLANT_TIMEMANAGEMENT_H
#define SMARTPLANT_TIMEMANAGEMENT_H

#include <NTPClient.h>
#include <WiFiUdp.h>

const int UTC = 2;
const long utcOffsetInSeconds = UTC * 3600;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

char* getDay() {
    return daysOfTheWeek[timeClient.getDay()];
}

char* getDate(char* ctime) {
    char h[10];
    char m[10];
    char s[10];
    itoa(timeClient.getHours(), h, 10);
    itoa(timeClient.getMinutes(), m, 10);
    itoa(timeClient.getSeconds(), s, 10);
    strcat(ctime, getDay());
    strcat(ctime, ":");
    strcat(ctime, h);
    strcat(ctime, ":");
    strcat(ctime, m);
    strcat(ctime, ":");
    strcat(ctime, s);
    return ctime;
}

#endif //SMARTPLANT_TIMEMANAGEMENT_H
