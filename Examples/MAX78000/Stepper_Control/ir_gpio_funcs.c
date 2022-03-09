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

#include "tmr_funcs.h"
#include "ir_gpio_funcs.h"
#include "motor_funcs.h"


#include "cnn_helper_funcs.h"

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
    //printf("4\n");
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
        // target_tics(1, -30); 
        rotate_revs(1, 0.15);
        MXC_Delay(450000);
        // target_tics(1, 60); 
        rotate_revs(1, -0.6);
    }

    //printf("queue size: %i\n",queue__size(&scrappy.queues[1]));
}

// ======================= Interrupt Handlers =====================

void ir_camera_handler(void* cbdata) 
{
    if (global_counter - last_camera_interrupt < systick_wait) return;
    
    set_flag(Camera);
    //printf("1\n");

    last_camera_interrupt = global_counter;

}


void ir_motor_handler_0(void* cbdata) 
{
    if (global_counter - last_motor_interrupt_0 < systick_wait) return;
    
    set_flag(Flipper0);
    //printf("2\n");

    last_motor_interrupt_0 = global_counter;
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

    // 0
    mxc_gpio_cfg_t gpio_interrupt_0;

    gpio_interrupt_0.port = IR_MOTOR_PORT_0;
    gpio_interrupt_0.mask = IR_MOTOR_PIN_0;
    gpio_interrupt_0.pad = MXC_GPIO_PAD_PULL_UP;
    gpio_interrupt_0.func = MXC_GPIO_FUNC_IN;
    gpio_interrupt_0.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&gpio_interrupt_0);
    MXC_GPIO_RegisterCallback(&gpio_interrupt_0, ir_motor_handler_0, &scrappy);
    MXC_GPIO_IntConfig(&gpio_interrupt_0, MXC_GPIO_INT_FALLING);
    MXC_GPIO_EnableInt(gpio_interrupt_0.port, gpio_interrupt_0.mask);
    NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(IR_MOTOR_PORT_0)));
    //printf("Motor IR Priority: %u\n", NVIC_GetPriority(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(IR_MOTOR_PORT_0))));
    flag_callbacks[Flipper0] = flipper_0_handler;


    // 1
    mxc_gpio_cfg_t gpio_interrupt_1;

    gpio_interrupt_1.port = IR_MOTOR_PORT_1;
    gpio_interrupt_1.mask = IR_MOTOR_PIN_1;
    gpio_interrupt_1.pad = MXC_GPIO_PAD_PULL_UP;
    gpio_interrupt_1.func = MXC_GPIO_FUNC_IN;
    gpio_interrupt_1.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&gpio_interrupt_1);
    MXC_GPIO_RegisterCallback(&gpio_interrupt_1, ir_motor_handler_1, &scrappy);
    MXC_GPIO_IntConfig(&gpio_interrupt_1, MXC_GPIO_INT_FALLING);
    MXC_GPIO_EnableInt(gpio_interrupt_1.port, gpio_interrupt_1.mask);
    NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(IR_MOTOR_PORT_1)));
    //printf("Motor IR Priority: %u\n", NVIC_GetPriority(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(IR_MOTOR_PORT_1))));
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