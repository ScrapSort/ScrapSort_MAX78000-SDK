#ifndef IR_GPIO_FUNCS_H
#define IR_GPIO_FUNCS_H

#include "sorter.h"
#include "flags.h"

extern sorter scrappy;

/***** Definitions *****/

// CAMERA IR
#define IR_CAMERA_PORT      MXC_GPIO1
#define IR_CAMERA_PIN       MXC_GPIO_PIN_6


// MOTOR IR 
#define IR_MOTOR_PORT_0      MXC_GPIO2
#define IR_MOTOR_PIN_0       MXC_GPIO_PIN_3



#define MXC_GPIO_PORT_INTERRUPT_STATUS  MXC_GPIO2
#define MXC_GPIO_PIN_INTERRUPT_STATUS   MXC_GPIO_PIN_1

void gpio_init(void);
void ir_motor_handler_0(void* cbdata);
void ir_motor_handler_1(void* cbdata);
void ir_camera_handler(void* cbdata);

// these functions get called by main when
// the corresponding interrupt flag has been set
void camera_handler();
void flipper_0_handler();
void flipper_1_handler();

// checks all interrupt flags
void check_all_callbacks();

#endif