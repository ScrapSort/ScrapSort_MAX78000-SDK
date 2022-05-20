/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mxc_device.h"
#include "mxc_delay.h"
#include "nvic_table.h"
#include "i2c_regs.h"
#include "i2c.h"
#include "dma.h"
#include "pb.h"
#include "board.h"
#include "mxc.h"


// personal
#include "e_queue.h"

void SW1_function() {
    printf("Ran SW1_function!\n");
}

void SW2_function() {
    printf("Ran SW2_function!\n");
}

void SW1_Callback()
{

}

void SW2_Callback()
{
}


// *****************************************************************************

int main() {

    // Configure push buttons
    PB_RegisterCallback(0, SW1_Callback);
    PB_IntEnable(0);
    PB_RegisterCallback(1, SW2_Callback);
    PB_IntEnable(1);



    return 0;
}