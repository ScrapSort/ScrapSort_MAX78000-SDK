/**
 * @file        main.c
 * @brief       I2C Loopback Example
 * @details     This example uses the I2C Master to read/write from/to the I2C Slave. For
 *              this example you must connect P0.12 to P0.18 (SCL) and P0.13 to P0.19 (SCL). The Master
 *              will use P0.12 and P0.13. The Slave will use P0.18 and P0.19. You must also
 *              connect the pull-up jumpers (JP23 and JP24) to the proper I/O voltage.
 *              Refer to JP27 to determine the I/O voltage.
 * @note        Other devices on the EvKit will be using the same bus. This example cannot be combined with
 *              a PMIC or bluetooth example because the I2C Slave uses GPIO pins for those devices.
 */

/*******************************************************************************
* Copyright (C) Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
*
******************************************************************************/


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


/***** Definitions *****/
// #define MASTERDMA

#define I2C_MASTER      MXC_I2C1
// #define I2C_SLAVE       MXC_I2C0

#define I2C_FREQ        100000
#define I2C_BYTES       32

#define NUM_SLAVES 1
#define START_SLAVE_ADDR 2

// #define I2C_SLAVE_ADDR1  1 
// #define I2C_SLAVE_ADDR2  2 
// #define I2C_SLAVE_ADDR3  3 
// #define I2C_SLAVE_ADDR4  4 
// #define I2C_SLAVE_ADDR5  5 

typedef enum {
    FAILED,
    PASSED
} test_t;

/***** Globals *****/
static uint8_t txdata[I2C_BYTES];
static uint8_t rxdata[I2C_BYTES];
volatile uint8_t DMA_FLAG = 0;
volatile int I2C_FLAG;
volatile int txnum = 0;
volatile int txcnt = 0;
volatile int rxnum = 0;
volatile int num;
int error;

/***** Functions *****/

//I2C callback function
void I2C_Callback(mxc_i2c_req_t* req, int error)
{
    printf("in callback\n");
    I2C_FLAG = error;
    return;
}

void printTransaction(int slave_addr, int tx_len, int rx_len) {
    printf("\n-->Transaction with slave %d\n", slave_addr);
    if (tx_len > 0) {
        printf("\tWriting: ");
        for (int i = 0; i < tx_len; i++) {
            printf("%02x ", txdata[i]);
        }
    }
    if (rx_len > 0) {
        printf("\n\tReading: ");
        for (int i = 0; i < rx_len; i++) {
            printf("%02x ", rxdata[i]);
        }
        printf("\n");
    }
}

//Prints out human-friendly format to read txdata and rxdata
void printData(void)
{
    int i;
    printf("\n-->TxData: ");
    
    for (i = 0; i < sizeof(txdata); ++i) {
        printf("%02x ", txdata[i]);
    }
    
    printf("\n\n-->RxData: ");
    
    for (i = 0; i < sizeof(rxdata); ++i) {
        printf("%02x ", rxdata[i]);
    }
    
    printf("\n");
    
    return;
}


int I2C_Init() {
    error = 0;

    //Setup the I2CM
    for (int slave_addr = START_SLAVE_ADDR; slave_addr < START_SLAVE_ADDR + NUM_SLAVES; slave_addr++) {
        error += MXC_I2C_Init(I2C_MASTER, 1, slave_addr);
    }
    
    if (error != E_NO_ERROR) {
        printf("-->Failed master\n");
        return 1;
    }
    else {
        printf("\n-->I2C Master Initialization Complete\n");
    }
    
    MXC_I2C_SetFrequency(I2C_MASTER, I2C_FREQ);


    return 0;
}

int I2C_Send_Message(int slave_addr, int tx_len, int rx_len, int restart) {
    mxc_i2c_req_t reqMaster;
    reqMaster.i2c = I2C_MASTER;
    reqMaster.addr = slave_addr;
    reqMaster.tx_buf = txdata;
    reqMaster.tx_len = tx_len; //I2C_BYTES;
    reqMaster.rx_buf = rxdata;
    reqMaster.rx_len = rx_len; //I2C_BYTES;
    reqMaster.restart = restart; //0
    reqMaster.callback = I2C_Callback;
    I2C_FLAG = 1;
    
    if ((error = MXC_I2C_MasterTransaction(&reqMaster)) != 0) {
        printf("ERROR WRITING: %d\n\tSlave Addr: %d\n", error, slave_addr);
        return FAILED;
    }

    printTransaction(slave_addr, tx_len, rx_len);

    return 0;
}

int I2C_Broadcast_Message(int tx_len, int rx_len, int restart) {

    for (int slave_addr = START_SLAVE_ADDR; slave_addr < START_SLAVE_ADDR + NUM_SLAVES; slave_addr++) {
        mxc_i2c_req_t reqMaster;
        reqMaster.i2c = I2C_MASTER;
        reqMaster.addr = slave_addr;
        reqMaster.tx_buf = txdata;
        reqMaster.tx_len = tx_len; //I2C_BYTES;
        reqMaster.rx_buf = rxdata;
        reqMaster.rx_len = rx_len; //I2C_BYTES;
        reqMaster.restart = restart; //0
        reqMaster.callback = I2C_Callback;
        I2C_FLAG = 1;
        
        if ((error = MXC_I2C_MasterTransaction(&reqMaster)) != 0) {
            printf("ERROR WRITING: %d\n\tSlave Addr: %d\n", error, slave_addr);
            return FAILED;
        }

        printTransaction(slave_addr, tx_len, rx_len);
    }

    return 0;
}

void Debug_Motors(void) {
    // PRINT OUT STATUS/ERROR VARS FOR DEBUG
    printf("\n#####\tDEBUG VARS\t#####\n\n");

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

        
    }
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
    // txdata[0] = 0xE6; // command
    // txdata[1] = 0x00; 
    // txdata[2] = 0x09;
    // txdata[3] = 0x3D;
    // txdata[4] = 0x00; 

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

    return 0;

}

// *****************************************************************************
int main()
{
    LED_Init();
    LED_On(1);

    printf("\n******** Steppers! *********\n");
    if (I2C_Init() != 0) {
        printf("I2C INITIALIZATION FAILURE");
    } else {
        printf("I2C INITIALIZED :)");
    }
   
    int i = 0;
    // Initialize test data
    for (i = 0; i < I2C_BYTES; i++) {
        txdata[i] = 0;
        rxdata[i] = 0;
    }

    if (Motor_Init_Settings() != 0) {
        printf("MOTOR SETTINGS INITIALIZATION FAILURE");
    } else {
        printf("MOTOR SETTINGS INITIALIZED :)");
    }

    // SET TARGET POSITION
    // full rotation = 200 encoder ticks
    txdata[0] = 0xE0;
    txdata[1] = 0x90;
    txdata[2] = 0x01;
    txdata[3] = 0x00;
    txdata[4] = 0x00; 

    I2C_Broadcast_Message(5, 0, 0);

    printf("\n");
    printData();
    printf("\n");

    MXC_Delay(5000000);

    Debug_Motors();

    // while(1);
}
