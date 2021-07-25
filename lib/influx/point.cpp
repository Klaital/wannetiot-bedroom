//
// Created by kit on 7/23/21.
//

#include <malloc.h>
#include <Arduino.h>
#include "point.h"

void init_point(Point *p) {
    p->tag_count = 0;
    p->field_count = 0;
}


void point_to_str(char *buf, Point *p) {
    int i;
    char tsbuf[10];

    strcat(buf, p->measurement);
    for(i=0; i < p->tag_count; i++) {
        strcat(buf, ",");
        strcat(buf, p->tag_names[i]);
        strcat(buf, "=");
        strcat(buf, p->tag_values[i]);
    }

    strcat(buf, " ");
    for(i=0; i < p->field_count; i++) {
        if (i > 0) {
            strcat(buf, ",");
        }
        strcat(buf, p->field_names[i]);
        strcat(buf, "=");
        strcat(buf, p->field_values[i]);
    }

    sprintf(tsbuf, " %ld", p->timestamp);
    strcat(buf, tsbuf);
}

int validate_point(Point *p) {
    if (p->measurement == nullptr) {
        return 0;
    }

    if (p->field_count == 0) {
        return 0;
    }

    int i;
    for (i=0; i < p->tag_count; i++) {
        if (p->tag_names[i] == nullptr || p->tag_values[i] == nullptr) {
            return 0;
        }
    }

    for (i=0; i < p->field_count; i++) {
        if (p->field_names[i] == nullptr || p->field_values[i] == nullptr) {
            return 0;
        }
    }

    // success!
    return 1;
}

void set_measurement(Point *p, char *measurement) {
    size_t bytes = strlen(measurement);
    p->measurement = (char*)malloc(bytes+1);
    strcpy(p->measurement, measurement);
}

void add_tag(Point *p, char *name, char *value) {
    size_t name_bytes = strlen(name)+1;
    p->tag_names[p->tag_count] = (char*)malloc(name_bytes);
    strcpy(p->tag_names[p->tag_count], name);

    size_t value_bytes = strlen(value)+1;
    p->tag_values[p->tag_count] = (char*)malloc(value_bytes);
    strcpy(p->tag_values[p->tag_count], value);

    p->tag_count++;
}

void add_field(Point *p, char *name, char *value) {
    size_t name_bytes = strlen(name)+1;
    p->field_names[p->field_count] = (char*)malloc(name_bytes);
    strcpy(p->field_names[p->field_count], name);

    size_t value_bytes = strlen(value)+1;
    p->field_values[p->field_count] = (char*)malloc(value_bytes);
    strcpy(p->field_values[p->field_count], value);

    p->field_count++;
}

void reset_point(Point *p) {
    int i;
    free(p->measurement);
    for (i=0; i < p->tag_count; i++) {
        free(p->tag_names[i]);
        free(p->tag_values[i]);
    }
    p->tag_count = 0;

    for (i=0; i < p->field_count; i++) {
        free(p->field_names[i]);
        free(p->field_values[i]);
    }
    p->field_count = 0;
}
