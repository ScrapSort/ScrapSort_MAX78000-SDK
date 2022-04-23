#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "tmr_funcs.h"

void init_ultrasonic_gpios();
void trigger_high();
void trigger_low();

#endif