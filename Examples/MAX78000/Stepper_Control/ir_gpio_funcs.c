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

// sorter s = Sorter(5);
sorter scrappy;
queue expirations;
//volatile int add_to_sorter = 0;
//volatile int pop_from_0 = 0;

int last_motor_interrupt_0 = 0;
int last_motor_interrupt_1 = 0;
int last_camera_interrupt = 0;
int systick_wait = 1000;
uint8_t curr_stepper_idx;
uint8_t next_stepper_idx;
flag_callback flag_callback_funcs[NUM_FLAGS];
uint8_t flag_callback_params[NUM_FLAGS] = {0};

int exp_times[] = {0,0,0,0,0};

bool is_first = true;

/***** Functions *****/


// ======================= Interrupt Callbacks =====================

void camera_handler()
{
    printf("Cam handler\n");
    static cnn_output_t output;

    // call camera take picture
    output = *run_cnn();

    show_cnn_output(output);

    int class_type = output.output_class;
    //printf("class type: %s\n", class_strings[class_type]);

    // add to queues w/ return val from classifier
    sorter__add_item(&scrappy, class_type);
}

// closes correpsonding arm
void close_handler()
{
    //printf("close_handler\n");
    // need to find current arm
    printf("Close Arm:%d\n --------------\n",curr_stepper_idx);
    target_tics(curr_stepper_idx, 0);
}

void flipper_callback(uint8_t flipperNum){
    // check if the item passing is this stepper's class
    if (sorter__detected_item(&scrappy, flipperNum)) { // same motor address as IR sensor address
        // open the arm
        target_tics(flipperNum, 30);
        printf("Open Arm:%d\n",flipperNum);

        // add this arm to the expiration queue with the expiration time (500ms delay)
        queue__push(&expirations, flipperNum);
        exp_times[flipperNum] = global_counter + 512;

        // something needs to start the expiration timer, only execute if this is the first item placed
        if(is_first)
        {
            printf("start tmr: %d\n", flipperNum);
            // clear flag
            is_first = false;
            
            // get the next deadline and set the expiration time
            int next_deadline = exp_times[flipperNum]; // do we need to reset this?
            MXC_TMR1->cnt = 512 - (next_deadline - global_counter);

            // start the next timer
            MXC_TMR_Start(MXC_TMR1);
        }

        // MXC_Delay(450000);
        // target_tics(0, -11); 
    }

    //printf("queue size: %i\n",queue__size(&scrappy.queues[1]));
}

// void flipper_0_handler()
// {
//     // check if the item passing is this stepper's class
//     if (sorter__detected_item(&scrappy, 0)) { // same motor address as IR sensor address
//         // open the arm
//         target_tics(0, 30);

//         // add this arm to the expiration queue with the expiration time (500ms delay)
//         queue__push(&expirations, 0);
//         exp_times[0] = global_counter + 512;

//         // something needs to start the expiration timer, only execute if this is the first item placed
//         if(is_first)
//         {
//             printf("startup: 0\n");
//             // clear flag
//             is_first = false;
            
//             // get the next deadline and set the expiration time
//             int next_deadline = exp_times[0]; // do we need to reset this?
//             MXC_TMR1->cnt = 512 - (next_deadline - global_counter);

//             // start the next timer
//             MXC_TMR_Start(MXC_TMR1);
//         }

//         // MXC_Delay(450000);
//         // target_tics(0, -11); 
//     }

//     //printf("queue size: %i\n",queue__size(&scrappy.queues[1]));
// }

// void flipper_1_handler()
// {
//     if (sorter__detected_item(&scrappy, 1)) { // same motor address as IR sensor address
//         target_tics(1, 30); 
//         queue__push(&expirations, 1);
//         exp_times[1] = global_counter + 512;

//         // something needs to start the expiration timer, only execute if this is the first item placed
//         if(is_first)
//         {
//             printf("startup: 1\n");
//             // clear flag
//             is_first = false;
            
//             // get the next deadline and set the expiration time
//             int next_deadline = exp_times[1]; // do we need to reset this?
//             MXC_TMR1->cnt = 512 - (next_deadline - global_counter);

//             // start the next timer
//             MXC_TMR_Start(MXC_TMR1);
//         }

//         // MXC_Delay(450000);
//         // target_tics(1, -17); 
//     }

//     //printf("queue size: %i\n",queue__size(&scrappy.queues[1]));
// }

