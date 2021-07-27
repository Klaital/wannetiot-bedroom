#include <Arduino.h>
#include <WiFiNINA.h>
#include <ctime>
#include "temperature.h"
#include "secrets.h"
#include "config.h"
#include "httpclient.h"
#include "../lib/helpers/float.h"

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <SdsDustSensor.h>

DHT dht(DHT_PIN, DHTTYPE);
SdsDustSensor sds(Serial1);

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
    double pm10;
    long rssi;
    time_t timestamp;
};
Sensors current_reading;

// read_wifi_strength queries the wifi chip for the current signal strength and saves the results in the current_readings global.
void read_wifi_strength() {
    current_reading.rssi = WiFi.RSSI();
    current_reading.timestamp = WiFi.getTime();
}

// read_atmo queries the temperature and humidity analog sensors
void read_atmo() {
    // TODO: the DHT22 can only be read every 2 seconds. Skip this except when the sensor is ready.
    // TODO: figure out how to make the Influx client not upload when there is no change in the timestamp

    float t = dht.readTemperature(true);
    float h = dht.readHumidity();
    if (isnan(t) || isnan(h)) {
        Serial.println("Failed to read from DHT sensor");
        return;
    }
    current_reading.temp = t;
    current_reading.humidity = h;
    current_reading.timestamp = WiFi.getTime();

    // TODO: the SDS011 sensor should only be read infrequently to maximize its service life. A separate, longer timer should be used here - maybe every 10 minutes?
    if (sds.queryWorkingState().isWorking()) {
        PmResult pm = sds.readPm();
        current_reading.pm25 = pm.pm25;
        current_reading.pm10 = pm.pm10;
    }
}

// record_metrics sends the metrics recorded so far to Influx if it has been long enough
char metricbuf[16];
Point p{};
void record_metrics() {
    if (difftime(current_reading.timestamp, last_influx_write) < INFLUX_WRITE_INTERVAL) {
        return;
    }
    time_t now = WiFi.getTime();

//#ifdef DEBUG
//    strftime(timebuf, sizeof(timebuf), "%FT%TZ", gmtime(&current_reading.timestamp));
//    sprintf(debugbuf, "Recording metrics (%s): temp=%f, humidity=%f, pm2.5=%f, rssi=%ld", timebuf, current_reading.temp, current_reading.humidity, current_reading.pm25, current_reading.rssi);
//    Serial.println(debugbuf);
//#endif

    // WiFi stats
    init_point(&p);
    p.timestamp = now;
    set_measurement(&p, (char*)MEASUREMENT_WIFI);
    add_tag(&p, (char*)TAG_NAME_NODE, (char*)TAG_VALUE_NODE);
    add_tag(&p, (char*)TAG_NAME_LOCATION, (char*)TAG_VALUE_LOCATION);
    sprintf(metricbuf, "%ld", current_reading.rssi);
    add_field(&p, (char*)METRIC_RSSI, metricbuf);
    add_point(&influx_client, &p); // Append the metrics to the InfluxClient's queue
    reset_point(&p);

    // atmo metrics
    init_point(&p);
    p.timestamp = now;
    set_measurement(&p, (char*)MEASUREMENT_ATMO);
    add_tag(&p, (char*)TAG_NAME_NODE, (char*)TAG_VALUE_NODE);
    add_tag(&p, (char*)TAG_NAME_LOCATION, (char*)TAG_VALUE_LOCATION);
    ftos(metricbuf, 3, 4, current_reading.temp);
    add_field(&p, (char*)METRIC_TEMP, metricbuf);
    ftos(metricbuf, 3, 4, current_reading.humidity);
    add_field(&p, (char*)METRIC_HUMIDITY, metricbuf);
    ftos(metricbuf, 3, 4, current_reading.pm25);
    add_field(&p, (char*)METRIC_PM25, metricbuf);
    ftos(metricbuf, 3, 4, current_reading.pm10);
    add_field(&p, (char*)METRIC_PM10, metricbuf);
    add_point(&influx_client, &p); // Append the metrics to the InfluxClient's queue
    reset_point(&p);

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
    // Initialize the temperature & humidity sensor
    dht.begin();
    // Initialize the Airborne Particulate sensor
    sds.begin();
    sds.setCustomWorkingPeriod(10); // only run the sensor every 10 minutes

    // Initialize the ADC
    analogReadResolution(12);
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
}

void loop() {
    delay(2000);
    // read sensors
    read_wifi_strength();
#ifdef DHT_PIN
    read_atmo();
#endif

    // TODO: every-so-often, write the metrics to Influx
    record_metrics();
}
