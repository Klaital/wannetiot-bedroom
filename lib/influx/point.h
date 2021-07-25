//
// Created by kit on 7/23/21.
//

#ifndef WANNETIOT_BEDROOM_POINT_H
#define WANNETIOT_BEDROOM_POINT_H

#include <ctime>
#include <cstring>
#include <cstdio>



#define INFLUX_MAX_TAGS 10
#define INFLUX_MAX_FIELDS 10

struct Point {
    char *measurement;
    char *tag_names[INFLUX_MAX_TAGS];
    char *tag_values[INFLUX_MAX_TAGS];
    int tag_count;
    char *field_names[INFLUX_MAX_FIELDS];
    char *field_values[INFLUX_MAX_FIELDS];
    int field_count;

    time_t timestamp;
};

// init_point initializes a new Point with the common default values
void init_point(Point *p);
// init_point frees any allocated memory for tags and fields and resets the stacks for them.
void reset_point(Point *p);

// set_measurement copies in the measurement name string
void set_measurement(Point *p, char *measurement);
// add_tag copies in the name/value for a data point's tag
void add_tag(Point *p, char *name, char *value);
// add_field copies in the name/value for a data point's field
void add_field(Point *p, char *name, char *value);

// point_to_str serializes a Point into the Influx wire protocol
// https://docs.influxdata.com/influxdb/v1.8/write_protocols/line_protocol_reference/
// Assumes that buf is already a valid string to be appended to.
void point_to_str(char *buf, Point *p);

// validate_point checks whether a Point is ready to be serialized, with all fields filled in.
int validate_point(Point *p);


#endif //WANNETIOT_BEDROOM_POINT_H
