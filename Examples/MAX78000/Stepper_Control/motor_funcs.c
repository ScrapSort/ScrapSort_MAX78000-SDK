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

void rotate_revs(int slave_addr, double rotations) {

    // GET CURRENT POSITION
    txdata[0] = 0xA1;
    txdata[1] = 0x22;

    I2C_Send_Message(slave_addr, 2, 4, 0);

    int curr_pos = rxdata[0] + (rxdata[1] << 8) + (rxdata[2] << 16) + (rxdata[3] << 24);


    // SET TARGET POSITION
    // full rotation = 200 encoder ticks

    int enc_target = (int)rotations * 200 + curr_pos;

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
    txdata[0] = 0x8C;

    I2C_Broadcast_Message(1, 0, 0);

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