/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include "mxc_device.h"
#include "nvic_table.h"
#include "board.h"
#include "mxc_sys.h"
#include "wdt.h"
#include "mxc_delay.h"
#include "pb.h"
#include "led.h"


/***** Globals *****/

#define NORMAL_OPERATION_STATE 1
#define STALL_OPERATION_STATE 0

volatile int operation_state = NORMAL_OPERATION_STATE;
static mxc_wdt_cfg_t cfg;

#define WATCHDOG_RESET_PERIOD MXC_WDT_PERIOD_2_30
#define WATCHDOG_INTERRUPT_PERIOD MXC_WDT_PERIOD_2_29

/***** Functions *****/

void watchdogHandler()
{
    MXC_WDT_ClearIntFlag(MXC_WDT0);
    printf("\nWatchdog has tripped!\n");

    // printf("### Debug Info ###\n");
    
    // printf("Attempting to rectify the situation with software\n");

    // RESET I2C

    // RESET GPIO

    // RESET TIMERS
    
}

// *****************************************************************************

void WDT0_IRQHandler(void)
{
    watchdogHandler();
}

// *****************************************************************************

void MXC_WDT_Setup()
{
    printf("\nConfiguring WDT . . .\n");

    MXC_WDT_Disable(MXC_WDT0);
    MXC_WDT_ResetTimer(MXC_WDT0);

    cfg.upperResetPeriod = WATCHDOG_RESET_PERIOD;
    MXC_WDT_SetResetPeriod(MXC_WDT0, &cfg);
    MXC_WDT_EnableReset(MXC_WDT0);
    printf("Reset period: 2^%d tics = about %.2f sec\n", 31 - WATCHDOG_RESET_PERIOD, 70.0 / (1UL << WATCHDOG_RESET_PERIOD ));

    cfg.upperIntPeriod = WATCHDOG_INTERRUPT_PERIOD;
    MXC_WDT_SetIntPeriod(MXC_WDT0, &cfg);
    MXC_WDT_EnableInt(MXC_WDT0);
    printf("Interrupt period: 2^%d tics = about %.2f sec\n", 31 - WATCHDOG_INTERRUPT_PERIOD, 70.0 / (1UL << WATCHDOG_INTERRUPT_PERIOD ));

    NVIC_SetVector(WDT0_IRQn, WDT0_IRQHandler);
    NVIC_EnableIRQ(WDT0_IRQn);

    MXC_WDT_Enable(MXC_WDT0);
    printf(". . . Setup complete!\n\n");
}

// *****************************************************************************

void SW1_Callback()
{
    operation_state = 1 - operation_state;
}

// *****************************************************************************

int main () {
    printf("\n************** basic bitch wdt test ***************\n");

    cfg.mode = MXC_WDT_COMPATIBILITY;
    MXC_WDT_Init(MXC_WDT0, &cfg);

    // DISPLAY STATUS OF PREVIOUS RESET
    uint32_t resetFlags = MXC_WDT_GetResetFlag(MXC_WDT0);
    printf("\nPrevious reset caused by: ");

    if (resetFlags) {
        printf("WATCHDOG.\n");
    } else {
        printf("NOT WATCHDOG.\n");
    }
    MXC_WDT_ClearResetFlag(MXC_WDT0);
    MXC_WDT_ClearIntFlag(MXC_WDT0);

    
    // CONFIGURE WDT
    MXC_WDT_Setup();


    // Configure push button
    PB_RegisterCallback(0, SW1_Callback);
    PB_IntEnable(0);


    // SIMULATE OPERATION
    while (1) {

        // SIMULATE NORMAL OPERATION
        if (operation_state == NORMAL_OPERATION_STATE)
            printf("Starting inf loop - NORMAL OPERATION...\n");
        while (operation_state == NORMAL_OPERATION_STATE) {
            MXC_Delay(MXC_DELAY_MSEC(500));
            LED_On(0);
            MXC_Delay(MXC_DELAY_MSEC(500));
            LED_Off(0);
            MXC_WDT_ResetTimer(MXC_WDT0);
        }

        // SIMULATE STALL
        if (operation_state == STALL_OPERATION_STATE)
            printf("Starting inf loop - STALL...\n");
        while (operation_state == STALL_OPERATION_STATE);

    }

    return 0;
}