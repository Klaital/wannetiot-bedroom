#include <Arduino.h>
#include <ctime>
#include <WiFiNINA.h>
#include "secrets.h"
#include "config.h"
#include "httpclient.h"


int status = WL_IDLE_STATUS;
char ssid[] = SECRET_SSID;
char debugbuf[1024];
char timebuf[sizeof "2011-10-08-07:07:09Z"];
InfluxClient influxClient;

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
char metricbuf[16];
Point p{};
void record_metrics() {
    if (difftime(current_reading.timestamp, last_influx_write) < INFLUX_WRITE_INTERVAL) {
        return;
    }

    strftime(timebuf, sizeof(timebuf), "%FT%TZ", gmtime(&current_reading.timestamp));
    sprintf(debugbuf, "Recording metrics (%s): temp=%f, humidity=%f, pm2.5=%f, rssi=%ld", timebuf, current_reading.temp, current_reading.humidity, current_reading.pm25, current_reading.rssi);
    Serial.println(debugbuf);

    // WiFi stats
    init_point(&p);
    set_measurement(&p, (char*)MEASUREMENT_WIFI);
    add_tag(&p, (char*)TAG_NAME_NODE, (char*)TAG_VALUE_NODE);
    add_tag(&p, (char*)TAG_NAME_LOCATION, (char*)TAG_VALUE_LOCATION);
    sprintf(metricbuf, "%ld", current_reading.rssi);
    add_field(&p, (char*)METRIC_RSSI, metricbuf);
    add_point(&influxClient, &p); // Append the metrics to the InfluxClient's queue
    reset_point(&p);

    // TODO: get atmo metrics

    // If the queue is full, run the HTTP submission
    if (influx_send(&influxClient)) {
        last_influx_write = WiFi.getTime();
    }
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

    WiFi.setHostname("wannetiot-bedroom");
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
    init_client(&influxClient);
    influxClient.host = (char*)INFLUX_HOST;
    influxClient.token = (char*)INFLUX_TOKEN;
    influxClient.org = (char*)INFLUX_ORG;
    influxClient.bucket = (char*)INFLUX_BUCKET;

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
