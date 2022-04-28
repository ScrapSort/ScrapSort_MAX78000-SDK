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
#define FAR_THRESH 20 // cm

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
uint8_t volatile trigger_state[] = {0,0,0,0}; // state variable to track if a sensor needs to fire

sorter sorting_queues;
volatile queue expirations;

volatile uint8_t curr_stepper_idx;
volatile uint8_t next_stepper_idx;

flag_callback flag_callback_funcs[NUM_FLAGS];
uint8_t flag_callback_params[NUM_FLAGS] = {0};

volatile int exp_times[] = {0,0,0,0,0};

bool is_first = true;


void camera_callback()
{
    printf("Cam handler\n");
    static cnn_output_t output;

    // call camera take picture
    output = *run_cnn();

    show_cnn_output(output);

    int class_type = output.output_class;
    printf("class type: %s\n", class_strings[class_type]);

    // add to queues w/ return val from classifier
    sorter__add_item(&sorting_queues, class_type);
}

// closes correpsonding arm
void close_arm_callback()
{
    printf("close_handler\n");
    //set to high torque mode
    //set_motor_profile(curr_stepper_idx, MOTOR_PROFILE_TORQUE);

    //set to home
    go_home_forward(curr_stepper_idx);
    //target_tics(curr_stepper_idx, 0);
    
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
            set_flag(sensor_idx); // will trigger arm to close in main
            // printf("object %d present\n",sensor_idx);
            printf("S2: %d\n",object_statuses[2]);
            printf("S1: %d\n",object_statuses[1]);
            printf("S0: %d\n",object_statuses[0]);
            printf("S3: %d\n",object_statuses[3]);
            printf("\033[0;0f");
        }
        // object in front of the sensor and beyond the threshold, update the state
        else if(object_statuses[sensor_idx] && time_intervals[sensor_idx] >= FAR_THRESH)
        {
            // reset the state
            object_statuses[sensor_idx] = 0;
            // printf("object %d left\n", sensor_idx);
            printf("S2: %d\n",object_statuses[2]);
            printf("S1: %d\n",object_statuses[1]);
            printf("S0: %d\n",object_statuses[0]);
            printf("S3: %d\n",object_statuses[3]);
            printf("\033[0;0f");
        }

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
    // check if the item that passed is this flipper's item
    if (sorter__detected_item(&sorting_queues, flipper_num)) { // same motor address as IR sensor address
        //set to high speed profile
        printf("Open Arm:%d\n",flipper_num);
        //set_motor_profile(flipper_num, MOTOR_PROFILE_SPEED);

        // open the arm
        target_tics(flipper_num, -40);

        // add this arm to the expiration queue with the expiration time (500ms delay)
        queue__push(&expirations, flipper_num);
        exp_times[flipper_num] = global_counter + 1024; // about 1 second
        printf("exp time added: %i\n", exp_times[flipper_num]);

        // something needs to start the expiration timer, only execute if this is the first item placed
        if(is_first)
        {
            printf("start tmr: %d\n", flipper_num);
            // clear flag
            is_first = false;
            
            // get the next deadline and set the expiration time
            int next_deadline = exp_times[flipper_num]; // do we need to reset this?
            MXC_TMR1->cnt = 1024 - (next_deadline - global_counter);

            // start the next timer
            MXC_TMR_Start(MXC_TMR1);
        }
    }
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
            activatecam();
            trigger_state[CAMERA] = 0;
            break;
        }

        case FLIPPER_0:
        {
            activate0();
            trigger_state[FLIPPER_0] = 0;
            break;
        }

        case FLIPPER_1:
        {
            activate1();
            trigger_state[FLIPPER_1] = 0;
            break;
        }

        case FLIPPER_2:
        {
            activate2();
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
    // sorting queues
    sorting_queues = Sorter(7,7);

    // timer expiration queue for closing arm
    expirations = Queue(10);

    // initialize the timer
    init_arm_timer();

    // callback for closing the arm
    flag_callback_funcs[CLOSE] = close_arm_callback;

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

void activate0()
{
    trigger0_high();
    MXC_Delay(10);
    trigger0_low();
}

void activate1()
{
    trigger1_high();
    MXC_Delay(10);
    trigger1_low();
}

void activate2()
{
    trigger2_high();
    MXC_Delay(10);
    trigger2_low();
}

void activatecam()
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


// ============================ timer stuff =====================

// Global variables
mxc_tmr_cfg_t tmr;
volatile int timer_period = 0;
volatile int current_periods_count = 0;


// timer expired
void expiration_handler()
{
    // Clear interrupt, stop timer
    MXC_TMR_ClearFlags(MXC_TMR1);
    MXC_TMR_Stop(MXC_TMR1);
    
    // get next item on the queue, says which stepper needs to close
    curr_stepper_idx = queue__pop(&expirations);
    printf("tmr exp: %i\n", curr_stepper_idx);
    //printf("curr:%i\n",curr_stepper_idx);

    // set up the next timer interrupt by looking at the next item on the queue
    int next_stepper = queue__peak(&expirations);

    // if there is no next item, we need to reset
    if(next_stepper == -1)
    {
        printf("q empty, rst\n");
        is_first = true;
    }
    else // there is a next item waiting
    {
        uint32_t next_deadline = exp_times[next_stepper];
        printf("next tmr start: %i, exp in %ims\n",next_stepper, next_deadline - global_counter);
        printf("next deadline: %i, global cntr: %i\n", next_deadline, global_counter);

        // set the next deadline
        MXC_TMR1->cnt = 1024 - (next_deadline - global_counter);

        // start the next timer
        MXC_TMR_Start(MXC_TMR1);
    }
    // close the current arm
    set_flag(CLOSE); 
}

int init_arm_timer()
{
    // setup the interrupt for timer 0
    NVIC_SetVector(TMR1_IRQn, expiration_handler);
    NVIC_EnableIRQ(TMR1_IRQn);

    // init timer 0 to interrupt every expiration period 500 ms (32KHz clock with prescaler 32 and count compare 1024)
    MXC_TMR_Shutdown(MXC_TMR1);
    tmr.pres = TMR_PRES_32; // counts every 1/1024 seconds
    tmr.mode = TMR_MODE_CONTINUOUS;
    tmr.bitMode = TMR_BIT_MODE_32;
    tmr.clock = MXC_TMR_32K_CLK;
    tmr.cmp_cnt = 1024; //expiration_period*1024/1000; // approximation, can only get exact for multiples of 2
    tmr.pol = 0;
    
    // init the timer
    if (MXC_TMR_Init(MXC_TMR1, &tmr, true) != E_NO_ERROR) 
    {
        printf("Failed one-shot timer Initialization.\n");
        return -1;
    }
    
    // enable the interrupt
    MXC_TMR_EnableInt(MXC_TMR1);

    printf("State timer initialized.\n\n");
    return 0;
}