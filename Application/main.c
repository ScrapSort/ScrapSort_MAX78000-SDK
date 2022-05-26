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

// *****************************************************************************
int main()
{
    // Switch to 100 MHz clock
    MXC_SYS_Clock_Select(MXC_SYS_CLOCK_IPO);
    SystemCoreClockUpdate();
    
    // set up the camera and LCD
    LCD_Camera_Setup();
    // init the CNN accelerator
    startup_cnn();
    
  
    // SYSTICK
    SysTick_Setup();
   
    

    // init I2C
    if (I2C_Init() != E_NO_ERROR) 
    {
        printf("I2C INITIALIZATION FAILURE\n");
    } 
    else 
    {
        printf("I2C INITIALIZED :)\n");
    } 
    
    // Initialize test data
    for (int i = 0; i < I2C_BYTES; i++) 
    {
        txdata[i] = 0;
        rxdata[i] = 0;
    }

    init_ultrasonic_timer();
    init_ultrasonic_sensors();
    printf("Ultrasonics Initialized\n");

    // init MOTORS
    Motor_Init(motors[0], 0);
    Motor_Init(motors[1], 1);
    Motor_Init(motors[2], 2);
    if (Motor_Init_Settings(motors, 3) != E_NO_ERROR) 
    {
        printf("MOTOR SETTINGS INITIALIZATION FAILURE\n");
    } 
    else 
    {
        printf("MOTOR SETTINGS INITIALIZED :)\n");
    }
    
    MXC_Delay(SEC(1));

    

    // ======================== Main Loop =========================
    while(1) 
    {
        check_all_callbacks();
        motor_handler(motors, 3);
        heartbeat();
    }
}
