#include <Arduino.h>
#include <ctime>
#include <WiFiNINA.h>
#include "secrets.h"
#include "config.h"
#include "httpclient.h"


int status = WL_IDLE_STATUS;
char ssid[] = SECRET_SSID;
#ifdef DEBUG
char debugbuf[1024];
#endif
char timebuf[sizeof "2011-10-08-07:07:09Z"];
InfluxClient influx_client;
WiFiClient wifi_client;

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

#ifdef DEBUG
    strftime(timebuf, sizeof(timebuf), "%FT%TZ", gmtime(&current_reading.timestamp));
    sprintf(debugbuf, "Recording metrics (%s): temp=%f, humidity=%f, pm2.5=%f, rssi=%ld", timebuf, current_reading.temp, current_reading.humidity, current_reading.pm25, current_reading.rssi);
    Serial.println(debugbuf);
#endif

    // WiFi stats
    init_point(&p);
    p.timestamp = WiFi.getTime();
    set_measurement(&p, (char*)MEASUREMENT_WIFI);
    add_tag(&p, (char*)TAG_NAME_NODE, (char*)TAG_VALUE_NODE);
    add_tag(&p, (char*)TAG_NAME_LOCATION, (char*)TAG_VALUE_LOCATION);
    sprintf(metricbuf, "%ld", current_reading.rssi);
    add_field(&p, (char*)METRIC_RSSI, metricbuf);
    add_point(&influx_client, &p); // Append the metrics to the InfluxClient's queue
    reset_point(&p);

    // TODO: get atmo metrics

    // If the queue is full, run the HTTP submission
    influx_send(&influx_client, &wifi_client); // This will only actually do networking if the output buffer is full.
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
#ifdef DEBUG
    while(!Serial);
#endif

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

    // initialize Influx client
    init_client(&influx_client);
    influx_client.host = (char*)INFLUX_HOST;
    influx_client.token = (char*)INFLUX_TOKEN;
    influx_client.org = (char*)INFLUX_ORG;
    influx_client.bucket = (char*)INFLUX_BUCKET;

    // TODO: take initial sensor readings for temperature, humidity, air quality
//    read_atmo();
    read_wifi_strength();
//    record_metrics();

}

void loop() {
//    delay(1000);
    // read sensors
    read_wifi_strength();
//    read_atmo();

    // TODO: every-so-often, write the metrics to Influx
    record_metrics();
}
