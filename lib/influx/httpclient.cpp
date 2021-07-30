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

void formatrequest(char *buf, InfluxClient *client) {
    int i;
    sprintf(buf, "POST /api/v2/write?bucket=%s&org=%s&precision=s HTTP/1.1\nAuthorization: Token %s\nHost: %s\nContent-Length: %d\n",
            client->bucket, client->org, client->token, client->host, client->content_length);
}


int influx_send(InfluxClient *client, WiFiClient *net, char *body) {
    if (body == nullptr) {
        return 0;
    }
    char requeststr[2048];

#ifdef DEBUG
    HttpResponse resp{};
#endif
    client->content_length = strlen(body);
    formatrequest(requeststr, client);

    //  run the http connection
    if (net->connect(client->host, client->port)) {
#ifdef DEBUG
        Serial.println(body);
#endif
        net->print(requeststr);
        net->print("\n");
        net->print(body);
        net->print("\n\n");
#ifdef DEBUG
        // Read the HTTP response to check the status code
        net->readBytes(debugbuf, 1024);
        parse_http_response(debugbuf, &resp);
        Serial.print("HTTP Response from Influx: ");
        Serial.print(resp.code);
        Serial.print(" ");
        Serial.println(resp.status);
        if (resp.code >= 300) {
            Serial.println();
            Serial.println(resp.body);
        }
#endif
    } else {
        Serial.println("Failed to open TCP connection");
        return 0;
    }

    return 1;
}

void parse_http_response(char *body, HttpResponse *resp) {
    char *token;
    char *copy = (char*)malloc(strlen(body)+1);
    strcpy(copy, body);

    token = strtok(copy, " "); // Discard the first token "HTTP/1.1"
    token = strtok(0, " ");
    resp->code = strtoul(token, nullptr, 10);
    token = strtok(0, "\n");
    strcpy(resp->status, token);
    free(copy);
    strcpy(resp->body, body);
}
