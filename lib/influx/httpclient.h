//
// Created by kit on 7/23/21.
//

#ifndef WANNETIOT_BEDROOM_HTTPCLIENT_H
#define WANNETIOT_BEDROOM_HTTPCLIENT_H

#include "point.h"
#include <WiFiNINA.h>
#include <SPI.h>

#define INFLUX_BUFFER_SIZE 10

struct InfluxClient{
    char *host;
    long port;
    char *data[INFLUX_BUFFER_SIZE];
    int points;
    size_t content_length;

    char *token;
    char *org;
    char *bucket;
};

// init_request initializes the default values for a HttpRequest object
void init_client(InfluxClient *c);

// add_point queues a data point for later transmission
int add_point(InfluxClient *c, Point *p);

// requestsize calculates a fairly safe buffer length for using with formatrequest
size_t requestsize(InfluxClient *client);

// formatrequest generates the HTTP request string
void formatrequest(char *buf, InfluxClient *client);

// influx_send transmits the buffered data points if the buffer is full enough. No-op if the buffer still has space left.
// The accumulated data buffer is free'd for future refilling.
// The return value indicates whether any IO was actually performed.
int influx_send(InfluxClient *client, WiFiClient *net);
// reset_client is used to safely clear out the  buffered data.
void reset_client(InfluxClient *client);

#endif //WANNETIOT_BEDROOM_HTTPCLIENT_H
