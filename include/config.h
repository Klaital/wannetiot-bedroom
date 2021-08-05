//
// Created by kit on 7/23/21.
//

#ifndef WANNETIOT_BEDROOM_CONFIG_H
#define WANNETIOT_BEDROOM_CONFIG_H
//#define DEBUG 1

#define INFLUX_WRITE_INTERVAL 5.0
#define INFLUX_HOST "klaital.com"
#define INFLUX_PORT 8086
#define INFLUX_ORG "klaital.com"
#define INFLUX_BUCKET "wannetiot"

#define SLACK_HOST "slack.com"
#define SLACK_PORT 443
#define SLACK_PAGER_CHANNEL "@chris"
#define SLACK_PAGER_MESSAGE "you have been summoned!"

#define TAG_VALUE_NODE "bed2"
#define TAG_VALUE_LOCATION "mbed"

// Analog Sensors: define which pins for Temperature, Humidity, etc
#define DHTTYPE DHT22

// How many minutes between dust sensor cycles?
#define SDS_WORKING_PERIOD 10

//
// Pin Config
//

// Temperature & Humidity DHT22
#define DHTSerial 1
// RF Remote Control receiver
#define RF_PIN0 A1
#define RF_PIN1 A2
#define RF_PIN2 A3
#define RF_PIN3 A4
// LED Strip control
#define LED_PIN_R 5
#define LED_PIN_G 4
#define LED_PIN_W 3
#define LED_PIN_B 2

#endif //WANNETIOT_BEDROOM_CONFIG_H
