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
#include "motor_funcs.h"
#include "tmr_funcs.h"
#include "cnn_helper_funcs.h"
#include "ultrasonic.h"
#include "camera_tft_funcs.h"



// *****************************************************************************
int main()
{
    // Switch to 100 MHz clock
    MXC_SYS_Clock_Select(MXC_SYS_CLOCK_IPO);
    SystemCoreClockUpdate();

    // initialize systick
    SysTick_Setup();

    LCD_Camera_Setup();

    startup_cnn();

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
    
    // initialize the ultrasonic sensor pins
    

    // set the motor profile for this test
    // set_motor_profile(0, MOTOR_PROFILE_SPEED);
    // set_motor_profile(1, MOTOR_PROFILE_SPEED);
    // set_motor_profile(2, MOTOR_PROFILE_SPEED);

    // go_home_forward(0);
    // MXC_Delay(10000);
    // go_home_forward(1);
    // MXC_Delay(10000);
    // go_home_forward(2);
    // MXC_Delay(10000);

    init_ultrasonic_timer();
    init_ultrasonic_sensors();
    printf("initialized\n");

    // ======================== Main Loop =========================
    while(1) 
    {
        check_all_callbacks();
    }
}
