//
// Created by kit on 7/23/21.
//

#include <Arduino.h>
#include <malloc.h>
#include "httpclient.h"

void init_client(InfluxClient *c) {
    c->port = 8086;
    c->points = 0;
}

size_t requestsize(InfluxClient *client) {
    int i;
    size_t bytes = 150 + 20 + 88 + 15 + 5; // boilerplate + org/bucket + token + hostname + content-length
    for (i = 0; i < client->points; i++) {
        bytes += strlen(client->data[i]) + 1;
    }
    return bytes + client->points;
}

void formatrequest(char *buf, InfluxClient *client) {
    int i;
    sprintf(buf, "POST /api/v2/write?bucket=%s&org=%s&precision=s HTTP/1.1\nAuthorization: Token %s\nHost: %s\nContent-Length: %d\n",
            client->bucket, client->org, client->token, client->host, client->content_length);
    // Append the body of the request
    for(i=0; i < client->points; i++) {
        Serial.println(client->data[i]);
        strcat(buf, "\n");
        strcat(buf, client->data[i]);
    }
}

int add_point(InfluxClient *c, Point *p) {
    char pointbuf[256];
    // If the buffer is full, refuse
    if (c->points == INFLUX_BUFFER_SIZE-1) {
        Serial.println("Buffer full!");
        return 0;
    }
    // If the Point is invalid for transmission, refuse
    if (!validate_point(p)) {
        Serial.println("Invalid data point");
        return 0;
    }

    // Copy the Point into the buffer
    pointbuf[0] = '\0';
    Serial.print("Buffering point: ");
    point_to_str(pointbuf, p);
    Serial.println(pointbuf);
    c->data[c->points] = (char*)malloc(strlen(pointbuf)+1);
    if (c->data[c->points] == nullptr) {
        Serial.println("Failed to allocate ram for the point");
        return 0;
    }
    strcpy(c->data[c->points], pointbuf);

    // Success!
    c->points += 1;
    c->content_length += strlen(pointbuf) + 1;
    return 1;
}

int influx_send(InfluxClient *client, WiFiClient *net) {
#ifdef DEBUG
    char debugbuf[1024];
#endif
    // Only run the HTTP connection when the buffer is full enough
    if (client->points < INFLUX_BUFFER_SIZE-2) {
        return 0;
    }
    size_t bytes = requestsize(client);
    char *requeststr = (char*)malloc(bytes);
    if (requeststr == nullptr) {
        Serial.println("OOM: failed to allocate string for influx request");
    }
    formatrequest(requeststr, client);
    reset_client(client);

    //  run the http connection
#ifdef DEBUG
    Serial.println("---------------------\nSending HTTP Request:");
    Serial.println(requeststr);
#endif

    if (net->connect(client->host, client->port)) {
        net->print(requeststr);
        net->print("\n\n");
#ifdef DEBUG
        // Read the HTTP response to check the status code
        net->readBytes(debugbuf, 1024);
        Serial.println("---------------------\nGGot HTTP Response:");
        Serial.println(debugbuf);
#endif
    } else {
        Serial.println("Failed to open TCP connection");
        return 0;
    }
    free(requeststr);

    return 1;
}


void reset_client(InfluxClient *client) {
    int i;
    for (i=0; i < client->points; i++) {
        free(client->data[i]);
    }
    client->points = 0;
}