// void flipper_2_handler()
// {
//     if (sorter__detected_item(&scrappy, 2)) { // same motor address as IR sensor address
//         target_tics(2, 30); 
//         queue__push(&expirations, 2);
//         exp_times[2] = global_counter + 512;

//         // something needs to start the expiration timer, only execute if this is the first item placed
//         if(is_first)
//         {
//             printf("startup: 2\n");
//             // clear flag
//             is_first = false;
            
//             // get the next deadline and set the expiration time
//             int next_deadline = exp_times[2]; // do we need to reset this?
//             MXC_TMR1->cnt = 512 - (next_deadline - global_counter);

//             // start the next timer
//             MXC_TMR_Start(MXC_TMR1);
//         }

//         // MXC_Delay(450000);
//         // target_tics(2, -17); 
//     }

//     //printf("queue size: %i\n",queue__size(&scrappy.queues[1]));
// }

// ======================= Interrupt Handlers =====================

void ir_camera_handler(void* cbdata) 
{
    //if (global_counter - last_camera_interrupt < systick_wait) return;
    
    set_flag(CAMERA);
    //printf("cam\n");

    //last_camera_interrupt = global_counter;

}


void ir_motor_handler_0(void* cbdata) 
{
    //if (global_counter - last_motor_interrupt_0 < systick_wait) return;
    
    set_flag(FLIPPER_0);
    //printf("f0\n");

    //last_motor_interrupt_0 = global_counter;
}

void ir_motor_handler_1(void* cbdata) 
{
    //if (global_counter - last_motor_interrupt_0 < systick_wait) return;
    
    set_flag(FLIPPER_1);
    //printf("f1\n");

    //last_motor_interrupt_0 = global_counter;
}

void ir_motor_handler_2(void* cbdata) 
{
    //if (global_counter - last_motor_interrupt_0 < systick_wait) return;
    
    set_flag(FLIPPER_2);
    //printf("f2\n");

    //last_motor_interrupt_0 = global_counter;
}

// set up interrupts
void gpio_init(void) {

    // sorting queues
    scrappy = Sorter(7,7);

    // timer expiration queue for closing arm
    expirations = Queue(10);

    // initialize the timer
    init_arm_timer();

    // callback for closing the arm
    flag_callback_funcs[CLOSE] = close_handler;
    
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
    flag_callback_funcs[CAMERA] = camera_handler;

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
    
    
    flag_callback_funcs[FLIPPER_0] = flipper_callback;
    flag_callback_params[FLIPPER_0] = 0; 


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
    NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(MXC_GPIO2)));
    //printf("Motor IR Priority: %u\n", NVIC_GetPriority(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(IR_MOTOR_PORT_0))));
    flag_callback_funcs[FLIPPER_1] = flipper_callback;
    flag_callback_params[FLIPPER_1] = 1; 


    mxc_gpio_cfg_t flipper2_interrupt;

    flipper2_interrupt.port = MXC_GPIO1;
    flipper2_interrupt.mask = MXC_GPIO_PIN_1;
    flipper2_interrupt.pad = MXC_GPIO_PAD_PULL_UP;
    flipper2_interrupt.func = MXC_GPIO_FUNC_IN;
    flipper2_interrupt.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&flipper2_interrupt);
    MXC_GPIO_RegisterCallback(&flipper2_interrupt, ir_motor_handler_2, &scrappy);
    MXC_GPIO_IntConfig(&flipper2_interrupt, MXC_GPIO_INT_FALLING);
    MXC_GPIO_EnableInt(flipper2_interrupt.port, flipper2_interrupt.mask);
    NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(MXC_GPIO1)));
    //printf("Motor IR Priority: %u\n", NVIC_GetPriority(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(IR_MOTOR_PORT_0))));
    flag_callback_funcs[FLIPPER_2] = flipper_callback;
    flag_callback_params[FLIPPER_2] = 2; 
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
        int next_deadline = exp_times[next_stepper];
        printf("next tmr start: %i, exp in %ims\n",next_stepper, next_deadline - global_counter);
        printf("next deadline: %i, global cntr: %i\n", next_deadline, global_counter);

        // set the next deadline
        MXC_TMR1->cnt = 512 - (next_deadline - global_counter);

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
    tmr.cmp_cnt = 512; //expiration_period*1024/1000; // approximation, can only get exact for multiples of 2
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