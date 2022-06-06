//musct have adafruit included except the build fails
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <BH1750.h>
#include <string>

#include "sevenSegment.h"
#include "distanceSensor.h"
#include "wifiCommunication.h"

//Rain - Soil moisture sensor activation pin
const int rainMoistureActivation = D7;

//Soil moisture
const int soilMoistureSensor = A0;
//Rain Sensor
const int rainSensor = 12;

//DHT11 humidity and temperature sensor
const int dhtPIN = 14;
DHT dht(dhtPIN, DHT11);
int tempOverHum = 0;

//BH1750 light sensor
/*TwoWire i2cLight;
BH1750 lightSensor;
const int sda = 3;
const int scl = 1;*/

//Water pump pin
const int pump = D8;

//Water level
const int maxWaterLevelFromSensor = 1; //[cm] the distance from the sensor to top of the maximum water level (the tank's full water capacity)
const int minWaterLevelFromSensor = 11; //[cm] the distance from the sensor to the bottom of the tank

//Pot structure describes every measurable state of a pot
struct PotState {
    int wLevel = 0;
    float temperature = 0.0f;
    int humidity = 0;
    int soilMoisture = 0;
    int raining = 0;
};

PotState pot1;

//functions
int waterLevel();
void pumpWater(int ms = 5000);
void readSoilMoistureAndRain(PotState* pot);

void setup() {
    Serial.begin(115200);
    Serial.println(WiFi.macAddress());

    pinMode(pump, OUTPUT);

    pinMode(trigger, OUTPUT);
    pinMode(echo, INPUT);
    
    pinMode(rainMoistureActivation, OUTPUT);
    pinMode(soilMoistureSensor, INPUT);
    pinMode(rainSensor, INPUT);

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

    //BH1750 setup
    /*i2cLight.begin(sda, scl);
    lightSensor.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &i2cLight);*/


    //connectToWifi(50);
    //connectToMqtt(50);

    //Setup dht11
    dht.begin();

    //init the rain and soil moisture
    readSoilMoistureAndRain(&pot1);
}

void loop() {
    //Serial.println(WiFi.macAddress());
    pot1.wLevel = (int)measureDistance();
    writeSevenSegment(pot1.wLevel);
    /*
    pot1.temperature = dht.readTemperature();
    pot1.humidity = (int)dht.readHumidity();


    if (pot1.soilMoisture < 40 && pot1.soilMoisture > 5 && pot1.wLevel > 10) {
        pumpWater();
        readSoilMoistureAndRain(&pot1);
    }

    //Serial.read()

    if (WiFi.status() == WL_CONNECTED && client.connected()) {
        client.loop();
        char buf1[30];
        char buf2[30];
        char buf3[30];


        buf1[0] = 0x01;
        itoa(pot1.humidity, &buf1[1], 10);
        strcat(buf1,"/esp8266/Humidity");

        buf2[0] = 0x01;
        std::string s = std::to_string(pot1.temperature);
        for (int i = 0; i < 10; i++) {
            buf2[1+i] = s[i];
        }
        strcat(buf2,"/esp8266/Temperature");

        buf3[0] = 0x01;
        itoa(pot1.wLevel, &buf3[1], 10);
        strcat(buf3,"/esp8266/WaterLevel");

        client.publish(topic, buf1);
        client.publish(topic, buf2);
        client.publish(topic, buf3);

        if (tempOverHum == 0) {
            char buf4[30];
            char buf5[30];
            buf4[0] = 0x01;
            itoa(pot1.soilMoisture, &buf4[1], 10);
            strcat(buf4, "/esp8266/SoilMoisture");

            buf5[0] = 0x01;
            itoa(pot1.raining, &buf5[1], 10);
            strcat(buf5, "/esp8266/Rain");
            client.publish(topic, buf4);
            client.publish(topic, buf5);
        }
    }
    else {
        char buf1[40];
        char buf2[40];
        char buf3[40];


        buf1[0] = 0x01;
        itoa(pot1.humidity, &buf1[1], 10);
        strcat(buf1, "/esp8266/Humidity");

        buf2[0] = 0x01;
        std::string s = std::to_string(pot1.temperature);
        for (int i = 0; i < 10; i++) {
            buf2[1+i] = s[i];
        }
        strcat(buf2, "/esp8266/Temperature");

        buf3[0] = 0x01;
        itoa(pot1.wLevel, &buf3[1], 10);
        strcat(buf3,"/esp8266/WaterLevel");

        Serial.println(buf1);
        Serial.println(buf2);
        Serial.println(buf3);

        if (tempOverHum == 0) {
            char buf4[30];
            char buf5[30];
            buf4[0] = 0x01;
            itoa(pot1.soilMoisture, &buf4[1], 10);
            strcat(buf4, "/esp8266/SoilMoisture");

            buf5[0] = 0x01;
            itoa(pot1.raining, &buf5[1], 10);
            strcat(buf5, "/esp8266/Rain");
            Serial.println(buf4);
            Serial.println(buf5);
        }
    }


    if (displayOn) {
        /*if (tempOverHum % 12 == 0) {
            //measures the light level and adjust the seven segment displays brightness to the given 5 light zones
            float lux = lightSensor.readLightLevel();
            if (lux < 5) shift(0x0a, 0x00);
            else if (lux < 25) shift(0x0a, 0x04);
            else if (lux < 150) shift(0x0a, 0x08);
            else if (lux < 500) shift(0x0a, 0x0b);
            else shift(0x0a, 0x0f);
        }*//*
        if (tempOverHum % 12 < 6) {
            writeSevenSegment(pot1.wLevel, 0, 3);
            writeSevenSegment(pot1.temperature, 4, 7);
        }
        else {
            writeSevenSegment(pot1.soilMoisture, 0, 3);
            if (pot1.raining) shift(0x04, 0x0b);
            writeSevenSegment(pot1.humidity, 4, 6);
            shift(0x08, 0x0c);
        }
        tempOverHum++;
        //Reading rain sensor and soil moisture every 15 mins to expand their life span
        if (tempOverHum >= 900) {
            tempOverHum = 0;
            readSoilMoistureAndRain(&pot1);
        }
    }*/

    delay(1000);
}

