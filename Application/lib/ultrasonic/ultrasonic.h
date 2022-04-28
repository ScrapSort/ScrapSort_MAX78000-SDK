#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "tmr_funcs.h"

// initialize the echo pins and callback functions
void init_echo_gpios();

// initialize the trigger pins
void init_trigger_gpios();

// these functions set the corresponding trigger pin high/low
void trigger0_high();
void trigger0_low();
void trigger1_high();
void trigger1_low();
void trigger2_high();
void trigger2_low();
void triggercam_high();
void triggercam_low();

// triggers the corresponding sensor
void activate0();
void activate1();
void activate2();
void activatecam();

// checks if a sensor needs to be triggered
void to_trigger();

// these functions are used to set up a timer for closing the arms
// void expiration_handler();
// int init_arm_timer();

// checks all interrupt flags
void check_all_callbacks();

#endif