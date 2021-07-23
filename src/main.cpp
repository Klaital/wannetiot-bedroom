#include <Arduino.h>
#include <ctime>
#include <WiFiNINA.h>
#include "secrets.h"
#include "config.h"

int status = WL_IDLE_STATUS;
char ssid[] = SECRET_SSID;
char debugbuf[1024];
char timebuf[sizeof "2011-10-08-07:07:09Z"];

time_t last_influx_write = time(0);
struct Sensors {
    double temp;
    double humidity;
    double pm25;
    long rssi;
    time_t timestamp;
};
Sensors current_reading;

// read_wifi_strength queries the wifi chip for the current signal strength and saves the results in the current_readings global.
void read_wifi_strength() {
    current_reading.rssi = WiFi.RSSI();
    current_reading.timestamp = WiFi.getTime();
}

// record_metrics sends the metrics recorded so far to Influx if it has been long enough
void record_metrics() {
    if (difftime(current_reading.timestamp, last_influx_write) < INFLUX_WRITE_INTERVAL) {
        return;
    }
    // TODO: actually write the metrics to the Influx client
    strftime(timebuf, sizeof(timebuf), "%FT%TZ", gmtime(&current_reading.timestamp));
    sprintf(debugbuf, "Recording metrics (%s): temp=%f, humidity=%f, pm2.5=%f, rssi=%ld", timebuf, current_reading.temp, current_reading.humidity, current_reading.pm25, current_reading.rssi);
    Serial.println(debugbuf);
    last_influx_write = WiFi.getTime();
}

void printData() {
    Serial.println("Board Information:");
    // print your board's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    Serial.println();
    Serial.println("Network Information:");
    Serial.print("SSID: ");
    Serial.println(ssid);

    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.println(rssi);
}

void setup() {
    Serial.begin(9600);
    while(!Serial);

    while(status != WL_CONNECTED) {
        Serial.print("Attempting to connect to network: ");
        Serial.println(ssid);
        status = WiFi.begin(ssid, SECRET_PASS);
        delay(10000);
    }

    Serial.println("Network connected");
    Serial.println("----------------------------------------");
    printData();
    Serial.println("----------------------------------------");
    // initialize digital pin LED_BUILTIN as an output.
    pinMode(LED_BUILTIN, OUTPUT);

    // TODO: initialize Influx client
    // Set write precision to milliseconds. Leave other parameters default.
//  influx.setWriteOptions(WriteOptions().writePrecision(WritePrecision::MS));

    // TODO: take initial sensor readings for temperature, humidity, air quality
//    read_atmo();
    read_wifi_strength();
//    record_metrics();

}

void loop() {
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);

    // TODO: read sensors
    read_wifi_strength();
    // TODO: every-so-often, write the metrics to Influx
    record_metrics();

    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
}
