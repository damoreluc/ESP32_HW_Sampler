#include <acquisition/sampler.h>

// mappatura dei tre canali analogici Ax, Ay, Az
gpio_num_t channels[ ] = {GPIO_NUM_36, GPIO_NUM_39, GPIO_NUM_32};

void getSample(volatile Sample *sample, uint32_t dt) {
    sample->timestamp = dt;
    sample->iax = analogRead(channels[0]);
    sample->iay = analogRead(channels[1]);
    sample->iaz = analogRead(channels[2]);
}