//
// Created by Török Álmos on 2022. 06. 06..
//

#ifndef SMARTPLANT_WIFICOMMUNICATION_H
#define SMARTPLANT_WIFICOMMUNICATION_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "sevenSegment.h"
#include "pump.h"

/*// WiFi home
const char *ssid = "torand-home3"; // Enter your WiFi name
const char *password = "der526MK_D3br3!3n";  // Enter WiFi password*/

// WiFi college
const char *ssid = "1701-belsos"; // Enter your WiFi name
const char *password = "sch1701belsos";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "10.152.69.0";
const char *topic = "python/mqtt/almos723";
const char *pump_topic = "python/mqtt/almos723/pump";
const char *mqtt_username = "viragcserep";
const char *mqtt_password = "smartpot";
const int mqtt_port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);

void connectToWifi(int maxTries) {
    int tries = 0;
    //Wifi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED && tries < maxTries) {
        //writeSevenSegment(tries, 0, 6);
        //writeSevenSegment(1, 7, 7);
        tries++;
        delay(1000);
    }
}

void onMqttMessage(char* msg_topic, byte* payload, unsigned int length) {
    if (strcmp(msg_topic, pump_topic) == 0) {
        pumpWater(7500);
    }
}

void connectToMqtt(int maxTries) {
    int tries = 0;
    //MQTT
    if (WiFi.status() == WL_CONNECTED) {
        client.setServer(mqtt_broker, mqtt_port);
        client.setKeepAlive(900);
        while (!client.connected() && tries < maxTries) {
            String client_id = "esp8266-client-";
            client_id += String(WiFi.macAddress());
            client.connect(client_id.c_str(), mqtt_username, mqtt_password);
            //writeSevenSegment(tries, 0, 6);
            //writeSevenSegment(2, 7, 7);
            tries++;
            delay(500);
        }
        if (client.connected()) {
            client.subscribe(pump_topic);
            client.setCallback(onMqttMessage);
        }
    }
}

#endif //SMARTPLANT_WIFICOMMUNICATION_H
