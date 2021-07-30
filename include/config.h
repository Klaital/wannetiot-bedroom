//
// Created by kit on 7/23/21.
//

#ifndef WANNETIOT_BEDROOM_CONFIG_H
#define WANNETIOT_BEDROOM_CONFIG_H
#define DEBUG 1

#define INFLUX_WRITE_INTERVAL 5.0

#define MEASUREMENT_WIFI "wifi"
#define MEASUREMENT_ATMO "atmo"

#define METRIC_TEMP "temp"
#define METRIC_HUMIDITY "humidity"
#define METRIC_PM25 "pm25"
#define METRIC_PM10 "pm10"
#define METRIC_RSSI "rssi"

#define TAG_NAME_NODE "node"
#define TAG_VALUE_NODE "bed2"
#define TAG_NAME_LOCATION "loc"
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
#define LED_PIN_R 2
#define LED_PIN_G 3
#define LED_PIN_B 4
#define LED_PIN_W 5

#endif //WANNETIOT_BEDROOM_CONFIG_H
