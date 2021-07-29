//
// Created by kit on 7/26/21.
//
#include <Arduino.h>
#include <WiFiNINA.h>
#include <ctime>
#include "lights.h"

void set_lights(LightConfig &pins, LightConfig &strength) {
    // TODO: add support for a fade speed or flicker effect
    analogWrite(pins.Red, strength.Red);
    analogWrite(pins.Green, strength.Green);
    analogWrite(pins.Blue, strength.Blue);
    analogWrite(pins.White, strength.White);
}

int wakeup(LightConfig &pins, LightConfig &strength) {
    // Gradually fade the lights in starting at 5am through 5:30am
    // TODO: allow the remote to override the setting either full-on or full-off. The security alarm also needs to be able to fire during this period.
    time_t t = WiFi.getTime();
    struct tm *now = localtime(&t);

    if (now->tm_hour == 5 && now->tm_min <= 30) {
        strength.Red = SOFT_WHITE_R;
        strength.Green = SOFT_WHITE_G;
        strength.Blue = SOFT_WHITE_B;

        double pct = (double)now->tm_min / 30.0;
        strength.White = (int) (pct * 255.0);
        set_lights(pins, strength);
        return 1;
    }
    return 0;
}