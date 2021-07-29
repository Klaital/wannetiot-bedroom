#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <WiFiNINA.h>
#include "secrets.h"
#include "config.h"
#include "httpclient.h"
#include <suite.h>

SensorSuite sensors((char*)TAG_VALUE_NODE, (char*)TAG_VALUE_LOCATION);

int status = WL_IDLE_STATUS;
char ssid[] = SECRET_SSID;
#ifdef DEBUG
char debugbuf[1024];
#endif
InfluxClient influx_client;
WiFiClient wifi_client;


// read_wifi_strength queries the wifi chip for the current signal strength and saves the results in the current_readings global.
void read_wifi_strength() {
    sensors.read_rssi();
}

// read_atmo queries the temperature and humidity analog sensors
void read_atmo() {
    // The SensorSuite library handles the different cooldown times for the different sensors
    float t = sensors.read_temperature();
#ifdef DEBUG
    Serial.print("Got Temperature: ");
    Serial.println(t);
#endif
    t = sensors.read_dust();
#ifdef DEBUG
    Serial.print("Got PM2.5: ");
    Serial.println(t);
#endif
}

struct RemoteState {
    int A; // Turn on the lights to 100%
    int B; // Set the lights to 10%
    int C; // Turn off the lights
    int D; // Activate a slack pager
};
RemoteState remoteButtons{};
// read_remote queries the state of the RF remote-control receiver
void read_remote() {
    remoteButtons.A = digitalRead(RF_PIN0);
    remoteButtons.B = digitalRead(RF_PIN1);
    remoteButtons.C = digitalRead(RF_PIN2);
    remoteButtons.D = digitalRead(RF_PIN3);
}
// run_lights sets the output values to operate the attached LED strip based on the Remote Control values
void run_lights() {
    if (remoteButtons.A) {
        remoteButtons.B = 0;
        remoteButtons.C = 0;
        Serial.println("Turning on the lights");
        analogWrite(5, 255);
        // TODO: actually turn the lights on
    }
    if (remoteButtons.B) {
        remoteButtons.A = 0;
        remoteButtons.C = 0;
        Serial.println("Dimming lights");
        analogWrite(5, 25);
        // TODO: actually turn the lights on
    }
    if (remoteButtons.C) {
        remoteButtons.A = 0;
        remoteButtons.B = 0;
        Serial.println("Turning off the lights");
        analogWrite(5, 0);
        // TODO: actually turn the lights off
    }
    if (remoteButtons.D) {
        Serial.println("Activating pager");
        // TODO: actually call Slack
    }
}

// record_metrics sends the metrics recorded so far to Influx if it has been long enough
char metricbuf[16];
char http_body[2048];
void record_metrics() {
    int suite_status = sensors.snapshot(WiFi.getTime());
#ifdef DEBUG
    Serial.print("Sensor snapshot status ");
    Serial.println(suite_status);
#endif
    if (suite_status > 1) {
        sensors.read_influx_buffer(http_body);
        influx_send(&influx_client, &wifi_client, http_body);
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
#ifdef DEBUG
    while(!Serial);
#endif
    // Set up the sensor suite
    // Initialize the temperature & humidity sensor
    DHT dht(DHTSerial, DHTTYPE);
    sensors.enable_dht(&dht);
    // Initialize the Airborne Particulate sensor
    SdsDustSensor sds(Serial1);
    sensors.enable_sds(&sds, SDS_WORKING_PERIOD);
    // Configure the sensor suite to check the WiFi stats
    sensors.enable_wifi();

    sensors.begin();

    // Initialize the RF Remote Control and LED output pins
    remoteButtons.A = 0;
    remoteButtons.B = 0;
    remoteButtons.C = 0;
    remoteButtons.D = 0;
    pinMode(RF_PIN0, INPUT);
    pinMode(RF_PIN1, INPUT);
    pinMode(RF_PIN2, INPUT);
    pinMode(RF_PIN3, INPUT);
    pinMode(5, OUTPUT);

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
    // read sensors
    read_wifi_strength();
    Serial.println("Reading atmo sensors");
    read_atmo();

    // every-so-often, write the metrics to Influx
    Serial.println("Recording metrics");
    record_metrics();

    // Read the RF remote, and then operate the lights accordingly
    Serial.println("Checking remote");
    read_remote();
    run_lights();
}
