#ifndef _SAMPLE_TYPE_H
#define _SAMPLE_TYPE_H

#include <Arduino.h>

typedef struct {
    uint32_t timestamp;
    int16_t iax;
    int16_t iay;
    int16_t iaz;
} Sample;

#endif