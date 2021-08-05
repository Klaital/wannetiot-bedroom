#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <WiFiNINA.h>
#include "secrets.h"
#include "config.h"
#include "httpclient.h"
#include <suite.h>

InfluxConfig influxConfig;
HttpClient influxClient;
SlackConfig slackConfig;
HttpClient slackClient;

DHT dht(DHTSerial, DHTTYPE);
SdsDustSensor sds(Serial1);
SensorSuite sensors((char*)TAG_VALUE_NODE, (char*)TAG_VALUE_LOCATION);

int status = WL_IDLE_STATUS;
char ssid[] = SECRET_SSID;
#ifdef DEBUG
char debugbuf[1024];
#endif
WiFiClient wifi_client;


// read_wifi_strength queries the wifi chip for the current signal strength and saves the results in the current_readings global.
void read_wifi_strength() {
    sensors.read_rssi();
}

// read_atmo queries the temperature and humidity analog sensors
void read_atmo() {
    // The SensorSuite library handles the different cooldown times for the different sensors
    sensors.read_temperature();
    sensors.read_dust();
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

struct LightSettings {
    int Red;
    int Green;
    int White;
    int Blue;
};
void set_light_state(LightSettings &newState) {
    analogWrite(LED_PIN_R, newState.Red);
    analogWrite(LED_PIN_G, newState.Green);
    analogWrite(LED_PIN_W, newState.White);
    analogWrite(LED_PIN_B, newState.Blue);
}

LightSettings SOFT_WHITE = LightSettings{
    .Red =  25,
    .Green = 0,
    .White = 255,
    .Blue = 0
};
LightSettings SOFT_WHITE_DIM = LightSettings{
    .Red =  2,
    .Green = 0,
    .White = 25,
    .Blue = 0
};
LightSettings LIGHTS_OFF = LightSettings{
    .Red =  0,
    .Green = 0,
    .White = 0,
    .Blue = 0
};
// run_lights sets the output values to operate the attached LED strip based on the Remote Control values
void run_lights() {
    if (remoteButtons.A) {
        remoteButtons.B = 0;
        remoteButtons.C = 0;
        Serial.println("Turning on the lights");
        set_light_state(SOFT_WHITE);
    }
    if (remoteButtons.B) {
        remoteButtons.A = 0;
        remoteButtons.C = 0;
        Serial.println("Dimming lights");
        set_light_state(SOFT_WHITE_DIM);
    }
    if (remoteButtons.C) {
        remoteButtons.A = 0;
        remoteButtons.B = 0;
        Serial.println("Turning off the lights");
        set_light_state(LIGHTS_OFF);
    }
    if (remoteButtons.D) {
        Serial.println("Activating pager");
        // TODO: actually call Slack
    }
}

// record_metrics sends the metrics recorded so far to Influx if it has been long enough
char metricbuf[16];
char http_body[1024];
char http_request[2048];

void record_metrics() {
    error err;
    int suite_status = sensors.snapshot(WiFi.getTime());
    if (suite_status > 1) {
        sensors.read_influx_buffer(http_body);
#ifdef DEBUG
        Serial.println("Prepared HTTP payload:");
        Serial.println(http_body);
#endif
        influxClient.body = http_body;
        influxClient.generate_influx_request(http_request, influxConfig);
        err = influxClient.exec(http_request);
        if (err == ERR_NO_CONNECTION) {
            Serial.println("Failed to send data to Influx. Re-establishing wifi connection.");
            status = WiFi.begin(ssid, SECRET_PASS);
            delay(5000);
        }
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
    sensors.enable_dht(&dht);
    // Initialize the Airborne Particulate sensor
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

    // initialize HTTP Client & service configs
    slackConfig.token = (char*)SLACK_BOT_TOKEN;
    slackConfig.channel = (char*)SLACK_PAGER_CHANNEL;
    slackClient.port = SLACK_PORT;
    slackClient.host = (char*)SLACK_HOST;
    slackClient.method = (char*)"POST";
    slackClient.endpoint = (char*)"/api/chat.postMessage";
    slackClient.net = &wifi_client;

    influxConfig.token = (char*)INFLUX_TOKEN;
    influxConfig.bucket = (char*)INFLUX_BUCKET;
    influxConfig.org = (char*)INFLUX_ORG;
    influxClient.port = INFLUX_PORT;
    influxClient.host = (char*)INFLUX_HOST;
    influxClient.method = (char*)"POST";
    influxClient.endpoint = (char*)"/api/v2/write";
    influxClient.net = &wifi_client;
}

void loop() {
    // read sensors
    read_wifi_strength();
    read_atmo();

    // every-so-often, write the metrics to Influx
    record_metrics();

    // Read the RF remote, and then operate the lights accordingly
    read_remote();
    run_lights();
}
