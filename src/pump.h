//
// Created by Török Álmos on 2023. 02. 06..
//

#ifndef SMARTPLANT_PUMP_H
#define SMARTPLANT_PUMP_H


//Water pump pin
const int pump = D5;

//default value for ms is 5000
void pumpWater(int ms = 5000) {
    digitalWrite(pump, HIGH);
    delay(ms);
    digitalWrite(pump,LOW);
    delay(ms);
}

#endif //SMARTPLANT_PUMP_H