//default value for ms is 5000
void pumpWater(int ms) {
    digitalWrite(pump, HIGH);
    shift(0x01, 0x0e);
    shift(0x02, 0x0f);
    shift(0x03, 0x0f);
    shift(0x04, 0x0f);
    shift(0x05, 0x0f);
    shift(0x06, 0x0f);
    shift(0x07, 0x0f);
    shift(0x08, 0x0f);
    delay(ms);
    digitalWrite(pump,LOW);
    shift(0x01, 0x0f);
    shift(0x02, 0x0f);
    shift(0x03, 0x0f);
    shift(0x04, 0x0f);
    shift(0x05, 0x0f);
    shift(0x06, 0x0f);
    shift(0x07, 0x0f);
    shift(0x08, 0x0f);
    delay(ms);
}

int waterLevel() {
    return 100 - (measureDistance() - maxWaterLevelFromSensor) * 100 / (minWaterLevelFromSensor - maxWaterLevelFromSensor);
}

void readSoilMoistureAndRain(PotState* pot) {
    digitalWrite(rainMoistureActivation, HIGH);
    delay(250);
    pot->soilMoisture = 100 - analogRead(soilMoistureSensor) * 100 / 1023;
    pot->raining = !digitalRead(rainSensor);
    delay(100);
    digitalWrite(rainMoistureActivation, LOW);
}

/*first byte:
 * 0x00: prints the following bytes as int to the seven segment display for 0.5s
 * 0x01: tries to connect to the wifi (default settings)
 * 0x02: tries to connect to the mqtt broker (default settings)
 */

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
    for (int i=0; i<length; i++) {
        Serial.print(payload[i]);
        Serial.print(", ");
    }
    Serial.println();
    if (length < 2 || payload[0] == 0x01) return;
    switch(payload[1]) {
        case 0x00:
            for (int i = 2; i < length; i++) {
                writeSevenSegment((int)payload[i], 0, 6);
                shift(0x08, 0x0e);
                delay(3000);
            }
            break;

        case 0x01:
            connectToWifi(50);
            break;

        case 0x02:
            connectToMqtt(50);
            break;
        case 0x03:
            setDiplay(true);
            break;
        case 0x04:
            setDiplay(false);
            break;


        default: break;
    }
}