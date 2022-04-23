#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "tmr_funcs.h"

void init_ultrasonic_gpios();
void trigger_high();
void trigger_low();
void init_trigger();

void triggered();

// timer stuff
// void expiration_handler();
// int init_arm_timer();

// // checks all interrupt flags
// void check_all_callbacks();

// // these functions get called by main when
// // the corresponding interrupt flag has been set
// void camera_callback();
// void close_arm_callback();
// void flipper_callback();

#endif