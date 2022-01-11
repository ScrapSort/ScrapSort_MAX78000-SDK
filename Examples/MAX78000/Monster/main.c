/*******************************************************************************
 * Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
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
 * $Date: 2018-09-05 16:46:11 -0500 (Wed, 05 Sep 2018) $
 * $Revision: 37695 $
 *
 ******************************************************************************/

/**
 * @file    main.c
 * @brief   Hello World!
 * @details This example uses the UART to print to a terminal and flashes an LED.
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

// TFT BEGIN
#include <stdlib.h>
#include <string.h>
#include "mxc.h"
#include "icc.h"
#include "utils.h"
#include "state.h"
#include "tft_fthr.h"
#include "keypad.h"
#include "led.h"
#include "pb.h"

char err_msg [40];


#define TFT_ENABLE    

#ifdef TFT_ENABLE
#define TFT_BUFF_SIZE		32		// TFT buffer size

void TFT_Print(char *str, int x, int y, int font) {
	// fonts id
	text_t text;
	text.data = str;
	text.len = 32;
	MXC_TFT_PrintFont(x, y, font, &text, NULL);
}

void quick_print(char *str, int buff_size) {
    #ifdef TFT_ENABLE
        MXC_TFT_ClearScreen();
        MXC_Delay(100000);

        MXC_TFT_SetBackGroundColor(BLACK);
        MXC_TFT_SetForeGroundColor(WHITE);   // set chars to white
        char buff[buff_size];
        
        memset(buff,0, buff_size);
        sprintf(buff, str);


        // TFT_Print(buff, 0, 10, (int)&Arial12x12[0]);
        text_t text;
        text.data = str;
        text.len = buff_size;
        MXC_TFT_PrintFont(0, 10, (int)&Arial12x12[0], &text, NULL);

        MXC_Delay(500000);
    #endif
}
#endif


// UART

// #define READ_EN
#define WRITE_EN

#define UART_BAUD           115200
#define BUFF_SIZE           4 //1024

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

// *****************************************************************************
int main(void)
{
    // LED + GENERAL SETUP
    LED_Init();

    int LED_INDEX = 0;
    int usec_delay = 3000000;
    
    // printf("M1112\0");
    // printf("M1112\n");
    LED_On(LED_INDEX);

    // MXC_Delay(usec_delay);
    // int z = 50;


    // TFT SETUP
    #ifdef TFT_ENABLE
        // Wait for PMIC 1.8V to become available, about 180ms after power up.
        MXC_Delay(200000);

        /* Enable cache */
        MXC_ICC_Enable(MXC_ICC0);

        /* Set system clock to 100 MHz */
        // MXC_SYS_Clock_Select(MXC_SYS_CLOCK_IPO);
        // SystemCoreClockUpdate();

        /* Initialize TFT display */
        MXC_TFT_Init(MXC_SPI0, 1, NULL, NULL);
        // TFT_Feather_test();

        char buff[TFT_BUFF_SIZE];
        sprintf(buff, "begin");
        quick_print(buff, 5);
    #endif 

    // UART
    
    int error, i, fail = 0;
    uint8_t TxData[BUFF_SIZE];
    uint8_t RxData[BUFF_SIZE];

    // printf("\n\n**************** MONSTER START ******************\n");
    // printf("This example sends data from one UART to another.\n");
    // printf("\nConnect the TX pin of UART%d to the RX pin of UART%d for this example.\n", WRITING_UART, READING_UART);
    
    // printf("\n-->UART Baud \t: %d Hz\n", UART_BAUD);
    // printf("\n-->Test Length \t: %d bytes\n", BUFF_SIZE);
    

    // Initialize the data buffers

    memset(TxData, 0x0, BUFF_SIZE);
#ifdef WRITE_EN
    int str_len = 7;
    char* str = "M1112\n"; //"M1112\n";
    
    for (i = 0; i < str_len; i++) {
        TxData[i] = str[i];
    }
