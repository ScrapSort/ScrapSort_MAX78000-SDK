#define FIRST_MOTOR_TEST_NUM 0
#define LAST_MOTOR_TEST_NUM 0
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
//#include "ir_gpio_funcs.h"

#include "ultrasonic.h"



// *****************************************************************************
int main()
{
    MXC_ICC_Enable(MXC_ICC0); // Enable cache
    // Switch to 100 MHz clock
    MXC_SYS_Clock_Select(MXC_SYS_CLOCK_IPO);
    SystemCoreClockUpdate();
    printf("systick init\n");
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
    
    init_trigger();  
    init_ultrasonic_gpios();
    set_motor_profile(0, MOTOR_PROFILE_SPEED);
    set_motor_profile(1, MOTOR_PROFILE_SPEED);
    set_motor_profile(2, MOTOR_PROFILE_SPEED);
    //printf("reload:%lu\n",SysTick->LOAD);
    //SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk;
    // ======================== Main Loop =========================
    
    activatecam();
    while(1) 
    {
        // activatecam();
        // //MXC_Delay(200);
        // activate0();
        // MXC_Delay(1000000);
        // activate1();
        // MXC_Delay(20000);
        // activate0();
        // MXC_Delay(20000);
        
        //MXC_Delay(20000);
        //triggered();
    }
}
