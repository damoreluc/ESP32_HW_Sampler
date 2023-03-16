#include <acquisition/sampler.h>

// mappatura dei tre canali analogici Ax, Ay, Az
gpio_num_t channels[ ] = {GPIO_NUM_36, GPIO_NUM_39, GPIO_NUM_32};
// durata: 254us
void getSample(volatile Sample *sample) {
    digitalWrite(GPIO_NUM_23, HIGH);  
    sample->iax = analogRead(channels[0]); 
    sample->iay = analogRead(channels[1]);
    sample->iaz = analogRead(channels[2]);
    digitalWrite(GPIO_NUM_23, LOW); 
}