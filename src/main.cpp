#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
//musct have adafruit included except the build fails
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <BH1750.h>

//8 digit 7 segment display
const int MAX7219_Data_IN = 2;
const int MAX7219_Chip_Select = 0;
const int MAX7219_Clock = 4;
bool displayOn = true;

//Ultra sonic distance sensor
const int trigger = 16;
const int echo = 5;

//Soil moisture
const int analog = A0;

//Rain Sensor
const int rainSensor = 12;

//DHT11 humidity and temperature sensor
const int dhtPIN = 14;
DHT dht(dhtPIN, DHT11);
int tempOverHum = 0;

//BH1750 light sensor
TwoWire i2cLight;
BH1750 lightSensor;
const int sda = 3;
const int scl = 1;

//Water pump pin
const int pump = 13;

/*// WiFi home
const char *ssid = "torand-home3"; // Enter your WiFi name
const char *password = "der526MK_D3br3!3n";  // Enter WiFi password*/

// WiFi college
const char *ssid = "1701-SSatT_belsos"; // Enter your WiFi name
const char *password = "sch1701wifi";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "152.66.183.67";
const char *topic = "python/mqtt/almos723";
const char *mqtt_username = "test";
const char *mqtt_password = "teszt2";
const int mqtt_port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);

//Water level
const int maxWaterLevelFromSensor = 1; //[cm] the distance from the sensor to top of the maximum water level (the tank's full water capacity)
const int minWaterLevelFromSensor = 11; //[cm] the distance from the sensor to the bottom of the tank


//functions
int waterLevel();
void pumpWater(int ms = 5000);
float measureDistance();
void shift(byte send_to_address, byte send_this_data);
void writeSevenSegment(int num, int fromDigit, int toDigit);
void connectToWifi(int maxTries);
void connectToMqtt(int maxTries);
void setDiplay(bool newStatus);
void onMqttMessage(char* topic, byte* payload, unsigned int length);

void setup() {
    Serial.begin(115200);
    //Serial.println(WiFi.macAddress());

    pinMode(pump, OUTPUT);

    pinMode(trigger, OUTPUT);
    pinMode(echo, INPUT);

    pinMode(analog, INPUT);

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
    i2cLight.begin(sda, scl);
    lightSensor.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &i2cLight);


    //connectToWifi(50);
    //connectToMqtt(50);

    //Setup dht11
    dht.begin();
}

void loop() {
    //Serial.println(WiFi.macAddress());
    int wLevel = waterLevel();
    int temperature = dht.readTemperature();
    int humidity = dht.readHumidity();
    int soilMoisture = 100 - analogRead(analog)*100/1023;
    int raining = !digitalRead(rainSensor);


    if (soilMoisture < 40 && soilMoisture > 5 && wLevel > 10) {
        pumpWater();
    }


    //Serial.read()

    if (WiFi.status() == WL_CONNECTED && client.connected()) {
        char buf1[30];
        char buf2[30];
        char buf3[30];
        char buf4[30];
        char buf5[30];

        buf1[0] = 0x01;
        itoa(humidity, buf1, 10);
        strcat(buf1,"/esp8266/Humidity");

        buf2[0] = 0x01;
        itoa(temperature, buf2, 10);
        strcat(buf2,"/esp8266/Temperature");

        buf3[0] = 0x01;
        itoa(wLevel, buf3, 10);
        strcat(buf3,"/esp8266/WaterLevel");

        buf4[0] = 0x01;
        itoa(soilMoisture, buf4, 10);
        strcat(buf4,"/esp8266/SoilMoisture");

        buf5[0] = 0x01;
        itoa(raining, buf5, 10);
        strcat(buf5,"/esp8266/Rain");

        client.publish(topic, buf1);
        client.publish(topic, buf2);
        client.publish(topic, buf3);
        client.publish(topic, buf4);
        client.publish(topic, buf5);
    }


    if (displayOn) {
        if (tempOverHum < 6) {
            writeSevenSegment(wLevel, 0, 3);
            writeSevenSegment(temperature, 4, 7);
        }
        else {
            writeSevenSegment(soilMoisture, 0, 3);
            if (raining) shift(0x04, 0x0b);
            writeSevenSegment(humidity, 4, 6);
            shift(0x08, 0x0c);
        }
        tempOverHum++;
        if (tempOverHum >= 12) {
            tempOverHum = 0;
            //measures the light level and adjust the seven segment displays brightness to the given 5 light zones
            float lux = lightSensor.readLightLevel();
            if (lux < 5) shift(0x0a, 0x00);
            else if (lux < 25) shift(0x0a, 0x04);
            else if (lux < 150) shift(0x0a, 0x08);
            else if (lux < 500) shift(0x0a, 0x0b);
            else shift(0x0a, 0x0f);
        }

    }

    delay(1000);
    client.loop();



}

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

float measureDistance() {
    delay(100);
    digitalWrite(trigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigger, LOW);
    long Time = pulseIn(echo, HIGH);
    float distanceCM = Time * 0.034;
    return distanceCM / 2;
}



void shift(byte send_to_address, byte send_this_data) {
    digitalWrite(MAX7219_Chip_Select, LOW);
    shiftOut(MAX7219_Data_IN, MAX7219_Clock, MSBFIRST, send_to_address);
    shiftOut(MAX7219_Data_IN, MAX7219_Clock, MSBFIRST, send_this_data);
    digitalWrite(MAX7219_Chip_Select, HIGH);
}

void writeSevenSegment(int num, int fromDigit, int toDigit) {
    if (fromDigit < 0 || toDigit > 7) return;
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

void connectToWifi(int maxTries) {
    int tries = 0;
    //Wifi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED && tries < maxTries) {
        writeSevenSegment(tries, 0, 6);
        writeSevenSegment(1, 7, 7);
        tries++;
        delay(1000);
    }
}

void connectToMqtt(int maxTries) {
    int tries = 0;
    //MQTT
    if (WiFi.status() == WL_CONNECTED) {
        client.setServer(mqtt_broker, mqtt_port);
        while (!client.connected() && tries < maxTries) {
            String client_id = "esp8266-client-";
            client_id += String(WiFi.macAddress());
            client.connect(client_id.c_str(), mqtt_username, mqtt_password);
            writeSevenSegment(tries, 0, 6);
            writeSevenSegment(2, 7, 7);
            tries++;
            delay(500);
        }
        if (client.connected()) {
            client.setCallback(onMqttMessage);
            client.subscribe(topic);
        }
    }
}

void setDiplay(bool newStatus) {
    for(int i = 0; i < 7; i++) {
        shift(0x01 + i, 0x0f);
    }
    displayOn = newStatus;
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