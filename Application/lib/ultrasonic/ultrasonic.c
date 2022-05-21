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
#define DEBOUNCE 3000 // 1 gc = 100us --> 1000 gc = 100ms
#define LPF_COEFF 1

// store interrupt callback functions
flag_callback flag_callback_funcs[NUM_FLAGS];
void* flag_callback_params[NUM_FLAGS];

uint16_t ultrasonic_active_time = 10;

Ultrasonic sensor_cam;
Ultrasonic sensor_flipper1;
Ultrasonic sensor_flipper2;
Ultrasonic sensor_flipper3;
Ultrasonic* sensors[4] = {&sensor_cam, &sensor_flipper1,&sensor_flipper2,&sensor_flipper3};
volatile Ultrasonic* active_ultrasonic_sensor;

//volatile queue task_queue;

void echo_handler(void* sensor)
{
    Ultrasonic* ultrasonic_sensor = sensor;
    //printf("In echo\n");
    // first interrupt (rising edge)
    if(ultrasonic_sensor->curr_rising_edge_global_count == 0)
    {
        // store the start time
        ultrasonic_sensor->curr_rising_edge_global_count = global_counter;
        //printf("Global Counter: %d\n", global_counter);
    }
    // second interrupt (falling edge)
    else
    {   
        //printf("Falling\n");
        // store the end time, convert to cm, reset the start time
        ultrasonic_sensor->curr_raw_distance_cm = (global_counter - ultrasonic_sensor->curr_rising_edge_global_count)*100/58.0;
        //Apply simple low-pass filter
        ultrasonic_sensor->curr_distance_cm = LPF_COEFF * ultrasonic_sensor->curr_raw_distance_cm + (1-LPF_COEFF)*ultrasonic_sensor->last_raw_distance_cm;
        ultrasonic_sensor->last_raw_distance_cm = ultrasonic_sensor->curr_raw_distance_cm;  
        ultrasonic_sensor->curr_rising_edge_global_count = 0;
    }
}


void camera_callback(void* cb_data)
{
    // static cnn_output_t output;

    // // call camera take picture
    // output = *run_cnn();

    // show_cnn_output(output);

    // int class_type = output.output_class;
    // //printf("class type: %s\n", class_strings[class_type]);
    printf("camera callback\n");
}

void flipper_callback(void* cb_data)
{   
    Ultrasonic *sensor = *(Ultrasonic**)(cb_data);
    // // do the arm movement test
    // target_tics(flipper_num,-40);
    // MXC_Delay(600000);
    // go_home_forward(flipper_num);

    printf("flipper %i callback\n",sensor->sensor_id);
}


void init_ultrasonic_sensor(Ultrasonic *sensor, mxc_gpio_regs_t *trigger_port, uint32_t trigger_mask, mxc_gpio_regs_t *echo_port, uint32_t echo_mask, uint8_t sensor_id, flag_callback cb_func){ 
    // Configure trigger pin (output)
    sensor->trigger_config.port = trigger_port;
    sensor->trigger_config.mask = trigger_mask;
    sensor->trigger_config.func = MXC_GPIO_FUNC_OUT;
    sensor->trigger_config.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&(sensor->trigger_config));
    MXC_GPIO_SetVSSEL(sensor->trigger_config.port, sensor->trigger_config.vssel, sensor->trigger_config.mask);
    trigger_low(sensor);
    
    
    // Configure echo pin (input)
    sensor->echo_config.port = echo_port;
    sensor->echo_config.mask = echo_mask;
    sensor->echo_config.func = MXC_GPIO_FUNC_IN;
    sensor->echo_config.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&(sensor->echo_config));
    MXC_GPIO_SetVSSEL(sensor->echo_config.port, sensor->echo_config.vssel, sensor->echo_config.mask);

    // function to call and data to pass in when interrupt triggered
    MXC_GPIO_RegisterCallback(&(sensor->echo_config), echo_handler, (void*)sensor);
    
    // enable interrupt on both edges
    MXC_GPIO_IntConfig(&(sensor->echo_config), MXC_GPIO_INT_BOTH);
    MXC_GPIO_EnableInt(sensor->echo_config.port, sensor->echo_config.mask);
    NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(sensor->echo_config.port)));
    
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

    // configure sensor identifier
    sensor->sensor_id = sensor_id;

    // configure the sensor's callback function
    sensor->cb_func = cb_func;
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

