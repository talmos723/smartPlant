//musct have adafruit included except the build fails
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <BH1750.h>
#include <string>

#include "sevenSegment.h"
#include "distanceSensor.h"
#include "wifiCommunication.h"
#include "pump.h"

//Rain - Soil moisture sensor activation pin
const int rainMoistureActivation = D2;

//Soil moisture
const int soilMoistureSensor = A0;
//Rain Sensor
const int rainSensor = D0;

//DHT11 humidity and temperature sensor
const int dhtPIN = D1;
DHT dht(dhtPIN, DHT11);

int iteration = 0;

//BH1750 light sensor
TwoWire i2cLight;
BH1750 lightSensor;
const int sda = D3;
const int scl = D4;

//Water level
const int maxWaterLevelFromSensor = 1; //[cm] the distance from the sensor to top of the maximum water level (the tank's full water capacity)
const int minWaterLevelFromSensor = 18; //[cm] the distance from the sensor to the bottom of the tank

//Pot structure describes every measurable state of a pot
struct PotState {
    int wLevel = 0;
    int temperature = 0;
    int humidity = 0;
    int soilMoisture = 0;
    int raining = 0;
    int light = 0;
};

PotState pot1, pot1_prev;

//functions
int waterLevel();
void sendData();
void readSoilMoistureAndRain();
void prepareData(char* buf1, char* buf2, char* buf3, char* buf4, char* buf5, char* buf6);

void setup() {
    Serial.begin(115200);
    //Serial.println(WiFi.macAddress());

    pinMode(pump, OUTPUT);

    pinMode(trigger, OUTPUT);
    pinMode(echo, INPUT);

    pinMode(rainMoistureActivation, OUTPUT);
    pinMode(soilMoistureSensor, INPUT);
    pinMode(rainSensor, INPUT);

    //initSevenSegment()

    //BH1750 setup
    i2cLight.begin(sda, scl);
    lightSensor.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &i2cLight);


    connectToWifi(50);
    connectToMqtt(50);

    //Setup dht11
    dht.begin();
}

void loop() {
    //Serial.println(WiFi.macAddress());

    if (WiFi.status() == WL_CONNECTED && client.connected()) {
        client.loop();
    }
    pot1_prev = pot1;

    pot1.wLevel = waterLevel();
    pot1.temperature = (int)(dht.readTemperature());
    pot1.humidity = (int)(dht.readHumidity());
    pot1.light = (int)(lightSensor.readLightLevel());

    //Reading rain sensor and soil moisture every 15 mins to expand their life span
    if (iteration >= 900 || iteration == 0) {
        iteration = 0;
        readSoilMoistureAndRain();
        sendData();
    }
    else if (iteration % 300 == 0) {
        sendData();
    }

    //if temperature, humidity, water, light level (higher than 100 lux) changes 5% sends status update
    //if light less than 100 only sends status update if changes more than 15 lux
    if (
        fabs(pot1.temperature - pot1_prev.temperature) / pot1_prev.temperature > 0.05 ||
        fabs(pot1.humidity - pot1_prev.humidity) / pot1_prev.humidity > 0.05 ||
        fabs(pot1.wLevel - pot1_prev.wLevel)  / pot1_prev.wLevel > 0.25 ||
        (fabs(pot1.light - pot1_prev.light) / pot1_prev.light > 0.05 && pot1.light > 100) ||
        (fabs(pot1.light - pot1_prev.light) > 15 && pot1.light <= 100)
                ) {
        sendData();
    }


    //Watering if the soil moisture is low
    /*if (pot1.soilMoisture < 40 && pot1.soilMoisture > 5 && pot1.wLevel > 10) {
        pumpWater();
        readSoilMoistureAndRain(&pot1);
    }*/

    //Serial.read()


    iteration++;
    delay(1000);
}

void sendData() {
    char buf1[50];
    char buf2[50];
    char buf3[50];
    char buf4[50];
    char buf5[50];
    char buf6[50];
    prepareData(buf1, buf2, buf3, buf4, buf5, buf6);
    //char timeString[100] = {0};
    //getDate(timeString);
    //strcat(timeString, "/esp8266/Time");

    if (WiFi.status() == WL_CONNECTED && client.connected()) {
        client.publish(topic, "BeginDataTransfer");
        //client.publish(topic, timeString);
        client.publish(topic, buf1);
        client.publish(topic, buf2);
        client.publish(topic, buf3);
        client.publish(topic, buf4);
        client.publish(topic, buf5);
        client.publish(topic, buf6);
        client.publish(topic, "EndDataTransfer");
    }
    else {
        Serial.println("BeginDataTransfer");
        //Serial.println(timeString);
        Serial.println(buf1);
        Serial.println(buf2);
        Serial.println(buf3);
        Serial.println(buf4);
        Serial.println(buf5);
        Serial.println(buf6);
        Serial.println("EndDataTransfer");
    }
}

void prepareData(char* buf1, char* buf2, char* buf3, char* buf4, char* buf5, char* buf6) {
    itoa(pot1.humidity, buf1, 10);
    strcat(buf1,"/esp8266/Humidity");

    itoa(pot1.temperature, buf2, 10);
    strcat(buf2, "/esp8266/Temperature");

    itoa(pot1.wLevel, buf3, 10);
    strcat(buf3, "/esp8266/WaterLevel");

    itoa(pot1.light, buf4, 10);
    strcat(buf4, "/esp8266/Light");

    itoa(pot1.soilMoisture, buf5, 10);
    strcat(buf5, "/esp8266/SoilMoisture");

    itoa(pot1.raining, buf6, 10);
    strcat(buf6, "/esp8266/Rain");
}

void prepareData2(char* buf1, char* buf2, char* buf3, char* buf4, char* buf5, char* buf6) {
    byte id = 0x0F;
    buf1[0] = id;
    itoa(pot1.humidity, &buf1[1], 10);
    strcat(buf1,"/esp8266/Humidity");

    buf2[0] = id;
    std::string s = std::to_string(pot1.temperature);
    for (int i = 0; i < 10; i++) {
        buf2[1+i] = s[i];
    }
    strcat(buf2,"/esp8266/Temperature");

    buf3[0] = id;
    itoa(pot1.wLevel, &buf3[1], 10);
    strcat(buf3,"/esp8266/WaterLevel");

    buf4[0] = id;
    itoa(pot1.light, &buf4[1], 10);
    strcat(buf4,"/esp8266/Light");

    buf5[0] = id;
    itoa(pot1.soilMoisture, &buf5[1], 10);
    strcat(buf5, "/esp8266/SoilMoisture");

    buf6[0] = id;
    itoa(pot1.raining, &buf6[1], 10);
    strcat(buf6, "/esp8266/Rain");
}

int waterLevel() {
    return 100 - (measureDistance() - maxWaterLevelFromSensor) * 100 / (minWaterLevelFromSensor - maxWaterLevelFromSensor);
}

void readSoilMoistureAndRain() {
    digitalWrite(rainMoistureActivation, HIGH);
    delay(250);
    pot1.soilMoisture = 100 - analogRead(soilMoistureSensor) * 100 / 1023;
    pot1.raining = !digitalRead(rainSensor);
    delay(100);
    digitalWrite(rainMoistureActivation, LOW);
}