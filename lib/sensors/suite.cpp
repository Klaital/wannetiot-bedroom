//
// Created by kit on 7/28/21.
//

#include <ctime>
#include <WiFiNINA.h>
#include "suite.h"

SensorSuite::SensorSuite(char *node_id, char *location_name, int record_frequency) {
    this->buffered_count = 0;
    this->record_frequency = record_frequency;
    this->last_record = time(0);

    this->temperature = 0.0;
    this->humidity = 0.0;
    this->reads_humidity = this->reads_temperature = false;

    this->pm25 = this->pm10 = 0.0;
    this->sds_working_interval = -1;
    this->reads_dust = false;

    strcpy(this->Location, location_name);
    strcpy(this->NodeId, node_id);
}
void SensorSuite::enable_dht(DHT *sensor) {
    if (sensor != nullptr) {
        this->dht_sensor = sensor;
        this->reads_temperature = true;
        this->reads_humidity = true;
    }
}

void SensorSuite::enable_sds(SdsDustSensor *sensor, int working_interval) {
    if (sensor != nullptr && working_interval > 0) {
        this->dust_sensor = sensor;
        this->dust_sensor->setCustomWorkingPeriod(working_interval);
        this->reads_dust = true;
    }
}
void SensorSuite::begin() {
    if (this->dht_sensor != nullptr) {
        this->dht_sensor->begin();
    }
    if (this->dust_sensor != nullptr) {
        this->dust_sensor->begin();
    }
}

float SensorSuite::read_temperature() {
    if (this->dht_sensor != nullptr) {
        this->temperature = this->dht_sensor->readTemperature(true);
        if (isnan(this->temperature)) {
            Serial.println("Error reading temperature");
            return 0.0;
        }
        this->humidity = this->dht_sensor->readHumidity();
        if (isnan(this->humidity)) {
            Serial.println("Error reading humidity");
            return 0.0;
        }
        return this->temperature;
    }
    return 0.0;
}
float SensorSuite::read_dust() {
    if (this->dust_sensor != nullptr) {
        if (this->dust_sensor->queryWorkingState().isWorking()) {
            PmResult pm = this->dust_sensor->readPm();
            this->pm25 = pm.pm25;
            this->pm10 = pm.pm10;
        }
    }
    return 0.0;
}

// Save the recorded metrics as strings into the Influx buffer
// Returns:
//  0 -> No op happened due to not enough elapsed time since the last snapshot.
//  1 -> The snapshot was done, and there is more room in the buffer.
//  2 -> The snapshot was done, but the buffer is now full. Flush to Influx before the next snapshot!
//  3 -> No op happened do to this buffer being full. Flush the buffer to Influx and retry!
int SensorSuite::snapshot(time_t now) {
    if (difftime(now, this->last_record) < this->record_frequency) {
        return 0;
    }
    if (this->buffered_count == INFLUX_BUFFER_SIZE) {
        return 3;
    }

    char scratch[INFLUX_STRING_MAX];
    char fbuf[7];
    bool firstmetric = true;
    sprintf(scratch, "%s,node=%s,loc=%s", "atmo", this->NodeId, this->Location);
    strcpy(this->influx_buffer[this->buffered_count], scratch);

    scratch[0] = '\0';
    if (this->reads_temperature) {
        if (firstmetric) {
            strcat(this->influx_buffer[this->buffered_count], " ");
            firstmetric = false;
        } else {
            strcat(this->influx_buffer[this->buffered_count], ",");
        }
        ftos(fbuf, 3, 2, this->temperature);
        sprintf(scratch, "temp=%s", fbuf);
        strcat(this->influx_buffer[this->buffered_count], scratch);
    }
    if (this->reads_humidity) {
        if (firstmetric) {
            strcat(this->influx_buffer[this->buffered_count], " ");
            firstmetric = false;
        } else {
            strcat(this->influx_buffer[this->buffered_count], ",");
        }
        ftos(fbuf, 3, 2, this->humidity);
        sprintf(scratch, "humidity=%s", fbuf);
        strcat(this->influx_buffer[this->buffered_count], scratch);
    }
    if (this->reads_dust) {
        if (firstmetric) {
            strcat(this->influx_buffer[this->buffered_count], " ");
            firstmetric = false;
        } else {
            strcat(this->influx_buffer[this->buffered_count], ",");
        }
        ftos(fbuf, 3, 2, this->pm10);
        sprintf(scratch, "pm10=%s", fbuf);
        strcat(this->influx_buffer[this->buffered_count], scratch);
        ftos(fbuf, 3, 2, this->pm25);
        sprintf(scratch, ",pm25=%s", fbuf);
        strcat(this->influx_buffer[this->buffered_count], scratch);
    }

    // Append the timestamp
    sprintf(scratch, " %ld", now);
    strcat(this->influx_buffer[this->buffered_count], scratch);
    this->buffered_count++;

    //
    // WiFi measurement
    //
    firstmetric = true;
    sprintf(this->influx_buffer[this->buffered_count], "%s,node=%s,loc=%s rssi=%ld %ld", "wifi", this->NodeId, this->Location, this->rssi, now);
    this->buffered_count++;
    this->last_record = now;

    // Success!
    if (this->buffered_count >= INFLUX_BUFFER_SIZE) {
        return 2;
    }
    return 1;
}

void SensorSuite::read_influx_buffer(char *outbuf) {
    int i;
    outbuf[0] = '\0';
    for (i=0; i < this->buffered_count; i++) {
        strcat(outbuf, this->influx_buffer[i]);
        strcat(outbuf, "\n");
    }
    this->buffered_count = 0;
}


unsigned long Pow(long V, byte shift)
{
    unsigned long Val = 1;
    while (shift-- != 0)
        Val *= V;

    return Val;
}

char * ftos(char * buf, byte W, byte D, float data)
{
    byte shf = 0;
    if (data < 0)
    {
        data *= -1;
        shf = 1;
        buf[0] = '-';
    }
    long Wdata = data, tmp;
    float dec = data - long(data);

    // get the first whole number and convert it
    buf[0 + shf] = Wdata / Pow(10, W - 1) + '0';
    tmp = Wdata % Pow(10, W - 1);

    //now get the rest of the whole numbers
    for (byte i = 1; i < W; i++)
    {
        long factor = Pow(10, W - 1 - i);
        buf[i + shf] = (tmp / factor) + '0';
        tmp %= factor;
    }

    buf[W + shf] = '.'; // add the decimal point

    // now do the decimal numbers
    for (byte i = 0; i < D; i++)
    {
        dec *= 10;
        buf[W + i + 1 + shf] = (long(dec) % 10) + '0';
    }
    // don't forget the NULL terminator
    buf[W + D + 1 + shf] = NULL;
    return buf;
}

void SensorSuite::enable_wifi() {
    this->reads_rssi = true;
}

long SensorSuite::read_rssi() {
    this->rssi = WiFi.RSSI();
    return this->rssi;
}
