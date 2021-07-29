//
// Created by kit on 7/26/21.
//

#ifndef WANNETIOT_BEDROOM_LIGHTS_H
#define WANNETIOT_BEDROOM_LIGHTS_H

// http://planetpixelemporium.com/tutorialpages/light.html
#define SOFT_WHITE_R 255
#define SOFT_WHITE_G 197
#define SOFT_WHITE_B 143

// LightConfig describes how strong to make each output.
struct LightConfig {
    int Red;
    int Green;
    int Blue;
    int White;
};

// set_lights sets the light intensity on each pin
void set_lights(LightConfig &pins, LightConfig &strength);

// wakeup runs the lights through the morning routine
// Return: 1 if wakeup is in progress, 0 otherwise.
int wakeup(LightConfig &pins, LightConfig &strength);

#endif //WANNETIOT_BEDROOM_LIGHTS_H
