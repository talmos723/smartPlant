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
const int rainMoistureActivation = D2;

//Soil moisture
const int soilMoistureSensor = A0;
//Rain Sensor
const int rainSensor = D0;

//DHT11 humidity and temperature sensor
const int dhtPIN = D1;
DHT dht(dhtPIN, DHT11);

int itaration = 0;

//BH1750 light sensor
TwoWire i2cLight;
BH1750 lightSensor;
const int sda = D3;
const int scl = D4;

//Water pump pin
const int pump = D5;

//Water level
const int maxWaterLevelFromSensor = 1; //[cm] the distance from the sensor to top of the maximum water level (the tank's full water capacity)
const int minWaterLevelFromSensor = 12; //[cm] the distance from the sensor to the bottom of the tank

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
void pumpWater(int ms = 5000);
void sendData(bool withSoilmoistureAndRain = false);
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

    //init the rain and soil moisture
    readSoilMoistureAndRain();
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
    if (itaration >= 900) {
        itaration = 0;
        readSoilMoistureAndRain();
        sendData(true);
    }
    else if (itaration % 300 == 0) {
        sendData();
    }

    if (
        fabs(pot1.temperature - pot1_prev.temperature) / pot1_prev.temperature > 0.05 ||
        fabs(pot1.humidity - pot1_prev.humidity) / pot1_prev.humidity > 0.05 ||
        fabs(pot1.wLevel - pot1_prev.wLevel)  / pot1_prev.wLevel > 0.15 ||
        fabs(pot1.light - pot1_prev.light) / pot1_prev.light > 0.05
                ) {
        sendData();
    }


    //Watering if the soil moisture is low
    /*if (pot1.soilMoisture < 40 && pot1.soilMoisture > 5 && pot1.wLevel > 10) {
        pumpWater();
        readSoilMoistureAndRain(&pot1);
    }*/

    //Serial.read()


    itaration++;
    delay(1000);
}

//withSoilmoistureAndRain false default
void sendData(bool withSoilmoistureAndRain) {
    char buf1[50];
    char buf2[50];
    char buf3[50];
    char buf4[50];
    char buf5[50];
    char buf6[50];
    prepareData(buf1, buf2, buf3, buf4, buf5, buf6);

    if (WiFi.status() == WL_CONNECTED && client.connected()) {
        client.publish(topic, buf1);
        client.publish(topic, buf2);
        client.publish(topic, buf3);
        client.publish(topic, buf4);

        if (withSoilmoistureAndRain) {
            client.publish(topic, buf5);
            client.publish(topic, buf6);
        }
    }
    else {
        Serial.println(buf1);
        Serial.println(buf2);
        Serial.println(buf3);
        Serial.println(buf4);

        if (withSoilmoistureAndRain) {
            Serial.println(buf5);
            Serial.println(buf6);
        }
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

//default value for ms is 5000
void pumpWater(int ms) {
    digitalWrite(pump, HIGH);
    delay(ms);
    digitalWrite(pump,LOW);
    delay(ms);
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