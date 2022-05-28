/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include "mxc_device.h"
#include "nvic_table.h"
#include "board.h"
#include "mxc_sys.h"
#include "wdt.h"
#include "mxc_delay.h"


#define WATCHDOG_RESET_PERIOD MXC_WDT_PERIOD_2_28
#define WATCHDOG_INTERRUPT_PERIOD MXC_WDT_PERIOD_2_27

void MXC_WDT_Setup(void);
void WDT0_IRQHandler(void);
void watchdogHandler(void);
void pat_the_dog(void);
void assess_prev_reset(void);

