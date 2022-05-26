#define FIRST_MOTOR_TEST_NUM 1
#define LAST_MOTOR_TEST_NUM 1
#define MOTOR_TEST_NUM 0
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
#include "motor.h"
#include "tmr_funcs.h"
#include "ir_gpio_funcs.h"




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
    Motor_Init(motors[0], 0);
    Motor_Init(motors[1], 1);
    Motor_Init(motors[2], 2);
    
    

    // init MOTORS
    if (Motor_Init_Settings(motors, 3) != E_NO_ERROR) 
    {
        printf("MOTOR SETTINGS INITIALIZATION FAILURE\n");
    } 
    else 
    {
        printf("MOTOR SETTINGS INITIALIZED :)\n");
    }
    
    set_current_limit(motors[0], 13);
    set_current_limit(motors[1], 13);
    set_current_limit(motors[2], 13);

    set_motor_profile(motors[0], MOTOR_PROFILE_DEFAULT);
    set_motor_profile(motors[1], MOTOR_PROFILE_DEFAULT);
    set_motor_profile(motors[2], MOTOR_PROFILE_DEFAULT);
    energize(motors[0]);
    energize(motors[1]);
    energize(motors[2]);
    calibrate_motors(motors, 3);
    printf("Motor 1: %d\n", motors[0]->maxMicrostep);
    printf("Motor 2: %d\n", motors[1]->maxMicrostep);
    printf("Motor 3: %d\n", motors[2]->maxMicrostep);
    printf("Motor 1: %d\n", motors[0]->maxStep);
    printf("Motor 2: %d\n", motors[1]->maxStep);
    printf("Motor 3: %d\n", motors[2]->maxStep);
    // energize(motors[MOTOR_TEST_NUM]);
    // MXC_Delay(MSEC(250));   
    // printf("Steps: %f\n", get_max_step(motors[MOTOR_TEST_NUM]));
    // printf("Max microstep: %d\n", motors[MOTOR_TEST_NUM]->maxMicrostep);
    // printf("Curr Angle: %f\n", get_angle(motors[0]));
    // set_motor_profile(motors[0], MOTOR_PROFILE_TORQUE);
    // set_angle(motors[0], 90);
    // printf("Current Postion: %d\n", get_current_position(motors[0]));
    // wait_for_target(motors[0]);
    
    // block_object(motors[MOTOR_TEST_NUM]);
    // MXC_Delay(SEC(2));
    // pull_object(motors[MOTOR_TEST_NUM]);
    // MXC_Delay(SEC(2));
    // go_home_forward(motors[MOTOR_TEST_NUM]);
    // wait_for_home(motors[MOTOR_TEST_NUM]);
    // MXC_Delay(SEC(2));
    // block_object(motors[MOTOR_TEST_NUM]);
    // MXC_Delay(SEC(2));
    // push_object(motors[MOTOR_TEST_NUM]);
    // MXC_Delay(SEC(2));
    // set_target_position(motors[MOTOR_TEST_NUM], motors[MOTOR_TEST_NUM]->maxStep);
    // set_target_position(motors[0], -100);
    printf("Done");
    MXC_Delay(SEC(1));
    // set_motor_profile(motors[MOTOR_TEST_NUM], MOTOR_PROFILE_TORQUE);
    // ======================== Main Loop =========================
    while(1) 
    {   
        for(size_t motor_num = 0; motor_num < 3; motor_num++){
            block_object(motors[motor_num]);
        }
        MXC_Delay(SEC(1));
        for(size_t motor_num = 0; motor_num < 3; motor_num++){
            pull_object(motors[motor_num]);
        }
        MXC_Delay(SEC(3));
        motor_handler(motors, 3);
        // go_home_forward(motors[MOTOR_TEST_NUM]);
        
    //     for(int currMotor = FIRST_MOTOR_TEST_NUM; currMotor < LAST_MOTOR_TEST_NUM+1; currMotor++){
    //         // target_tics(currMotor, OUT_TIC_NUM);
    //         printf("Setting position to 2\n");
    //         // set_target_position(motors[currMotor], 2);
    //         // set_current_limit(motors[2], 32);
            
    //         printf("Current Limit: %d\n", get_current_limit(motors[currMotor]));
    //         printf("Accel Max: %d\n", get_accel_max(motors[currMotor]));
    //         printf("Speed Max: %d\n", get_speed_max(motors[currMotor]));
    //         printf("Speed Start: %d\n", get_speed_start(motors[currMotor]));
    //         printf("Step Mode: %d\n", get_step_mode(motors[currMotor]));
    //         printf("Speed Home Towards: %d\n", get_speed_homing_towards(motors[currMotor]));
    //         MXC_Delay(SEC(3));
    //         go_home_forward(motors[currMotor]);
    //         MXC_Delay(SEC(3));
            
    //     }
        
    //     printf("OUT\n");
        
    //      for(int currMotor = FIRST_MOTOR_TEST_NUM; currMotor < LAST_MOTOR_TEST_NUM+1; currMotor++){
    //         // target_tics(currMotor, IN_TIC_NUM);
    //         go_home_reverse(motors[currMotor]);
    //     }
    //     printf("IN\n");
    //     MXC_Delay(SEC(10));
        // MXC_Delay(SEC(1));
    }
}
