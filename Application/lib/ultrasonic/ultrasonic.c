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

// thresholds for ultrasonic distance
// 2 cm null area to avoid oscillation when in between
#define CLOSE_THRESH 18 /// cm
#define FAR_THRESH 22 // cm
#define DEBOUNCE 10000 // 1 gc = 100us --> 1000 gc = 1sec

// interrupt function parameters (pass these to echo interrupt)
Flag camera_idx = CAMERA;
Flag flipper0_idx = FLIPPER_0;
Flag flipper1_idx = FLIPPER_1;
Flag flipper2_idx = FLIPPER_2;

// the ultrasonic sensor that is currently firing
Flag volatile active_sensor = CAMERA;

// gpios for trigger and echos
mxc_gpio_cfg_t trigger1_gpio;
mxc_gpio_cfg_t trigger0_gpio;
mxc_gpio_cfg_t trigger2_gpio;
mxc_gpio_cfg_t triggercam_gpio;
mxc_gpio_cfg_t echo_cam_gpio;
mxc_gpio_cfg_t echo_flipper0_gpio;
mxc_gpio_cfg_t echo_flipper1_gpio;
mxc_gpio_cfg_t echo_flipper2_gpio;

// state variables for ultrasonic sensors
uint32_t volatile current_pulse_values[] = {0,0,0,0}; // rising edge time
uint32_t volatile time_intervals[] = {100,100,100,100}; // pulse width in ticks, init to 100 to prevent false alarm on init
uint16_t volatile object_statuses[] = {0,0,0,0}; // state variable to track if object in front of sensor
uint32_t volatile object_timestamps[] = {0,0,0,0}; // debouncing
uint8_t volatile trigger_state[] = {0,0,0,0}; // state variable to track if a sensor needs to fire

// store interrupt callback functions
flag_callback flag_callback_funcs[NUM_FLAGS];
uint8_t flag_callback_params[NUM_FLAGS] = {0};


void camera_callback(uint8_t cb_data)
{
    static cnn_output_t output;

    // call camera take picture
    output = *run_cnn();

    show_cnn_output(output);

    int class_type = output.output_class;
    //printf("class type: %s\n", class_strings[class_type]);
}


void echo_handler(void* cb_data)
{
    // get the sensor idx from the callback data
    Flag volatile sensor_idx = *(Flag*)(cb_data);

    // don't allow nonactive sensors to triger interrupts
    // for example if there is interference between sensors
    if(sensor_idx != active_sensor)
    {
        return;
    }

    // first interrupt (rising edge)
    if(current_pulse_values[sensor_idx] == 0)
    {
        // store the start time
        current_pulse_values[sensor_idx] = global_counter;
    }
    // second interrupt (falling edge)
    else
    {
        // store the end time, convert to cm, reset the start time
        time_intervals[sensor_idx] = (global_counter - current_pulse_values[sensor_idx])*100/58;
        current_pulse_values[sensor_idx] = 0;

        // no object in front of the sensor yet and it is within the threshold, trigger the arm to close
        if(!object_statuses[sensor_idx] && time_intervals[sensor_idx] < CLOSE_THRESH)
        {
            // there is an object in front of the sensor
            object_statuses[sensor_idx] = 1; // state update
            object_timestamps[sensor_idx] = global_counter;
            set_flag(sensor_idx); // will trigger arm to close in main
            //printf("present: %d\n",sensor_idx);
            
            // printf("S2: %d\n",object_statuses[2]);
            // printf("S1: %d\n",object_statuses[1]);
            // printf("S0: %d\n",object_statuses[0]);
            // printf("S3: %d, %d cm\n",object_statuses[3],time_intervals[sensor_idx]);
            // printf("\033[0;0f");
        }
        // update the timestamp if object still there
        else if(object_statuses[sensor_idx] && time_intervals[sensor_idx] < CLOSE_THRESH)
        {
            object_timestamps[sensor_idx] = global_counter;
        }
        // object in front of the sensor and beyond the threshold, update the state
        else if(object_statuses[sensor_idx] && time_intervals[sensor_idx] >= FAR_THRESH && ((global_counter-object_timestamps[sensor_idx]) > DEBOUNCE))
        {
            // reset the state
            object_statuses[sensor_idx] = 0;
            //printf("left: %d\n",sensor_idx);
            // printf("S2: %d\n",object_statuses[2]);
            // printf("S1: %d\n",object_statuses[1]);
            // printf("S0: %d\n",object_statuses[0]);
            // printf("S3: %d, %d cm\n",object_statuses[3],time_intervals[sensor_idx]);
            // printf("\033[0;0f");
        }
        // if(object_statuses[3])
        //     printf("gc: %d ot: %d tsa: %d\n",global_counter, object_timestamps[3],global_counter-object_timestamps[3]);
        // after receiving a response, tell the next sensor to trigger
        active_sensor += 1;
        if(active_sensor == 4)
        {
            active_sensor = 0;
        }
        trigger_state[active_sensor] = 1;
    }
}