void activate_trigger_callback(void *cb_data){
    Ultrasonic *sensor = *(Ultrasonic**)(cb_data);

    // first update the state of the last sensor
    int last_id = (sensor->sensor_id-1 >= 0) ? sensor->sensor_id-1 : 3;
    Ultrasonic* last_sensor = sensors[last_id];

    if(!last_sensor->object_status && last_sensor->curr_distance_cm < CLOSE_THRESH)
    {
        // there is an object in front of the sensor
        last_sensor->object_status = 1; // state update
        last_sensor->object_timestamp = global_counter;
        set_flag(last_id); // will trigger arm to close in main
        //printf("present: %d\n",last_id);
        
        // printf("S2: %d, %d cm\n",object_statuses[2],time_intervals[2]);
        // printf("S1: %d, %d cm\n",object_statuses[1],time_intervals[1]);
        // printf("S0: %d, %d cm\n",object_statuses[0],time_intervals[0]);
        // printf("S3: %d, %d cm\n",object_statuses[3],time_intervals[3]);
        // printf("\033[0;0f");
    }
    // update the timestamp if object still there
    else if(last_sensor->object_status && last_sensor->curr_distance_cm < CLOSE_THRESH)
    {
        last_sensor->object_timestamp = global_counter;
    }
    // object in front of the sensor and beyond the threshold, update the state
    else if(last_sensor->object_status && last_sensor->curr_distance_cm >= FAR_THRESH && ((global_counter-last_sensor->object_timestamp) > DEBOUNCE))
    {
        // reset the state
        last_sensor->object_status = 0;
        //printf("left: %d\n",last_id);
        // printf("S2: %d, %d cm\n",object_statuses[2],time_intervals[2]);
        // printf("S1: %d, %d cm\n",object_statuses[1],time_intervals[1]);
        // printf("S0: %d, %d cm\n",object_statuses[0],time_intervals[0]);
        // printf("S3: %d, %d cm\n",object_statuses[3],time_intervals[3]);
        // printf("\033[0;0f");
    }

    // now trigger the next sensor
    trigger_high(sensor);
    MXC_Delay(10);
    trigger_low(sensor);

    //printf("activate trigger callback: %i\n",sensor->sensor_id);

    // set the next active sensor
    int next_id = (sensor->sensor_id+1 == 4) ? 0 : sensor->sensor_id+1;
    active_ultrasonic_sensor = sensors[next_id];
}

void trigger_high(Ultrasonic *sensor){
    MXC_GPIO_OutSet(sensor->trigger_config.port, sensor->trigger_config.mask);
}

void trigger_low(Ultrasonic *sensor){
    MXC_GPIO_OutClr(sensor->trigger_config.port, sensor->trigger_config.mask);
}


void ultrasonic_timer_expiration_handler()
{
    // Clear interrupt, stop timer
    MXC_TMR_ClearFlags(MXC_TMR2);
    MXC_TMR_Stop(MXC_TMR2);
    
    // reset the timer
    MXC_TMR2->cnt = 0;

    //printf("ultrasonic timer expired\n");
    // start the next timer
    MXC_TMR_Start(MXC_TMR2);

    // add interrupt trigger to event queue
    set_flag(ULTRASONIC_FIRE);
}


void init_ultrasonic_timer()
{
    // setup the interrupt for timer 2
    NVIC_SetVector(TMR2_IRQn, ultrasonic_timer_expiration_handler);
    NVIC_EnableIRQ(TMR2_IRQn);

    // init timer 0 to interrupt every expiration period 50 ms
    // (32KHz clock with prescaler 32 and count compare ultrasonic_active_time)
    MXC_TMR_Shutdown(MXC_TMR2);

    mxc_tmr_cfg_t tmr;
    tmr.pres = TMR_PRES_32; // frequency becomes 1kHz (ticks once every ms)
    tmr.mode = TMR_MODE_CONTINUOUS;
    tmr.bitMode = TMR_BIT_MODE_32;
    tmr.clock = MXC_TMR_32K_CLK;
    tmr.cmp_cnt = ultrasonic_active_time; // ticks 50 times (50ms) before triggering interrupt
    tmr.pol = 0;
    
    // init the timer
    if (MXC_TMR_Init(MXC_TMR2, &tmr, true) != E_NO_ERROR) 
    {
        printf("Failed ultrasonic timer Initialization.\n");
    }
    
    // set up the callback for firing the ultrasonic
    flag_callback_funcs[ULTRASONIC_FIRE] = activate_trigger_callback;
    flag_callback_params[ULTRASONIC_FIRE] = (void*)(&active_ultrasonic_sensor);

    // enable the interrupt
    MXC_TMR_EnableInt(MXC_TMR2);

    printf("Ultrasonic timer initialized.\n\n");
}


// void init_task_queue()
// {
//     // 20 tasks on the queue max
//     task_queue = Queue(20);
// }

void init_ultrasonic_sensors()
{
    init_ultrasonic_sensor(&sensor_cam, MXC_GPIO2, MXC_GPIO_PIN_4, MXC_GPIO2, MXC_GPIO_PIN_3,0,camera_callback);
    init_ultrasonic_sensor(&sensor_flipper1, MXC_GPIO1, MXC_GPIO_PIN_0, MXC_GPIO1, MXC_GPIO_PIN_1,1,flipper_callback);
    init_ultrasonic_sensor(&sensor_flipper2, MXC_GPIO2, MXC_GPIO_PIN_6, MXC_GPIO2, MXC_GPIO_PIN_7,2,flipper_callback);
    init_ultrasonic_sensor(&sensor_flipper3, MXC_GPIO3, MXC_GPIO_PIN_1, MXC_GPIO1, MXC_GPIO_PIN_6,3,flipper_callback);

    flag_callback_funcs[CAMERA] = camera_callback;
    flag_callback_params[CAMERA] = &sensors[0];
    flag_callback_funcs[FLIPPER_1] = flipper_callback;
    flag_callback_params[FLIPPER_1] = &sensors[1];
    flag_callback_funcs[FLIPPER_2] = flipper_callback;
    flag_callback_params[FLIPPER_2] = &sensors[2];
    flag_callback_funcs[FLIPPER_3] = flipper_callback;
    flag_callback_params[FLIPPER_3] = &sensors[3];

    // set the first active sensor
    active_ultrasonic_sensor = sensors[0];

    printf("initialized ultrasonic sensors, start timer\n");
    MXC_TMR_Start(MXC_TMR2);
}