//
// Created by kit on 7/23/21.
//

#include <Arduino.h>
#include <malloc.h>
#include "httpclient.h"

error ERR_NO_CONNECTION = "no network connection";
error ERR_NO_PAYLOAD = "no payload";
error ERR_NO_WIFI = "null wifi";

HttpClient::HttpClient() {
    this->port = 80;
}

void HttpClient::generate_influx_request(char *buf, InfluxConfig &influx) {
    sprintf(buf, "%s %s?bucket=%s&org=%s&precision=s HTTP/1.1\nAuthorization: Token %s\nHost: %s\nContent-Length: %d\n\n%s\n\n",
            this->method, this->endpoint,
            influx.bucket, influx.org, influx.token,
            this->host,
            strlen(this->body),
            this->body);
}
void SlackConfig::generate_request_body(char *buf, char *message) {
    sprintf(buf, "{\"channel\":\"%s\",\"text\":\"%s\"}",
            this->channel,
            message);
}
void HttpClient::generate_slack_request(char *buf, SlackConfig &slack) {
    sprintf(buf, "%s %s HTTP/1.1\nConnection: close\nContent-Type: application/json; charset=utf-8\nAuthorization: Bearer %s\nHost: %s\nContent-Length: %d\n\n%s\n\n",
            this->method, this->endpoint,
            slack.token, this->host,
            strlen(this->body),
            this->body);
}

char responsebuf[256];
error HttpClient::exec(char *request, HttpResponse *response) {
    if (request == nullptr) {
        return ERR_NO_PAYLOAD;
    }
    if (this->net == nullptr) {
        return ERR_NO_WIFI;
    }

    //  run the http connection
    int connState;
    if (this->port == 443) {
        Serial.print("Connecting via SSL to ");
        Serial.print(this->host);
        Serial.print(":");
        Serial.println(this->port);
        connState = this->net->connectSSL(this->host, this->port);
    } else {
        Serial.print("Connecting to ");
        Serial.print(this->host);
        Serial.print(":");
        Serial.println(this->port);
        connState = this->net->connect(this->host, this->port);
    }
    if (connState) {
        net->print(request);
        if (response != nullptr) {
            Serial.println(request);
            // Read the HTTP response to check the status code
            net->readBytes(responsebuf, 256);
            parse_http_response(responsebuf, response);
            Serial.print("HTTP Response: ");
            Serial.print(response->code);
            Serial.print(" ");
            Serial.println(response->status);
            if (response->code >= 300) {
                Serial.println();
                Serial.println(response->body);
            }

        }

    } else {
        Serial.println("Failed to open TCP connection");
        return ERR_NO_CONNECTION;
    }

    this->net->stop();
    return nullptr;
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
