//
// Created by kit on 7/23/21.
//

#ifndef WANNETIOT_BEDROOM_HTTPCLIENT_H
#define WANNETIOT_BEDROOM_HTTPCLIENT_H

#include "point.h"
#include "error.h"
#include <WiFiNINA.h>
#include <SPI.h>

extern error ERR_NO_CONNECTION;
extern error ERR_NO_PAYLOAD;
extern error ERR_NO_WIFI;

#define INFLUX_BUFFER_SIZE 10

struct InfluxConfig {
    char *token;
    char *bucket;
    char *org;
};

struct SlackConfig {
    char *token;
    char *channel;
    void generate_request_body(char *buf, char *message);
};

class HttpClient{
public:
    HttpClient();
    WiFiClient *net;

    char *host;
    long port;

    char *body;
    char *endpoint;
    char *method;

    void generate_influx_request(char *buf, InfluxConfig &influx);
    void generate_slack_request(char *buf, SlackConfig &slack);

    error exec(char *request);
};

struct HttpResponse {
    unsigned long code;
    char status[16];
    char body[256];
};


// influx_send transmits the given pre-formatted data points
error influx_send(HttpClient *client, WiFiClient *net, char *body);

void parse_http_response(char *body, HttpResponse *resp);

#endif //WANNETIOT_BEDROOM_HTTPCLIENT_H
