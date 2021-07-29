//
// Created by kit on 7/28/21.
//

#ifndef WANNETIOT_BEDROOM_SUITE_H
#define WANNETIOT_BEDROOM_SUITE_H
#include <DHT.h>
#include <SdsDustSensor.h>

#define INFLUX_STRING_MAX 256
#define INFLUX_BUFFER_SIZE 10

// ftos converts the floating point number into a string.
// WARNING: only preserves to digits after the decimal point.
char * ftos(char * buf, byte W, byte D, float data);

class SensorSuite {
private:
    //
    // Cached data from sensors
    //
    float temperature, humidity;
    bool reads_temperature, reads_humidity;
    float pm25, pm10;
    bool reads_dust;
    long rssi;
    bool reads_rssi;


    char influx_buffer[INFLUX_STRING_MAX][INFLUX_BUFFER_SIZE];
    int buffered_count;
    int record_frequency;
    time_t last_record;

    //
    // Metadata
    //
    char Location[8];
    char NodeId[8];

    //
    // Atmo Sensors
    //

    // DHT22 Temperature+Humidity
    DHT *dht_sensor;

    // Air Quality, Particulate
    SdsDustSensor *dust_sensor;
    int sds_working_interval; // how often it should run, in minutes

    //
    // Metadata: WiFi signal strength, battery voltage, etc
    //
    // TODO: battery voltage sensor


public:
    SensorSuite(char *node_id, char *location_name, int record_frequency=2); // how often to save readings in seconds
    void enable_dht(DHT *sensor);
    void enable_sds(SdsDustSensor *sensor, int working_interval);
    float read_temperature(); // also reads humidity if enabled
    float read_dust(); // returns pm2.5, also reads pm1.0
    void enable_wifi();
    long read_rssi(); // returns the WiFiClient's rssi measurement

    int snapshot(time_t now); // grab the sensor values and record them as an Influx Measurement

    void read_influx_buffer(char *outbuf); // reset the buffer index after writing the data points to Influx

    // begin initializes all the configured sensor libraries
    void begin();
};


#endif //WANNETIOT_BEDROOM_SUITE_H
