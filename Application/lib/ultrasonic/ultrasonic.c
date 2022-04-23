#include "ultrasonic.h"
#include "motor_funcs.h"
#include "sorter.h"
#include "flags.h"
#include "motor_funcs.h"

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
#define CLOSE_THRESH 2
#define FAR_THRESH 5

typedef enum
{
    CAM = 0,
    FLIPPER0,
    FLIPPER1,
    FLIPPER2
} ultrasonic_t;

ultrasonic_t camera_idx = CAM;
ultrasonic_t flipper0_idx = FLIPPER0;
ultrasonic_t flipper1_idx = FLIPPER1;
ultrasonic_t flipper2_idx = FLIPPER2;

// gpios for trigger and echos
mxc_gpio_cfg_t trigger_gpio;
mxc_gpio_cfg_t echo_cam_gpio;
mxc_gpio_cfg_t echo_flipper0_gpio;
mxc_gpio_cfg_t echo_flipper1_gpio;
mxc_gpio_cfg_t echo_flipper2_gpio;

// state variables for echo pulse
uint16_t volatile current_pulse_values[] = {0,0,0,0};
uint16_t volatile time_intervals[] = {10,10,10,10};
uint16_t volatile object_statuses[] = {0,0,0,0};

uint8_t volatile triggers[] = {0,0,0,0};
// sorter sorting_queues;
// volatile queue expirations;
//volatile int add_to_sorter = 0;
//volatile int pop_from_0 = 0;

// int last_motor_interrupt_0 = 0;
// int last_motor_interrupt_1 = 0;
// int last_motor_interrupt_2 = 0;
// int last_camera_interrupt = 0;
// int systick_wait = 1000;
// volatile uint8_t curr_stepper_idx;
// volatile uint8_t next_stepper_idx;
// flag_callback flag_callback_funcs[NUM_FLAGS];
// uint8_t flag_callback_params[NUM_FLAGS] = {0};

// volatile int exp_times[] = {0,0,0,0,0};

// bool is_first = true;


// void camera_callback()
// {
//     //printf("Cam handler\n");
//     //static cnn_output_t output;

//     // call camera take picture
//     //output = *run_cnn();

//     //show_cnn_output(output);

//     //int class_type = output.output_class;
//     //printf("class type: %s\n", class_strings[class_type]);

//     // add to queues w/ return val from classifier
//     //sorter__add_item(&scrappy, class_type);
// }

// // closes correpsonding arm
// void close_arm_callback()
// {
//     printf("close_handler\n");
//     //set to high torque mode
//     //set_motor_profile(curr_stepper_idx, MOTOR_PROFILE_TORQUE);

//     //set to home
//     target_tics(curr_stepper_idx, 0);
    
// }

// void flipper_callback(uint8_t flipperNum){
//     //printf("cb: %i\n",flipperNum);
//     // check if the item passing is this stepper's class
//     if (sorter__detected_item(&sorting_queues, flipperNum)) { // same motor address as IR sensor address
//         //set to high speed profile
//         printf("Open Arm:%d\n",flipperNum);
//         //set_motor_profile(flipperNum, MOTOR_PROFILE_SPEED);

//         // open the arm
//         target_tics(flipperNum, -30);

//         // add this arm to the expiration queue with the expiration time (500ms delay)
//         queue__push(&expirations, flipperNum);
//         exp_times[flipperNum] = global_counter + 1024;
//         printf("exp time added: %i\n", exp_times[flipperNum]);

//         // something needs to start the expiration timer, only execute if this is the first item placed
//         if(is_first)
//         {
//             printf("start tmr: %d\n", flipperNum);
//             // clear flag
//             is_first = false;
            
//             // get the next deadline and set the expiration time
//             int next_deadline = exp_times[flipperNum]; // do we need to reset this?
//             MXC_TMR1->cnt = 1024 - (next_deadline - global_counter);

//             // start the next timer
//             MXC_TMR_Start(MXC_TMR1);
//         }

//         // MXC_Delay(450000);
//         // target_tics(0, -11); 
//     }

//     //printf("queue size: %i\n",queue__size(&scrappy.queues[1]));
// }

void echo_handler(void* cb_data)
{
    // get the sensor idx from the callback data
    ultrasonic_t sensor_idx = *(ultrasonic_t*)(cb_data);

    // first interrupt (rising edge)
    if(!current_pulse_values[sensor_idx])
    {
        // store the start time
        current_pulse_values[sensor_idx] = global_counter;
    }
    // second interrupt (falling edge)
    else
    {
        // find the pulse length in ms, reset the current value
        time_intervals[sensor_idx] = global_counter - current_pulse_values[sensor_idx];
        //printf("dist:%d, cp: %d, gc:%d\n",time_intervals[sensor_idx],current_pulse_values[sensor_idx],global_counter);
        current_pulse_values[sensor_idx] = 0;
    }

    // interval less than 10ms means object detected (don't want repeated detection)
    if(!object_statuses[sensor_idx] && time_intervals[sensor_idx] < CLOSE_THRESH)
    {
        // there is an object in front of the sensor
        object_statuses[sensor_idx] = 1;
        triggers[sensor_idx] = 1;
        printf("object %d present\n",sensor_idx);
    }
    else if(object_statuses[sensor_idx] && time_intervals[sensor_idx] >= FAR_THRESH)
    {
        // reset the state
        object_statuses[sensor_idx] = 0;
        printf("object %d left\n", sensor_idx);
    }
}

void triggered()
{
    // check if any arm has been triggered
    for(int i = 0; i < 3; i++)
    {
        if(triggers[i] == 1)
        {
            // do the arm movement test
            go_home_reverse(i);
            MXC_Delay(SEC(1));
            go_home_forward(i);

            // reset the trigger
            triggers[i] = 0;
        }
    }
}

void init_ultrasonic_gpios()
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
}

void init_trigger()
{
    // trigger.port = MXC_GPIO2;
    // trigger.mask = MXC_GPIO_PIN_4;
    // trigger.func = MXC_GPIO_FUNC_OUT;
    // trigger.vssel = MXC_GPIO_VSSEL_VDDIOH;
    // MXC_GPIO_Config(&trigger);
    

    mxc_tmr_cfg_t tmr;   
    
    MXC_TMR_Shutdown(PWM_TIMER);
    
    tmr.pres = TMR_PRES_8; // 1 MHz --> 8MHz/8
    tmr.mode = TMR_MODE_PWM;
    tmr.bitMode = TMR_BIT_MODE_32;    
    tmr.clock = MXC_TMR_8M_CLK; // 8MHz
    tmr.cmp_cnt = 50000; // 50ms period
    tmr.pol = 1;

    int err;
    if ((err = MXC_TMR_Init(PWM_TIMER, &tmr, true)) != E_NO_ERROR) {
        printf("Failed PWM timer Initialization: %d\n", err);
        return;
    }

    //MXC_GPIO2->vssel |= MXC_GPIO_VSSEL_VDDIOH;

    // trigger for 10 us
    if ((err = MXC_TMR_SetPWM(PWM_TIMER, 10)) != E_NO_ERROR) {
        printf("Failed TMR_PWMConfig: %d\n", err);
        return;
    }
    
    MXC_TMR_Start(PWM_TIMER);
    
    printf("PWM started.\n\n");
}

void trigger_high()
{
    MXC_GPIO_OutSet(MXC_GPIO2, MXC_GPIO_PIN_4);
}

void trigger_low()
{
    MXC_GPIO_OutClr(MXC_GPIO2, MXC_GPIO_PIN_4);
}
