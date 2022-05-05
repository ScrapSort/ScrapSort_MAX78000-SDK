#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "tmr_funcs.h"


typedef struct Ultrasonic_t{
    mxc_gpio_regs_t *trigger_port; 
    uint32_t trigger_mask;

    mxc_gpio_regs_t *echo_port; 
    uint32_t echo_mask;

    uint8_t close_threshold;
    uint8_t far_threshold;

    uint32_t volatile rising_edge_time;
    uint32_t volatile time_interval;
    uint16_t volatile object_status;
    uint32_t volatile object_timestamp;
    uint8_t volatile trigger_state;

    
} Ultrasonic;

// initialize the echo pins and callback functions
void init_echo_gpios();

// initialize the trigger pins
void init_trigger_gpios();

// these functions set the corresponding trigger pin high/low
void trigger_high(Ultrasonic *sensor);
void trigger_low(Ultrasonic *sensor);

// triggers the corresponding sensor
void activate_trigger(Ultrasonic *sensor);

// checks if a sensor needs to be triggered
void to_trigger();

// checks all interrupt flags
void check_all_callbacks();

#endif