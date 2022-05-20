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
#include "led.h"

// personal
#include "motor_funcs.h"
#include "I2C_funcs.h"
#include "tic.h"

void Debug_Motors(void) {
    // PRINT OUT STATUS/ERROR VARS FOR DEBUG
    printf("\n#######  DEBUG VARS  #######\n\n");

    for (int slave_addr = START_SLAVE_ADDR; slave_addr < START_SLAVE_ADDR + NUM_SLAVES; slave_addr++) {
        
        // GET OPERATION STATE
        printf("Operation Status: ");
        txdata[0] = 0xA1;
        txdata[1] = 0x00;

        I2C_Send_Message(slave_addr, 2, 1, 0);


        // GET MISC FLAGS
        printf("Misc Flags: ");
        txdata[0] = 0xA1;
        txdata[1] = 0x01;

        I2C_Send_Message(slave_addr, 2, 1, 0);

        // GET ERROR STATUS
        printf("Error Status: ");
        txdata[0] = 0xA1;
        txdata[1] = 0x02;

        I2C_Send_Message(slave_addr, 2, 2, 0);

        // GET ERRORS OCCURRED
        printf("Errors Occurred: ");
        txdata[0] = 0xA1;
        txdata[1] = 0x04;

        I2C_Send_Message(slave_addr, 2, 4, 0);

        // GET CURRENT POSITION
        printf("Current Position: ");
        txdata[0] = 0xA1;
        txdata[1] = 0x22;

        I2C_Send_Message(slave_addr, 2, 4, 0);

        printf("\n############\n");

        
    }
}


void set_motor_profile(uint8_t slave_addr, MOTOR_PROFILE profile){
    uint32_t profile_accel_max, profile_decel_max, profile_speed_max, profile_speed_start;
    uint8_t profile_step_mode; 

    if(profile==MOTOR_PROFILE_TORQUE){
        profile_accel_max = 200*100;
        profile_decel_max = profile_accel_max;
        profile_speed_max = 400*10000;
        // profile_speed_max = 900*10000;
        profile_speed_start = 0;
        profile_step_mode = TicStepMode__Microstep4;
    }
    else if(profile == MOTOR_PROFILE_SPEED){
        profile_accel_max = 5000*100;
        profile_decel_max = profile_accel_max;
        profile_speed_max = 2000*10000;
        profile_speed_start = 0;
        profile_step_mode = TicStepMode__Full;
    }
    else{
        printf("Invalid Motor Profile Set");
        return;
    }

    set_accel_max(slave_addr, profile_accel_max);
    set_decel_max(slave_addr, profile_decel_max);
    set_speed_max(slave_addr, profile_speed_max);
    set_speed_start(slave_addr, profile_speed_start);
    set_step_mode(slave_addr, profile_step_mode);
}

void deenergize(uint8_t slave_addr){
    txdata[0] = TicCommand__Deenergize;
    I2C_Send_Message(slave_addr, 1, 0, 0);
}

void energize(uint8_t slave_addr){
    txdata[0] = TicCommand__Energize;
    I2C_Send_Message(slave_addr, 1, 0, 0);
}

void set_target_velocity(uint8_t slave_addr, uint32_t target_velocity){
    txdata[0] = TicCommand__SetTargetVelocity;
    fill_tx_32b(target_velocity);
    I2C_Send_Message(slave_addr, 5, 0, 0);
}

void set_speed_max(uint8_t slave_addr, uint32_t speed_max){
    txdata[0] = TicCommand__SetSpeedMax;
    fill_tx_32b(speed_max);
    I2C_Send_Message(slave_addr, 5, 0, 0);
}

void set_speed_start(uint8_t slave_addr, uint32_t speed_start){
    txdata[0] = TicCommand__SetStartingSpeed;
    fill_tx_32b(speed_start);
    I2C_Send_Message(slave_addr, 5, 0, 0);
}

void set_accel_max(uint8_t slave_addr, uint32_t accel_max){
    txdata[0] = TicCommand__SetAccelMax;
    fill_tx_32b(accel_max);
    I2C_Send_Message(slave_addr, 5, 0, 0);
}

