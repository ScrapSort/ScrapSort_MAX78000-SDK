#define FIRST_MOTOR_TEST_NUM 1
#define LAST_MOTOR_TEST_NUM 1
#define IN_TIC_NUM 10
#define OUT_TIC_NUM -110

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





// *****************************************************************************
int main()
{
    
    // Switch to 100 MHz clock
    MXC_SYS_Clock_Select(MXC_SYS_CLOCK_IPO);
    SystemCoreClockUpdate();
    

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
    set_motor_profile(0, MOTOR_PROFILE_TORQUE);
    set_motor_profile(1, MOTOR_PROFILE_TORQUE);
    set_motor_profile(2, MOTOR_PROFILE_TORQUE);
    
    // ======================== Main Loop =========================
    while(1) 
    {
        
        for(int currMotor = FIRST_MOTOR_TEST_NUM; currMotor < LAST_MOTOR_TEST_NUM+1; currMotor++){
            printf("Before Out\n");
            // target_tics(currMotor, OUT_TIC_NUM);
            go_home_forward(currMotor);
            MXC_Delay(MSEC(1));
        }
        printf("OUT\n");
        MXC_Delay(SEC(4));
         for(int currMotor = FIRST_MOTOR_TEST_NUM; currMotor < LAST_MOTOR_TEST_NUM+1; currMotor++){
            // target_tics(currMotor, IN_TIC_NUM);
            go_home_reverse(currMotor);
            MXC_Delay(MSEC(1));
        }
        printf("IN\n");
        MXC_Delay(SEC(4));
    
    }
}
