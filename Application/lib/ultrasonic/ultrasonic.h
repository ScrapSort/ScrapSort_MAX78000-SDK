#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "tmr_funcs.h"

// initialize the echo pins
void init_ultrasonic_gpios();

// initialize the trigger pins
void init_trigger();

// set0 is even flippers (flipper 0 and 2)
// set 1 is odd flippers (cam=3 and flipper 1)
// these functions set the corresponding trigger pin high/low
void trigger0_high();
void trigger0_low();
void trigger1_high();
void trigger1_low();
void trigger2_high();
void trigger2_low();
void triggercam_high();
void triggercam_low();

// triggers the corresponding set
void activate0();
void activate1();
void activate2();
void activatecam();

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