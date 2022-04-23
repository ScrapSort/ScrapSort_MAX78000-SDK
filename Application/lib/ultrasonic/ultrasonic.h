#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "tmr_funcs.h"

void init_echo_cam();
void init_echo_flipper0();
void init_echo_flipper1();
void init_trigger();
void trigger_high();
void trigger_low();

#endif