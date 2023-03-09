#ifndef _SAMPLER_H
#define _SAMPLER_H

#include <Arduino.h>
#include <userdatatypes/sampletype.h>

void getSample(volatile Sample *sample, uint32_t dt);

#endif