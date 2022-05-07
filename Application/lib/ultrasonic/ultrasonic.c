#include "ultrasonic.h"
#include "motor_funcs.h"
#include "sorter.h"
#include "flags.h"
#include "motor_funcs.h"
#include "cnn_helper_funcs.h"

#include "mxc_device.h"
#include "mxc_delay.h"
#include "nvic_table.h"
#include "board.h"
#include "gpio.h"
#include "mxc.h"
#include "mxc_sys.h"
#include "nvic_table.h"
#include "gcr_regs.h"
#include "tmr_funcs.h"

// thresholds for ultrasonic distance
// 2 cm null area to avoid oscillation when in between
#define CLOSE_THRESH 18 /// cm
#define FAR_THRESH 22 // cm
#define DEBOUNCE 10000 // 1 gc = 100us --> 1000 gc = 1sec
#define LPF_COEFF 1

// the ultrasonic sensor that is currently firing
Flag volatile active_sensor = FLIPPER_0;

// store interrupt callback functions
flag_callback flag_callback_funcs[NUM_FLAGS];
uint8_t flag_callback_params[NUM_FLAGS] = {0};



void echo_handler(void* sensor)
{
    // sensor = *(Ultrasonic*)(sensor);
    //TODO Remove
   
    // don't allow nonactive sensors to triger interrupts
    // for example if there is interference between sensors
    // if(ultrasonic_sensor.sensor_type != active_sensor)
    // {
    //     return;
    // }
    Ultrasonic* ultrasonic_sensor = sensor;
    printf("In echo\n");
    // first interrupt (rising edge)
    if(ultrasonic_sensor->curr_rising_edge_global_count == 0)
    {
         
        // store the start time
        ultrasonic_sensor->curr_rising_edge_global_count = global_counter;
        printf("Global Counter: %d\n", global_counter);
    }
    // second interrupt (falling edge)
    else
    {   
        printf("Falling\n");
        // store the end time, convert to cm, reset the start time
        ultrasonic_sensor->curr_raw_distance_cm = (global_counter - ultrasonic_sensor->curr_rising_edge_global_count)*100/58.0;
        //Apply simple low-pass filter
        ultrasonic_sensor->curr_distance_cm = LPF_COEFF * ultrasonic_sensor->curr_raw_distance_cm + (1-LPF_COEFF)*ultrasonic_sensor->last_raw_distance_cm;
        ultrasonic_sensor->last_raw_distance_cm = ultrasonic_sensor->curr_raw_distance_cm;  
        ultrasonic_sensor->curr_rising_edge_global_count = 0;


        // active_sensor += 1;
        // if(active_sensor == 4)
        // {
        //     active_sensor = 0;
        // }
        // trigger_state[active_sensor] = 1;
    }
}


void camera_callback(uint8_t cb_data)
{
    static cnn_output_t output;

    // call camera take picture
    output = *run_cnn();

    show_cnn_output(output);

    int class_type = output.output_class;
    //printf("class type: %s\n", class_strings[class_type]);
}

void flipper_callback(uint8_t flipper_num)
{   
    // do the arm movement test
    target_tics(flipper_num,-40);
    MXC_Delay(600000);
    go_home_forward(flipper_num);
}


void init_ultrasonic_sensor(Ultrasonic *sensor, mxc_gpio_regs_t *trigger_port, uint32_t trigger_mask, mxc_gpio_regs_t *echo_port, uint32_t echo_mask){ 
    //Configure trigger pin (output)
    sensor->trigger_config.port = trigger_port;
    sensor->trigger_config.mask = trigger_mask;
    sensor->trigger_config.func = MXC_GPIO_FUNC_OUT;
    sensor->trigger_config.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&(sensor->trigger_config));
    MXC_GPIO_SetVSSEL(sensor->trigger_config.port, sensor->trigger_config.vssel, sensor->trigger_config.mask);
    trigger_low(sensor);
    
    
    //Configure echo pin (input)
    sensor->echo_config.port = echo_port;
    sensor->echo_config.mask = echo_mask;
    sensor->echo_config.func = MXC_GPIO_FUNC_IN;
    sensor->echo_config.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&(sensor->echo_config));
    MXC_GPIO_SetVSSEL(sensor->echo_config.port, sensor->echo_config.vssel, sensor->echo_config.mask);

    // MXC_Delay(MXC_DELAY_MSEC(60));
    // printf("Test 6\n");
    MXC_GPIO_RegisterCallback(&(sensor->echo_config), echo_handler, (void*)sensor);
    // printf("Test 7\n");
    MXC_GPIO_IntConfig(&(sensor->echo_config), MXC_GPIO_INT_BOTH);
    // printf("Test 8\n");
    MXC_GPIO_EnableInt(sensor->echo_config.port, sensor->echo_config.mask);
    // printf("Test 9\n");
    NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(sensor->echo_config.port)));
    printf("Test 10\n");
    
    //set other data as defaults
    sensor->curr_rising_edge_global_count = 0;
    sensor->curr_raw_distance_cm = 100;
    sensor->last_raw_distance_cm = 0;
    sensor->curr_distance_cm = 100;
    sensor->object_status = 0;
    sensor->object_timestamp = 0;
    sensor->trigger_state = 0;

    sensor->close_threshold = CLOSE_THRESH;
    sensor->far_threshold = FAR_THRESH;
    sensor->debounce_us = DEBOUNCE;
}


void check_all_callbacks()
{
    for (Flag f = 0; f < NUM_FLAGS; f++) 
    {
        // if a flag is set, call its handler code, then reset the flag
        if (is_flag_set(f))
        {
            (*flag_callback_funcs[f])(flag_callback_params[f]);
            unset_flag(f);
        }
    } 
}

void activate_trigger(Ultrasonic *sensor){
    trigger_high(sensor);
    MXC_Delay(10);
    trigger_low(sensor);
}

void trigger_high(Ultrasonic *sensor){
    MXC_GPIO_OutSet(sensor->trigger_config.port, sensor->trigger_config.mask);
}

void trigger_low(Ultrasonic *sensor){
    MXC_GPIO_OutClr(sensor->trigger_config.port, sensor->trigger_config.mask);
}

