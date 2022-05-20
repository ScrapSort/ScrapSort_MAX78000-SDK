#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "flags.h"
#include "gpio.h"

typedef struct Ultrasonic_t{
    // gpio configuration
    mxc_gpio_cfg_t trigger_config;
    mxc_gpio_cfg_t echo_config;

    // distance in cm to for present object
    uint8_t close_threshold;

    // distance in cm for no object present
    uint8_t far_threshold;

    // rising edge time for echo
    uint32_t volatile curr_rising_edge_global_count;

    // pulse width in cm, init to 100 to prevent false positive on startup
    uint32_t volatile curr_raw_distance_cm; // distance read currently (x[n])
    uint32_t volatile last_raw_distance_cm; // exponential running average distance (y[n-1])
    uint32_t volatile curr_distance_cm; // result of exponential average (y[n])

    // track if object in front of sensor
    uint16_t volatile object_status;
   
    // store when the object was first present
    uint32_t volatile object_timestamp;

    // minimum time object is present, 1 gc = 100us --> 1000 gc = 1sec
    uint16_t debounce_us;

    // track if sensor needs to fire
    uint8_t volatile trigger_state;

    // sensor identifier for the callback
    uint8_t sensor_id;

    // callback function
    flag_callback cb_func;

} Ultrasonic;

// initialize the ultrasonic sensor struct
void init_ultrasonic_sensor(Ultrasonic *sensor, mxc_gpio_regs_t *trigger_port, uint32_t trigger_mask, mxc_gpio_regs_t *echo_port, uint32_t echo_mask, uint8_t sensor_id, flag_callback cb_func);

// these functions set the corresponding trigger pin high/low
void trigger_high(Ultrasonic *sensor);
void trigger_low(Ultrasonic *sensor);

// triggers the corresponding sensor
void activate_trigger_callback(void *cb_data);

// checks if a sensor needs to be triggered
void to_trigger();

// checks all interrupt flags
void check_all_callbacks();

// continuous timer for triggering ultrasonic sensors in round robin fashion
void init_ultrasonic_timer();

// initiailizes a queue for handling tasks triggered by interrupts
void init_task_queue();

void init_ultrasonic_sensors();

#endif