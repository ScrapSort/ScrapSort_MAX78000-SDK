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

#include "I2C_funcs.h"
#include "motor_funcs.h"

/***** Globals *****/
uint8_t txdata[I2C_BYTES]; //was static 
uint8_t rxdata[I2C_BYTES]; //was static 
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
    //printf("in callback\n");
    I2C_FLAG = error;
    return;
}

void fill_tx_32b(uint32_t step_speed) {
    // leave [0] to be filled separately with command hex
    txdata[1] = step_speed & 0xFF;
    txdata[2] = (step_speed & 0xFF00) >> 8;
    txdata[3] = (step_speed & 0xFF0000) >> 16;
    txdata[4] = (step_speed & 0xFF000000) >> 24;
}

void printTransaction(int slave_addr, int tx_len, int rx_len) {
    printf("\n-->Transaction with slave %d\n", slave_addr);
    if (tx_len > 0) {
        printf("\tWriting: ");
        for (int i = 0; i < tx_len; i++) {
            printf("%02x ", txdata[i]);
        }
        printf("\n");
    }
    if (rx_len > 0) {
        printf("\tReading: ");
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
        //printf("-->Failed master\n");
        return E_COMM_ERR;
    }
    else {
        printf("\n-->I2C Master Initialization Complete\n");
    }
    
    MXC_I2C_SetFrequency(I2C_MASTER, I2C_FREQ);


    return E_NO_ERROR;
}

int I2C_Broadcast_Message(int tx_len, int rx_len, int restart) {
    error = E_NO_ERROR;

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
            error += E_COMM_ERR;
        }
        MXC_Delay(4000);
        //printTransaction(slave_addr, tx_len, rx_len);
    }

    return error;
}

int I2C_Send_Message(int slave_addr, int tx_len, int rx_len, int restart) {
    //if (!slave_addr) return I2C_Broadcast_Message(tx_len, rx_len, restart);

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
        return E_COMM_ERR;
    }

    //printTransaction(slave_addr, tx_len, rx_len);

    return E_NO_ERROR;
}
