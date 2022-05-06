#ifndef ULTRASONIC_H
#define ULTRASONIC_H

// #include "tmr_funcs.h"

#include "flags.h"

typedef struct Ultrasonic_t{
    // mxc_gpio_regs_t *trigger_port; 
    // uint32_t trigger_mask;
    mxc_gpio_cfg_t trigger_config;

    // mxc_gpio_regs_t *echo_port; 
    // uint32_t echo_mask;
    mxc_gpio_cfg_t echo_config;

    //cm
    uint8_t close_threshold;
    //cm
    uint8_t far_threshold;
    //1 gc = 100us --> 1000 gc = 1sec
    uint8_t debounce_us;

    //rising edge time
    uint32_t volatile curr_rising_edge_global_count;
    //pulse width in ticks, init to 100 to prevent false positive on startup
    uint32_t volatile curr_raw_distance_cm;
    uint32_t volatile last_raw_distance_cm;
    uint32_t volatile curr_distance_cm;
    //track if object in front of sensor
    uint16_t volatile object_status;
    //debouncing
    uint32_t volatile object_timestamp;
    //track if sensor needs to fire
    uint8_t volatile trigger_state;

    Flag sensor_type;    
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