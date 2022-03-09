/***** Includes *****/

// GPIO
#include <stdio.h>
#include <string.h>
#include "mxc_device.h"
#include "mxc_delay.h"
#include "nvic_table.h"
#include "pb.h"
#include "board.h"
#include "gpio.h"
#include <stdint.h>
#include "mxc.h"
#include "mxc_sys.h"
#include "nvic_table.h"
#include "gcr_regs.h"

#include "tmr_funcs.h"
#include "ir_gpio_funcs.h"
#include "motor_funcs.h"


#include "cnn_helper_funcs.h"
#include "geffen_timer_funcs.h"

// sorter s = Sorter(5);
sorter scrappy;
//volatile int add_to_sorter = 0;
//volatile int pop_from_0 = 0;

int last_motor_interrupt_0 = 0;
int last_motor_interrupt_1 = 0;
int last_camera_interrupt = 0;
int systick_wait = 1000;
flag_callback flag_callbacks[num_flags];

/***** Functions *****/


// ======================= Interrupt Callbacks =====================

void camera_handler()
{
    static cnn_output_t output;

    // call camera take picture
    output = *run_cnn();

    show_cnn_output(output);

    int class_type = output.output_class;
    //printf("class type: %s\n", class_strings[class_type]);

    // add to queues w/ return val from classifier
    sorter__add_item(&scrappy, class_type);
}

void flipper_0_handler()
{
    if (sorter__detected_item(&scrappy, 0)) { // same motor address as IR sensor address
        target_tics(0, 30); 
        MXC_Delay(450000);
        target_tics(0, -11); 
    }

    //printf("queue size: %i\n",queue__size(&scrappy.queues[1]));
}

void flipper_1_handler()
{
    if (sorter__detected_item(&scrappy, 1)) { // same motor address as IR sensor address

        target_tics(1, 30); 
        MXC_Delay(450000);
        target_tics(1, -11); 
    }

    //printf("queue size: %i\n",queue__size(&scrappy.queues[1]));
}

// ======================= Interrupt Handlers =====================

void ir_camera_handler(void* cbdata) 
{
    //if (global_counter - last_camera_interrupt < systick_wait) return;
    
    set_flag(Camera);
    printf("1\n");

    //last_camera_interrupt = global_counter;

}


void ir_motor_handler_0(void* cbdata) 
{
    //if (global_counter - last_motor_interrupt_0 < systick_wait) return;
    
    set_flag(Flipper0);
    //printf("2\n");

    //last_motor_interrupt_0 = global_counter;
}

void ir_motor_handler_1(void* cbdata) 
{
    //if (global_counter - last_motor_interrupt_0 < systick_wait) return;
    
    set_flag(Flipper1);
    //printf("3\n");

    //last_motor_interrupt_0 = global_counter;
}

void ir_motor_handler_1(void* cbdata) 
{
    if (global_counter - last_motor_interrupt_1 < systick_wait) return;
    
    set_flag(Flipper1);
    //printf("2\n");

    last_motor_interrupt_1 = global_counter;
}

