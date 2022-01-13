/**
 * @file    dexarm.c
 */

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include "mxc_device.h"
#include "led.h"
#include "board.h"
#include "mxc_delay.h"

// UART
#include "uart.h"
#include "dma.h"
#include "nvic_table.h"

char err_msg [40];

// UART

#define READ_EN
#define WRITE_EN

#define UART_BAUD           115200
#define BUFF_SIZE           32 //1024

volatile int READ_FLAG;
volatile int DMA_FLAG;

#define READING_UART        2
#define WRITING_UART        3

#ifdef DMA
    void DMA_Handler(void)
    {
        MXC_DMA_Handler();
        DMA_FLAG = 0;
    }
#else
    void UART_Handler(void)
    {
        MXC_UART_AsyncHandler(MXC_UART_GET_UART(READING_UART));
    }
#endif

void readCallback(mxc_uart_req_t* req, int error)
{
    READ_FLAG = error;
}

int error, i, fail = 0;
uint8_t TxData[BUFF_SIZE];
uint8_t RxData[BUFF_SIZE];




void init() {
    // Initialize the data buffers

    memset(TxData, 0x0, BUFF_SIZE);
    memset(RxData, 0x0, BUFF_SIZE);

    // configure interrupts
#ifdef DMA
    MXC_DMA_ReleaseChannel(0);
    NVIC_SetVector(DMA0_IRQn, DMA_Handler);
    NVIC_EnableIRQ(DMA0_IRQn);
#else
    NVIC_ClearPendingIRQ(MXC_UART_GET_IRQ(READING_UART));
    NVIC_DisableIRQ(MXC_UART_GET_IRQ(READING_UART));
    NVIC_SetVector(MXC_UART_GET_IRQ(READING_UART), UART_Handler);
    NVIC_EnableIRQ(MXC_UART_GET_IRQ(READING_UART));
#endif

    // Initialize the UART
    #ifdef READ_EN
    if((error = MXC_UART_Init(MXC_UART_GET_UART(READING_UART), UART_BAUD, MXC_UART_APB_CLK)) != E_NO_ERROR) {
        printf("-->Error initializing UART: %d\n", error);
        while (1) {}
    }
    #endif
    #ifdef WRITE_EN
    if((error = MXC_UART_Init(MXC_UART_GET_UART(WRITING_UART), UART_BAUD, MXC_UART_APB_CLK)) != E_NO_ERROR) {
        printf("-->Error initializing UART: %d\n", error);
        while (1) {}
    }
    #endif
}



void send_cmd(char* cmd, int cmd_len, int wait_ok) {
    // memset(TxData, 0x0, BUFF_SIZE);
    // for (int i = 0; i < cmd_len; i++) {
    //     TxData[i] = cmd[i];
    // }

    mxc_uart_req_t write_req;
    write_req.uart = MXC_UART_GET_UART(WRITING_UART);
    write_req.txData = cmd;
    write_req.txLen = cmd_len;
    write_req.rxLen = 0;
    write_req.callback = NULL;

    error = MXC_UART_Transaction(&write_req);

    if (error != E_NO_ERROR) {
        printf("-->Error starting sync write: %d\n", error);
    } else {
        printf("Tx:%s", TxData);
    }

    int ok = 0;
    while (wait_ok && !ok) {
        READ_FLAG == 1;

        #ifdef DMA
            while (DMA_FLAG);
        #else
            while (READ_FLAG == 1) {
                // printf("READ_FLAG:%d\n", READ_FLAG);
            }
            
            if (READ_FLAG != E_NO_ERROR) {
                printf("-->Error with UART_ReadAsync callback; %d\n", READ_FLAG);
                fail++;
            } else {
                printf("Rx: %s", RxData);
                if (RxData[0] == 'o' && RxData[1] == 'k') {
                    ok = 1;
                }
            }

        #endif
    }
}


void go_home() {
    // Go to home position and enable the motors. Should be called each time when power on.
    send_cmd("M1112\r", 6, 1);
}

void set_work_origin() {
    // Set the current position as the new work origin.
    send_cmd("G92 X0 Y0 Z0 E0\r", 16, 1);
}

void set_acceleration(int acceleration, int travel_acceleration, int retract_acceleration){
    // Set the preferred starting acceleration for moves of different types.
    // Args:
    //     acceleration (int): printing acceleration. Used for moves that employ the current tool.
    //     travel_acceleration (int): used for moves that include no extrusion.
    //     retract_acceleration (int): used for extruder retraction moves.
    char* cmd;
    sprintf(cmd, "M204P%dT%dT%d\r\r\r", acceleration, travel_acceleration, retract_acceleration);
    
    send_cmd(cmd, 21, 1);
}

void set_module_type();
void get_module_type();

void move_to(int x, int y, int z, int e, int feedrate, char* mode, int wait) {
    // Move to a cartesian position. This will add a linear move to the queue to be performed after all previous moves are completed.
    // Args:
    //     mode (string, G0 or G1): G1 by default. use G0 for fast mode
    //     x, y, z (int): The position, in millimeters by default. Units may be set to inches by G20. Note that the center of y axis is 300mm.
    //     feedrate (int): set the feedrate for all subsequent moves
    
    // char* cmd = mode + "F" + str(feedrate)
    // if x is not None:
    //     cmd = cmd + "X"+str(round(x))
    // if y is not None:
    //     cmd = cmd + "Y" + str(round(y))
    // if z is not None:
    //     cmd = cmd + "Z" + str(round(z))
    // if e is not None:
    //     cmd = cmd + "E" + str(round(e))
    // cmd = cmd + "\r\n"
    // self._send_cmd(cmd, wait=wait)
}


void get_current_position();
void delay_ms();
void delay_s();
void soft_gripper_pick();
void soft_gripper_place();
void soft_gripper_stop();
void soft_gripper_neutral();
void air_picker_pick();
void air_picker_place();
void air_picker_neutral();
void air_picker_stop();
void close();

