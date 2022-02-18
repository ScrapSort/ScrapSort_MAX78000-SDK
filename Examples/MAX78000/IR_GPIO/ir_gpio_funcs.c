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

/***** Functions *****/

void gpio_isr(void* cbdata)
{
    mxc_gpio_cfg_t* cfg = cbdata;
    MXC_GPIO_OutToggle(cfg->port, cfg->mask);
    
    if (pause_ir_interrupts) return;

    printf("interrupt\n");

    NVIC_SetVector(TMR5_IRQn, OneshotTimerHandler);
    NVIC_EnableIRQ(TMR5_IRQn);
    
    OneshotTimer();
}
