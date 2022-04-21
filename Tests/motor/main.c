#define FIRST_MOTOR_TEST_NUM 0
#define LAST_MOTOR_TEST_NUM 0
#define IN_TIC_NUM 10
#define OUT_TIC_NUM -120

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
#include "ir_gpio_funcs.h"




// *****************************************************************************
int main()
{
    
    // Switch to 100 MHz clock
    MXC_SYS_Clock_Select(MXC_SYS_CLOCK_IPO);
    SystemCoreClockUpdate();
    
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

    // init MOTORS
    if (Motor_Init_Settings() != E_NO_ERROR) 
    {
        printf("MOTOR SETTINGS INITIALIZATION FAILURE\n");
    } 
    else 
    {
        printf("MOTOR SETTINGS INITIALIZED :)\n");
    }
    
    
    // ======================== Main Loop =========================
    while(1) 
    {
        for(int currMotor = FIRST_MOTOR_TEST_NUM; currMotor < LAST_MOTOR_TEST_NUM+1; currMotor++){
            target_tics(currMotor, OUT_TIC_NUM);
        }
        printf("OUT\n");
        MXC_Delay(SEC(2));
         for(int currMotor = FIRST_MOTOR_TEST_NUM; currMotor < LAST_MOTOR_TEST_NUM+1; currMotor++){
            target_tics(currMotor, IN_TIC_NUM);
        }
        printf("IN\n");
        MXC_Delay(SEC(2));
    
    }
}
