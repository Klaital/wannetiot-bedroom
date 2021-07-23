#include <Arduino.h>
#include <time.h>
#include <WiFiNINA.h>
#include "secrets.h"


int status = WL_IDLE_STATUS;
char ssid[] = SECRET_SSID;

time_t last_influx_write = time(0);
struct Sensors {
    double temp;
    double humidity;
    double pm25;
    long rssi;
    time_t timestamp;
};

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

    // Synchronize the clock
//    timeSync("PST8PDT", "pool.ntp.org", "time.nis.gov");

    // TODO: initialize Influx client
    // Set write precision to milliseconds. Leave other parameters default.
//  influx.setWriteOptions(WriteOptions().writePrecision(WritePrecision::MS));

    // TODO: take initial sensor readings for temperature, humidity, air quality
//    read_atmo();
//    read_wifi_strength();
//    record_metrics();

}

void loop() {
    // TODO: read sensors

    // TODO: every-so-often, write the metrics to Influx
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
}