void flipper_callback(uint8_t flipper_num)
{   
    // do the arm movement test
    target_tics(flipper_num,-40);
    MXC_Delay(600000);
    go_home_forward(flipper_num);
}

void to_trigger()
{
    // check if any sensor needs to be triggered
    Flag sensor_idx = -1;
    for(int i = 0; i < 4; i++)
    {
        if(trigger_state[i] == 1)
        {
            sensor_idx = i;
            break;
        }
    }
    // if none need to be triggered, return
    if(sensor_idx == -1)
    {
        return;
    }
    
    // trigger the corresponding sensor
    switch (sensor_idx)
    {
        case CAMERA:
        {
            activate_triggercam();
            trigger_state[CAMERA] = 0;
            break;
        }

        case FLIPPER_0:
        {
            activate_trigger0();
            trigger_state[FLIPPER_0] = 0;
            break;
        }

        case FLIPPER_1:
        {
            activate_trigger1();
            trigger_state[FLIPPER_1] = 0;
            break;
        }

        case FLIPPER_2:
        {
            activate_trigger2();
            trigger_state[FLIPPER_2] = 0;
            break;
        }
        
        default:
        {
            break;
        }
    }
}

void init_echo_gpios()
{
    // cam echo gpio
    echo_cam_gpio.port = MXC_GPIO1;
    echo_cam_gpio.mask = MXC_GPIO_PIN_6;
    echo_cam_gpio.func = MXC_GPIO_FUNC_IN;
    echo_cam_gpio.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&echo_cam_gpio);
    MXC_GPIO_RegisterCallback(&echo_cam_gpio, echo_handler, (void*)&camera_idx);
    MXC_GPIO_IntConfig(&echo_cam_gpio, MXC_GPIO_INT_BOTH);
    MXC_GPIO_EnableInt(echo_cam_gpio.port, echo_cam_gpio.mask);
    NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(MXC_GPIO1)));
    flag_callback_funcs[CAMERA] = camera_callback;
    flag_callback_params[CAMERA] = 3; 

    // flipper 0 echo gpio
    echo_flipper0_gpio.port = MXC_GPIO2;
    echo_flipper0_gpio.mask = MXC_GPIO_PIN_3;
    echo_flipper0_gpio.func = MXC_GPIO_FUNC_IN;
    echo_flipper0_gpio.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&echo_flipper0_gpio);
    MXC_GPIO_RegisterCallback(&echo_flipper0_gpio, echo_handler, (void*)&flipper0_idx);
    MXC_GPIO_IntConfig(&echo_flipper0_gpio, MXC_GPIO_INT_BOTH);
    MXC_GPIO_EnableInt(echo_flipper0_gpio.port, echo_flipper0_gpio.mask);
    NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(MXC_GPIO2)));
    flag_callback_funcs[FLIPPER_0] = flipper_callback;
    flag_callback_params[FLIPPER_0] = 0; 

    // // // flipper 1 echo gpio
    echo_flipper1_gpio.port = MXC_GPIO2;
    echo_flipper1_gpio.mask = MXC_GPIO_PIN_7;
    echo_flipper1_gpio.func = MXC_GPIO_FUNC_IN;
    echo_flipper1_gpio.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&echo_flipper1_gpio);
    MXC_GPIO_RegisterCallback(&echo_flipper1_gpio, echo_handler, (void*)&flipper1_idx);
    MXC_GPIO_IntConfig(&echo_flipper1_gpio, MXC_GPIO_INT_BOTH);
    MXC_GPIO_EnableInt(echo_flipper1_gpio.port, echo_flipper1_gpio.mask);
    NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(MXC_GPIO2)));
    flag_callback_funcs[FLIPPER_1] = flipper_callback;
    flag_callback_params[FLIPPER_1] = 1; 

    // // flipper 2 echo gpio
    echo_flipper2_gpio.port = MXC_GPIO1;
    echo_flipper2_gpio.mask = MXC_GPIO_PIN_1;
    echo_flipper2_gpio.func = MXC_GPIO_FUNC_IN;
    echo_flipper2_gpio.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&echo_flipper2_gpio);
    MXC_GPIO_RegisterCallback(&echo_flipper2_gpio, echo_handler, (void*)&flipper2_idx);
    MXC_GPIO_IntConfig(&echo_flipper2_gpio, MXC_GPIO_INT_BOTH);
    MXC_GPIO_EnableInt(echo_flipper2_gpio.port, echo_flipper2_gpio.mask);
    NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(MXC_GPIO1)));
    flag_callback_funcs[FLIPPER_2] = flipper_callback;
    flag_callback_params[FLIPPER_2] = 2; 
}