void set_decel_max(uint8_t slave_addr, uint32_t decel_max){
    txdata[0] = TicCommand__SetDecelMax;
    fill_tx_32b(decel_max);
    I2C_Send_Message(slave_addr, 5, 0, 0);
}

void set_step_mode(uint8_t slave_addr, uint8_t step_mode){
    txdata[0] = TicCommand__SetStepMode;
    fill_tx_32b(step_mode);
    I2C_Send_Message(slave_addr, 5, 0, 0);
}

void set_current_limit(uint8_t slave_addr, uint8_t current_limit){
    txdata[0] = TicCommand__SetCurrentLimit;
    fill_tx_32b(current_limit);
    I2C_Send_Message(slave_addr, 5, 0, 0);
}

void set_decay_mode(uint8_t slave_addr, uint8_t decay_mode){
    txdata[0] = TicCommand__SetDecayMode;
    fill_tx_32b(decay_mode);
    I2C_Send_Message(slave_addr, 5, 0, 0);
}

void rotate_revs(int slave_addr, float rotations) {
    

    // GET CURRENT POSITION
    txdata[0] = 0xA1;
    txdata[1] = 0x22;

    I2C_Send_Message(slave_addr, 2, 4, 0);

    int curr_pos = rxdata[0] + (rxdata[1] << 8) + (rxdata[2] << 16) + (rxdata[3] << 24);

    // SET TARGET POSITION
    // full rotation = 200 encoder ticks

    int enc_target = (int)(rotations * 200 + curr_pos);

    // printf("")

    txdata[0] = 0xE0;
    fill_tx_32b(enc_target);

    I2C_Send_Message(slave_addr, 5, 0, 0);
}

void target_tics(int slave_addr, int enc_tics) {
    
    // SET TARGET POSITION
    // full rotation = 200 encoder ticks

    txdata[0] = 0xE0;
    fill_tx_32b(enc_tics);

    I2C_Send_Message(slave_addr, 5, 0, 0);
}

int Motor_Init_Settings() {

    // RESET COMMAND TIMEOUT
    txdata[0] = 0x8C;

    I2C_Broadcast_Message(1, 0, 0);

    // RESET 
    txdata[0] = 0xB0;
    I2C_Broadcast_Message(1, 0, 0);

    MXC_Delay(1000);

    // RESET COMMAND TIMEOUT
    // txdata[0] = 0x8C;

    // I2C_Broadcast_Message(1, 0, 0);

    // EXIT SAFE START
    txdata[0] = 0x83;
    I2C_Broadcast_Message(1, 0, 0);

    // ENERGIZE
    txdata[0] = 0x85;
    I2C_Broadcast_Message(1, 0, 0);

    // // SET MAX SPEED
    // // 0 to 500,000,000 microsteps per 10,000 s
    // txdata[0] = 0xE6;
    // fill_tx_32b(499999999);
    // I2C_Broadcast_Message(5, 0, 0);

    // // SET MAX ACCELERATION
    // // 100 to 2,147,483,647 = 0x64 to 0x7FFF FFFF
    // txdata[0] = 0xEA;
    // fill_tx_32b(600000);
    // I2C_Broadcast_Message(5, 0, 0);

    // // SET MAX DECCELERATION
    // // 100 to 2,147,483,647 = 0x64 to 0x7FFF FFFF
    // txdata[0] = 0xE9;
    // fill_tx_32b(600000);
    // I2C_Broadcast_Message(5, 0, 0);


    for (int slave_addr = START_SLAVE_ADDR; slave_addr < START_SLAVE_ADDR + NUM_SLAVES; slave_addr++) {
        // GET VARIABLE: OPERATION STATE
        txdata[0] = 0xA1;
        txdata[1] = 0x00;

        I2C_Send_Message(slave_addr, 2, 1, 0);

        if (rxdata[0] != 10) { // normal operation state
            printf("ERROR could not init motor %d\n", slave_addr);
            // GET ERROR STATUS
            txdata[0] = 0xA2;
            txdata[1] = 0x02;

            I2C_Send_Message(slave_addr, 2, 1, 0);
            printf("\tERROR CODE: %d\n", rxdata[0]);
            return -1;
        }
    }

    return E_NO_ERROR;

}