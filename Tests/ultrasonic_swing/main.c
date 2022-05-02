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
#include "camera_tft_funcs.h"
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

    // init MOTORS
    if (Motor_Init_Settings() != E_NO_ERROR) 
    {
        printf("MOTOR SETTINGS INITIALIZATION FAILURE\n");
    } 
    else 
    {
        printf("MOTOR SETTINGS INITIALIZED :)\n");
    }
    
    // initialize the ultrasonic sensor pins
    init_trigger_gpios();  
    init_echo_gpios();

    // set the motor profile for this test
    set_motor_profile(0, MOTOR_PROFILE_SPEED);
    set_motor_profile(1, MOTOR_PROFILE_SPEED);
    set_motor_profile(2, MOTOR_PROFILE_SPEED);

    startup_cnn();

    // activate the first ultrasonic sensor
    activate_triggercam();

    // ======================== Main Loop =========================
    
    while(1) 
    {
        // check interrupt callbacks (code that should be executed outside interrupts)
        check_all_callbacks();

        // check if the next ultrasonic sensor should be triggered
        to_trigger();

        if(global_counter % 20000 == 0)
        {
            get_heartbeat();
        }
    }
}
