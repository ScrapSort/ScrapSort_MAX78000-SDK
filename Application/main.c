/**
 * @file        main.c
 * @brief       Scrapsort Application
 * @details     This program creates a self-sufficient recycling device.
 */

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
#include "I2C_funcs.h"
#include "motor.h"
#include "tmr_funcs.h"

#include "sorter.h"
#include "cnn_helper_funcs.h"
#include "camera_tft_funcs.h"
#include "ultrasonic.h"
#include "heartbeat.h"
#include "watchdog.h"
#include "color_print.h"
#include "debug_flags.h"



// *****************************************************************************
int main()
{
    printf("\n\n" ANSI_COLOR_YELLOW "****************** SCRAPSORT ******************" ANSI_COLOR_RESET "\n\n");

    // printf("*** DEBUG STATUS ***\n");


    printf("*** SETUP ***\n\n");

    // Switch to 100 MHz clock
    MXC_SYS_Clock_Select(MXC_SYS_CLOCK_IPO);
    SystemCoreClockUpdate();
    printf(ANSI_COLOR_GREEN "--> Core Clock Updated" ANSI_COLOR_RESET "\n");

    // Init Watchdog
    MXC_WDT_Setup();
    pat_the_dog();

    
    // Set up the Camera and LCD
    LCD_Camera_Setup();
    pat_the_dog();

    // Init CNN accelerator
    startup_cnn();
    printf(ANSI_COLOR_GREEN "--> CNN Started" ANSI_COLOR_RESET "\n");
    pat_the_dog();
    
  
    // Init Systick
    SysTick_Setup();
    pat_the_dog();

    // Init I2C
    if (I2C_Init() != E_NO_ERROR) {
        printf(ANSI_COLOR_RED "--> I2C Initialization Failure" ANSI_COLOR_RESET "\n");
    } else {
        printf(ANSI_COLOR_GREEN "--> I2C Initialized" ANSI_COLOR_RESET "\n");
    }
    pat_the_dog();
    
    // Initialize test data
    memset(txdata, 0, I2C_BYTES);
    memset(rxdata, 0, I2C_BYTES);
    // for (int i = 0; i < I2C_BYTES; i++) 
    // {
    //     txdata[i] = 0;
    //     rxdata[i] = 0;
    // }

    // Init Ultrasonics
    init_ultrasonic_timer();
    init_ultrasonic_sensors();
    printf(ANSI_COLOR_GREEN "--> Ultrasonics Initialized" ANSI_COLOR_RESET "\n");
    pat_the_dog();


    // Init Motors
    printf("--> Motors\n");
    Motor_Init(motors[0], 0);
    Motor_Init(motors[1], 1);
    Motor_Init(motors[2], 2);
    
    pat_the_dog();

    if (Motor_Init_Settings(motors, 3) != E_NO_ERROR) {
        printf(ANSI_COLOR_RED "--> Motor Settings Initialization Failure" ANSI_COLOR_RESET "\n");
    } else {
        printf(ANSI_COLOR_GREEN "--> Motor Settings Initialized" ANSI_COLOR_RESET "\n");
    }

    pat_the_dog();
    
    MXC_Delay(SEC(1));

    

    // ======================== Main Loop =========================
    
    printf("\n\n*** READY TO SORT ***\n\n");

    while(1) 
    {
        check_all_callbacks();
        motor_handler(motors, 3);
        pat_the_dog();
        
        #ifdef DEBUG_HEARTBEAT
        heartbeat();
        #endif
    }
}
