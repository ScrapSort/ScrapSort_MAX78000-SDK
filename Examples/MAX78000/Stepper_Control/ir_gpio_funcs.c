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

#include "sorter.h"

#include "cnn_helper_funcs.h"

// sorter s = Sorter(5);
sorter s;

int last_motor_interrupt_0 = 0;
int last_camera_interrupt = 0;
int systick_wait = 1500;

/***** Functions *****/

void ir_camera_handler(void* cbdata) {
    static cnn_output_t output;
    if (global_counter - last_camera_interrupt < systick_wait) return;
    // if (pause_camera_interrupts) return;

    printf("\nir camera interrupt: adding to queue\n");

    sorter* s_ptr = cbdata;

    // call camera take picture
    output = *run_cnn();
    show_cnn_output(output);

    int class_type = output.output_class;

    // add to queues w/ return val from classifier
    sorter__add_item(s_ptr, class_type);
    // printf("here\n");
    // sorter__print(s_ptr);

    // NVIC_SetVector(TMR5_IRQn, OneshotTimerHandler);
    // NVIC_EnableIRQ(TMR5_IRQn);
    
    // OneshotTimer();
    last_camera_interrupt = global_counter;

}


void ir_motor_handler_0(void* cbdata) {
    if (global_counter - last_motor_interrupt_0 < systick_wait) return;
    // if (pause_motor_interrupts_0) return;

    // pause_motor_interrupts_0 = 1;

    printf("\nir motor interrupt: %d\n", global_counter);

    sorter* s_ptr = cbdata;

    printf("Queue status BEFORE pop\n");
    // sorter__print(s_ptr);
    queue__print(&(s_ptr->queues[1]));

    // mxc_gpio_cfg_t* cfg = cbdata;
    // MXC_GPIO_OutToggle(MXC_GPIO2, MXC_GPIO_PIN_1);
    
    // if (pause_motor_interrupts) return;

    if (sorter__detected_item(s_ptr, 1)) { // same motor address as IR sensor address
        target_tics(0, -40); 
    }

    printf("Queue status AFTER pop\n");
    // sorter__print(s_ptr);
    queue__print(&(s_ptr->queues[1]));


    // NVIC_SetVector(TMR5_IRQn, OneshotTimerHandler);
    // NVIC_EnableIRQ(TMR5_IRQn);
    
    // OneshotTimer();
    // pause_motor_interrupts_0 = 0;

    last_motor_interrupt_0 = global_counter;
}

void gpio_init(void) {

    s = Sorter(5);
    //sorter__add_item(&s, 1);
    //sorter__add_item(&s, 1);

    // CAMERA IR
    mxc_gpio_cfg_t ir_camera_interrupt;

    ir_camera_interrupt.port = IR_CAMERA_PORT;
    ir_camera_interrupt.mask = IR_CAMERA_PIN;
    ir_camera_interrupt.pad = MXC_GPIO_PAD_PULL_UP;
    ir_camera_interrupt.func = MXC_GPIO_FUNC_IN;
    ir_camera_interrupt.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&ir_camera_interrupt);
    MXC_GPIO_RegisterCallback(&ir_camera_interrupt, ir_camera_handler, &s);
    MXC_GPIO_IntConfig(&ir_camera_interrupt, MXC_GPIO_INT_FALLING);
    MXC_GPIO_EnableInt(ir_camera_interrupt.port, ir_camera_interrupt.mask);
    NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(IR_CAMERA_PORT)));
    printf("Camera IR Priority: %u\n", NVIC_GetPriority(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(IR_CAMERA_PORT))));


    // MOTOR IR

    mxc_gpio_cfg_t gpio_interrupt;

    gpio_interrupt.port = IR_MOTOR_PORT_0;
    gpio_interrupt.mask = IR_MOTOR_PIN_0;
    gpio_interrupt.pad = MXC_GPIO_PAD_PULL_UP;
    gpio_interrupt.func = MXC_GPIO_FUNC_IN;
    gpio_interrupt.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&gpio_interrupt);
    MXC_GPIO_RegisterCallback(&gpio_interrupt, ir_motor_handler_0, &s);
    MXC_GPIO_IntConfig(&gpio_interrupt, MXC_GPIO_INT_FALLING);
    MXC_GPIO_EnableInt(gpio_interrupt.port, gpio_interrupt.mask);
    NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(IR_MOTOR_PORT_0)));
    printf("Motor IR Priority: %u\n", NVIC_GetPriority(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(IR_MOTOR_PORT_0))));
    
}