#endif
    // for (i = 0; i < BUFF_SIZE; i++) {
    //     TxData[i] = i;
    // }
    memset(RxData, 0x0, BUFF_SIZE);

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
        // printf("-->Error initializing UART: %d\n", error);
        // printf("-->Example Failed\n");
        sprintf(buff, "-->Error initializing UART: %d\n", error);
        quick_print(buff, 32);
        while (1) {}
    }
    #endif
    #ifdef WRITE_EN
    if((error = MXC_UART_Init(MXC_UART_GET_UART(WRITING_UART), UART_BAUD, MXC_UART_APB_CLK)) != E_NO_ERROR) {
        // printf("-->Error initializing UART: %d\n", error);
        // printf("-->Example Failed\n");
        sprintf(buff, "-->Error initializing UART: %d\n", error);
        quick_print(buff, 32);
        while (1) {}
    }
    #endif

    // printf("-->UART Initialized\n\n");
    // sprintf(buff, "-->UART Initialized\n");
    // quick_print(buff);


    while (1) {
    #ifdef READ_EN
        mxc_uart_req_t read_req;
        read_req.uart = MXC_UART_GET_UART(READING_UART);
        read_req.rxData = RxData;
        read_req.rxLen = BUFF_SIZE;
        read_req.txLen = 0;
        read_req.callback = readCallback;
        
        READ_FLAG = 1;    
        DMA_FLAG = 1;

        MXC_UART_ClearRXFIFO(MXC_UART_GET_UART(READING_UART));
    #endif

    #ifdef WRITE_EN
        mxc_uart_req_t write_req;
        write_req.uart = MXC_UART_GET_UART(WRITING_UART);
        write_req.txData = TxData;
        write_req.txLen = BUFF_SIZE;
        write_req.rxLen = 0;
        write_req.callback = NULL;
    #endif
        
        // READ_FLAG = 1;    
        // DMA_FLAG = 1;

        // MXC_UART_ClearRXFIFO(MXC_UART_GET_UART(READING_UART));

    #ifdef READ_EN
        #ifdef DMA
            error = MXC_UART_TransactionDMA(&read_req);
        #else
            error = MXC_UART_TransactionAsync(&read_req);
        #endif

            if (error != E_NO_ERROR) {
                // printf("-->Error starting async read: %d\n", error);
                // printf("-->Example Failed\n");
                sprintf(buff, "-->Error starting async read: %d\n", error);
                quick_print(buff, 32);
                while (1) {}
            }
    #endif

    #ifdef WRITE_EN
        error = MXC_UART_Transaction(&write_req);

        if (error != E_NO_ERROR) {
            // printf("-->Error starting sync write: %d\n", error);
            // printf("-->Example Failed\n");
            sprintf(buff, "-->Error starting sync write: %d\n", error);
            quick_print(buff, 32);
            while (1) {}
        } else {
            sprintf(buff, "Tx:%s", TxData);
            quick_print(buff, 32);
        }
    #endif

    #ifdef READ_EN
        #ifdef DMA
            
            while (DMA_FLAG);
            
        #else
            
            while (READ_FLAG == 1) {
                // sprintf(buff, "READ_FLAG:%d\n", READ_FLAG);
                // quick_print(buff);

                // LED_On(LED_INDEX);
                // MXC_Delay(usec_delay/3);
                // LED_Off(LED_INDEX);
                // MXC_Delay(usec_delay/3);
            }
            
            if (READ_FLAG != E_NO_ERROR) {
                // printf("-->Error with UART_ReadAsync callback; %d\n", READ_FLAG);
                sprintf(buff, "READ ERR: %d\n", READ_FLAG);
                quick_print(buff, 32);
                fail++;
            } else {
                char print_rx_buff[7];
                sprintf(print_rx_buff, "Rx:%c%c%c%c", RxData[0],RxData[1],RxData[2],RxData[3]);
                quick_print(print_rx_buff, 7);
            }

            
        #endif
    #endif

        

        // MXC_Delay(usec_delay);

        // COMPARE RX TO TX
        // if ((error = memcmp(RxData, TxData, BUFF_SIZE)) != 0) {
        //     // printf("-->Error verifying Data: %d\n", error);
        //     sprintf(buff, "-->Error verifying Data: %d\n", error);
        //     quick_print(buff);
        //     fail++;
        // }
        // else {
        //     // printf("-->Data verified\n");
        //     sprintf(buff, "-->Data verified\n");
        //     quick_print(buff);
        // }
        
        // // printf("\n");
        
        // if (fail == 0) {
        //     LED_On(LED1);
        //     // printf("-->EXAMPLE SUCCEEDED\n");
        //     sprintf(buff, "-->EXAMPLE SUCCEEDED\n");
        //     quick_print(buff);
        // }
        // else {
        //     // printf("-->EXAMPLE FAILED\n");
        //     sprintf(buff, "-->EXAMPLE FAILED\n");
        //     quick_print(buff);
        // }

        

        

        // LOOP 
        // while (1) {
        //     printf("end loop\n");
        //     // UART
        
        //     // TFT  
        //     // #ifdef TFT_ENABLE
        //     // MXC_TFT_ClearScreen();
        //     // memset(buff,32, TFT_BUFF_SIZE);
        //     // // sprintf(buff, "G0 X0 Y300 Z%d",z);
        //     // sprintf(buff, "M1112");
        //     // TFT_Print(buff, 0, 10, (int)&Arial24x23[0]);
        //     // #endif
        //     // printf("%s\0",buff);
        //     // printf("%s\r\n",buff);
            
        //     z = 50 - z;

        //     // printf("M1112");
        //     MXC_Delay(usec_delay);
            

            LED_Toggle(LED_INDEX);
            
            
        //     // MXC_Delay(usec_delay);
        //     // LED_On(LED_INDEX);
            
    }
}
