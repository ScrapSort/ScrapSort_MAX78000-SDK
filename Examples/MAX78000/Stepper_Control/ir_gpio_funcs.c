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

// sorter s = Sorter(5);
sorter s;

int motor_ir_index[] = {0, 1, 2, 3, 4};

/***** Functions *****/

void ir_camera_handler(void* cbdata) {
    printf("\nir camera interrupt\n");

    sorter* s_ptr = cbdata;

    // call camera take picture

    int class_type = 2; // ex

    // add to queues w/ return val from classifier
    sorter__add_item(s_ptr, class_type);
}



void ir_motor_handler(void* cbdata) {
    printf("\nir motor interrupt\n");

    sorter* s_ptr = cbdata;

    // mxc_gpio_cfg_t* cfg = cbdata;
    // MXC_GPIO_OutToggle(cfg->port, cfg->mask);
    
    // if (pause_ir_interrupts) return;

    if (sorter__detected_item(s_ptr, 2)) { // same motor address as IR sensor address
        target_tics(2, -40); 
    }
    

    // NVIC_SetVector(TMR5_IRQn, OneshotTimerHandler);
    // NVIC_EnableIRQ(TMR5_IRQn);
    
    // OneshotTimer();
}

void gpio_init(void) {

    s = Sorter(5);

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


    // MOTOR IR

    mxc_gpio_cfg_t gpio_interrupt;

    gpio_interrupt.port = IR_MOTOR_PORT_0;
    gpio_interrupt.mask = IR_MOTOR_PIN_0;
    gpio_interrupt.pad = MXC_GPIO_PAD_PULL_UP;
    gpio_interrupt.func = MXC_GPIO_FUNC_IN;
    gpio_interrupt.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&gpio_interrupt);
    MXC_GPIO_RegisterCallback(&gpio_interrupt, ir_motor_handler, &s);
    MXC_GPIO_IntConfig(&gpio_interrupt, MXC_GPIO_INT_FALLING);
    MXC_GPIO_EnableInt(gpio_interrupt.port, gpio_interrupt.mask);
    NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(IR_MOTOR_PORT_0)));
    
}