// set up interrupts
void gpio_init(void) {

    scrappy = Sorter(6,5);
    //sorter__add_item(&scrappy, 1);
    //sorter__add_item(&scrappy, 1);

    // CAMERA IR
    mxc_gpio_cfg_t ir_camera_interrupt;

    ir_camera_interrupt.port = IR_CAMERA_PORT;
    ir_camera_interrupt.mask = IR_CAMERA_PIN;
    ir_camera_interrupt.pad = MXC_GPIO_PAD_PULL_UP;
    ir_camera_interrupt.func = MXC_GPIO_FUNC_IN;
    ir_camera_interrupt.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&ir_camera_interrupt);
    MXC_GPIO_RegisterCallback(&ir_camera_interrupt, ir_camera_handler, &scrappy);
    MXC_GPIO_IntConfig(&ir_camera_interrupt, MXC_GPIO_INT_FALLING);
    MXC_GPIO_EnableInt(ir_camera_interrupt.port, ir_camera_interrupt.mask);
    NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(IR_CAMERA_PORT)));
    //printf("Camera IR Priority: %u\n", NVIC_GetPriority(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(IR_CAMERA_PORT))));
    flag_callbacks[Camera] = camera_handler;

    // MOTOR IR

    mxc_gpio_cfg_t flipper0_interrupt;

    flipper0_interrupt.port = IR_MOTOR_PORT_0;
    flipper0_interrupt.mask = IR_MOTOR_PIN_0;
    flipper0_interrupt.pad = MXC_GPIO_PAD_PULL_UP;
    flipper0_interrupt.func = MXC_GPIO_FUNC_IN;
    flipper0_interrupt.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&flipper0_interrupt);
    MXC_GPIO_RegisterCallback(&flipper0_interrupt, ir_motor_handler_0, &scrappy);
    MXC_GPIO_IntConfig(&flipper0_interrupt, MXC_GPIO_INT_FALLING);
    MXC_GPIO_EnableInt(flipper0_interrupt.port, flipper0_interrupt.mask);
    NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(IR_MOTOR_PORT_0)));
    //printf("Motor IR Priority: %u\n", NVIC_GetPriority(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(IR_MOTOR_PORT_0))));
    flag_callbacks[Flipper0] = flipper_0_handler;


    mxc_gpio_cfg_t flipper1_interrupt;

    flipper1_interrupt.port = MXC_GPIO2;
    flipper1_interrupt.mask = MXC_GPIO_PIN_7;
    flipper1_interrupt.pad = MXC_GPIO_PAD_PULL_UP;
    flipper1_interrupt.func = MXC_GPIO_FUNC_IN;
    flipper1_interrupt.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&flipper1_interrupt);
    MXC_GPIO_RegisterCallback(&flipper1_interrupt, ir_motor_handler_1, &scrappy);
    MXC_GPIO_IntConfig(&flipper1_interrupt, MXC_GPIO_INT_FALLING);
    MXC_GPIO_EnableInt(flipper1_interrupt.port, flipper1_interrupt.mask);
    NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(MXC_GPIO_PORT_2)));
    //printf("Motor IR Priority: %u\n", NVIC_GetPriority(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(IR_MOTOR_PORT_0))));
    flag_callbacks[Flipper1] = flipper_1_handler;
}

void check_all_callbacks()
{
    for (Flag f = 0; f < num_flags; f++) 
    {
        // if a flag is set, call its handler code, then reset the flag
        if (is_flag_set(f))
        {
            (*flag_callbacks[f])();
            unset_flag(f);
        }
    } 
}


// ============================ timer stuff =====================

// Global variables
mxc_tmr_cfg_t tmr;
volatile int timer_period = 0;
volatile int current_seconds_count = 0;

void expiration_handler(flag_callback cb_func)
{
    // Clear interrupt
    MXC_TMR_ClearFlags(MXC_TMR1);
    
    // increments a second
    current_seconds_count += 1;

    // check if expired
    if(current_seconds_count >= timer_period)
    {
        current_seconds_count = 0;
        stop_state_timer();
        set_flag(Flipper1);
    }
}

int init_state_timer(int expiration_period, flag_callback cb_func)
{
    // init timer variables
    timer_period = expiration_period;

    // setup the interrupt for timer 0
    NVIC_SetVector(TMR1_IRQn, expiration_handler);
    NVIC_EnableIRQ(TMR1_IRQn);

    // init timer 0 to interrupt every 1s (32KHz clock with prescaler 32 and count compare 1024)
    MXC_TMR_Shutdown(MXC_TMR1);
    tmr.pres = TMR_PRES_32;
    tmr.mode = TMR_MODE_CONTINUOUS;
    tmr.bitMode = TMR_BIT_MODE_32;
    tmr.clock = MXC_TMR_32K_CLK;
    tmr.cmp_cnt = 1024;
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
    flag_callbacks[Flipper1] = flipper_1_handler;
    return 0;
}

void set_expiration_period(int expiration_period)
{
    timer_period = expiration_period;
}

int get_expiration_period()
{
    return timer_period;
}

int get_state_time_left()
{
    return (timer_period - current_seconds_count);
}

void reset_state_timer()
{
    MXC_TMR_Stop(MXC_TMR1);
    current_seconds_count = 0;
    MXC_TMR1->cnt = 1; // this is the reset value for the timer count
}

void start_state_timer()
{
    MXC_TMR_Start(MXC_TMR1);
}

void stop_state_timer()
{
    MXC_TMR_Stop(MXC_TMR1);
}