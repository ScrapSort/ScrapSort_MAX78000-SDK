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

#include "watchdog.h"


/***** Globals *****/

#define NORMAL_OPERATION_STATE 1
#define STALL_OPERATION_STATE 0

volatile int operation_state = NORMAL_OPERATION_STATE;

/***** Functions *****/

void SW1_Callback()
{
    operation_state = 1 - operation_state;
}

// *****************************************************************************

int main () {
    printf("\n************** basic bitch wdt test ***************\n");

    // DISPLAY STATUS OF PREVIOUS RESET
    assess_prev_reset();


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
            // MXC_WDT_ResetTimer(MXC_WDT0);
            pat_the_dog();
        }

        // SIMULATE STALL
        if (operation_state == STALL_OPERATION_STATE)
            printf("Starting inf loop - STALL...\n");
        while (operation_state == STALL_OPERATION_STATE);

    }

    return 0;
}