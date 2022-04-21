#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "tmr_funcs.h"

void init_echo();
void init_trigger();
void trigger_high();
void trigger_low();

#endif