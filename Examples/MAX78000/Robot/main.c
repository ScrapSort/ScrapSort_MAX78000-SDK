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
#include "dexarm.h"

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

    while (1) {
    }
}
