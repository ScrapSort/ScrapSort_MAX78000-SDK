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
#include "ir_gpio_funcs.h"

#include "sorter.h"
#include "cnn_helper_funcs.h"
#include "camera_tft_funcs.h"
#include "ultrasonic.h"

//#define COLLECT_DATA
//#define STREAM_MODE

#ifdef COLLECT_DATA
#include "capture_button.h"
#endif


// *****************************************************************************
int main()
{
    // Switch to 100 MHz clock
    MXC_SYS_Clock_Select(MXC_SYS_CLOCK_IPO);
    SystemCoreClockUpdate();
    
    // set up the camera and LCD
    LCD_Camera_Setup();

    #ifdef COLLECT_DATA
    init_card();
    init_class_button();
    init_capture_button();
    #endif

    #ifndef STREAM_MODE
    // SYSTICK
    SysTick_Setup();
    #endif
    
    #ifndef COLLECT_DATA
    // init the CNN accelerator
    //startup_cnn();

    #ifndef STREAM_MODE
    // init the IR GPIOs
    //gpio_init();
    #endif

    // init the PWM & TMR
    //PWMTimer();
    
    #ifndef STREAM_MODE
    // // init I2C
    // if (I2C_Init() != E_NO_ERROR) 
    // {
    //     printf("I2C INITIALIZATION FAILURE\n");
    // } 
    // else 
    // {
    //     printf("I2C INITIALIZED :)\n");
    // } 
    
    // // Initialize test data
    // for (int i = 0; i < I2C_BYTES; i++) 
    // {
    //     txdata[i] = 0;
    //     rxdata[i] = 0;
    // }

    // // init MOTORS
    // if (Motor_Init_Settings() != E_NO_ERROR) 
    // {
    //     printf("MOTOR SETTINGS INITIALIZATION FAILURE\n");
    // } 
    // else 
    // {
    //     printf("MOTOR SETTINGS INITIALIZED :)\n");
    // }
    #endif
    #endif

    // cnn_output_t output;
    // set_motor_profile(0, MOTOR_PROFILE_SPEED);
    // set_motor_profile(1, MOTOR_PROFILE_SPEED);
    // set_motor_profile(2, MOTOR_PROFILE_SPEED);

    init_trigger();  
    init_ultrasonic_gpios();

    // ======================== Main Loop =========================
    while(1) 
    {
        #ifndef COLLECT_DATA
        #ifndef STREAM_MODE
        // keep checking for interrupt flags
        check_all_callbacks();
        #endif
        #endif
        
        #ifdef COLLECT_DATA
        capture_camera_img();
      
        display_RGB565_img(56,96,NULL,false);
        if(clicked() == 1)
        {
            capture();
        }
        if(switched() == 1)
        {
            switch_class();
        }
        #endif

        #ifdef STREAM_MODE
        output = *run_cnn();
        show_cnn_output(output);
        #endif
    }
}
