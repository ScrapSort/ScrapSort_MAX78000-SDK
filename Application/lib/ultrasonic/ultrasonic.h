#ifndef ULTRASONIC_H
#define ULTRASONIC_H

/***** Includes *****/
#include "tmr_funcs.h"

/*
    Description: This function initializes the GPIOs for the ultrasonic
                 echo pins. This includes registering an interrupt service
                 routine triggered by rising and falling edges for each echo pin. In
                 addition, each echo pin has a corresponding callback function
                 which gets scheduled when the interrupt happens (i.e. we don't 
                 complete the task in the interrupt but in the main thread). 

    Parameters: none

    Return: none
*/
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
void activate_trigger0();
void activate_trigger1();
void activate_trigger2();
void activate_triggercam();

// checks if a sensor needs to be triggered
void to_trigger();

// these functions are used to set up a timer for closing the arms
void expiration_handler();
int init_arm_timer();

// checks all interrupt flags
void check_all_callbacks();

void get_heartbeat();

#endif