void init_trigger_gpios()
{
    // cam
    triggercam_gpio.port = MXC_GPIO2;
    triggercam_gpio.mask = MXC_GPIO_PIN_4;
    triggercam_gpio.func = MXC_GPIO_FUNC_OUT;
    triggercam_gpio.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&triggercam_gpio);

    // flipper 0
    trigger0_gpio.port = MXC_GPIO1;
    trigger0_gpio.mask = MXC_GPIO_PIN_0;
    trigger0_gpio.func = MXC_GPIO_FUNC_OUT;
    trigger0_gpio.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&trigger0_gpio);

    // flipper 2
    trigger2_gpio.port = MXC_GPIO3;
    trigger2_gpio.mask = MXC_GPIO_PIN_1;
    trigger2_gpio.func = MXC_GPIO_FUNC_OUT;
    trigger2_gpio.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&trigger2_gpio);

    // flipper 1
    trigger1_gpio.port = MXC_GPIO2;
    trigger1_gpio.mask = MXC_GPIO_PIN_6;
    trigger1_gpio.func = MXC_GPIO_FUNC_OUT;
    trigger1_gpio.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&trigger1_gpio);
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

void activate_trigger0()
{
    trigger0_high();
    MXC_Delay(10);
    trigger0_low();
}

void activate_trigger1()
{
    trigger1_high();
    MXC_Delay(10);
    trigger1_low();
}

void activate_trigger2()
{
    trigger2_high();
    MXC_Delay(10);
    trigger2_low();
}

void activate_triggercam()
{
    triggercam_high();
    MXC_Delay(10);
    triggercam_low();
}

void triggercam_high()
{
    MXC_GPIO_OutSet(MXC_GPIO2, MXC_GPIO_PIN_4);
}

void triggercam_low()
{
    MXC_GPIO_OutClr(MXC_GPIO2, MXC_GPIO_PIN_4);
}

void trigger2_high()
{
    MXC_GPIO_OutSet(MXC_GPIO3, MXC_GPIO_PIN_1);
}

void trigger2_low()
{
    MXC_GPIO_OutClr(MXC_GPIO3, MXC_GPIO_PIN_1);
}

void trigger1_high()
{
    MXC_GPIO_OutSet(MXC_GPIO2, MXC_GPIO_PIN_6);
}

void trigger1_low()
{
    MXC_GPIO_OutClr(MXC_GPIO2, MXC_GPIO_PIN_6);
}

void trigger0_high()
{
    MXC_GPIO_OutSet(MXC_GPIO1, MXC_GPIO_PIN_0);
}

void trigger0_low()
{
    MXC_GPIO_OutClr(MXC_GPIO1, MXC_GPIO_PIN_0);
}