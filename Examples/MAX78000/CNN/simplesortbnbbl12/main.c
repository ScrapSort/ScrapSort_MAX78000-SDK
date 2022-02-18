/*******************************************************************************
* Copyright (C) 2019-2021 Maxim Integrated Products, Inc., All rights Reserved.
*
* This software is protected by copyright laws of the United States and
* of foreign countries. This material may also be protected by patent laws
* and technology transfer regulations of the United States and of foreign
* countries. This software is furnished under a license agreement and/or a
* nondisclosure agreement and may only be used or reproduced in accordance
* with the terms of those agreements. Dissemination of this information to
* any party or parties not specified in the license agreement and/or
* nondisclosure agreement is expressly prohibited.
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
*******************************************************************************/

// simplesortbnbbl12
// Created using ./ai8xize.py --verbose --log --test-dir sdk/Examples/MAX78000/CNN --prefix simplesortbnbbl12 --no-bias 0,1,2 --checkpoint-file trained/sort_qat_bb_l12-q.pth.tar --config-file networks/simplesortingbb.yaml --device MAX78000 --softmax --compact-data --mexpress --timer 0 --fifo --display-checkpoint

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "mxc.h"
#include "cnn.h"
#include "camera.h"
#include "tft_fthr.h"
#include "camera_tft_funcs.h"
#include "cnn_helper_funcs.h"


/***** Macros *****/
#define TFT_BUFF_SIZE   50    // TFT buffer size
#define CAMERA_FREQ   (10 * 1000 * 1000)
#define BB_COLOR YELLOW // the bounding box color
#define BB_W 2 // the bounding box width in pixels
#define SCREEN_W 128 // image output width
#define SCREEN_H 128 // image output height
#define SCREEN_X 56 // image output top left corner
#define SCREEN_Y 140 // image output top left corner


/***** Globals *****/

// buffer for touch screen text
char buff[TFT_BUFF_SIZE];
int font_1 = (int)&Arial12x12[0];
volatile uint32_t cnn_time; // Stopwatch
char* class_strings[] = {"CUP","HEXAGON","TRAPEZOID","CAN","BOTTLE","NONE"};

int main(void)
{

  MXC_ICC_Enable(MXC_ICC0); // Enable cache

  // Switch to 100 MHz clock
  MXC_SYS_Clock_Select(MXC_SYS_CLOCK_IPO);
  SystemCoreClockUpdate();

  printf("Waiting...\n");

  // DO NOT DELETE THIS LINE:
  MXC_Delay(SEC(2)); // Let debugger interrupt if needed


  // Initialize DMA for camera interface
	MXC_DMA_Init();
	int dma_channel = MXC_DMA_AcquireChannel();

  // Initialize TFT display.
  init_LCD();
  MXC_TFT_ClearScreen();

  // Initialize camera.
  printf("Init Camera.\n");
  camera_init(CAMERA_FREQ);
  
  set_image_dimensions(128, 128);

  // Setup the camera image dimensions, pixel format and data acquiring details.
  // four bytes because each pixel is 2 bytes, can get 2 pixels at a time
	int ret = camera_setup(get_image_x(), get_image_y(), PIXFORMAT_RGB565, FIFO_FOUR_BYTE, USE_DMA, dma_channel);
  if (ret != STATUS_OK) 
  {
		printf("Error returned from setting up camera. Error %d\n", ret);
		return -1;
	}
  
  MXC_TFT_SetBackGroundColor(4);
  MXC_TFT_SetForeGroundColor(YELLOW);

  // init the CNN accelerator
  startup_cnn();

  cnn_output_t output;
  init_trigger();
  static area_t top = {56, 140, 4, 4};
  static area_t left = {56, 140, 4, 4};
  static area_t bottom = {56, 140, 4, 4};
  static area_t right = {56, 140, 4, 4};
  while(true)
  {
    // display inference time
    #ifdef CNN_INFERENCE_TIMER
    //printf("Approximate inference time: %u us\n\n", cnn_time);
    //memset(buff,32,TFT_BUFF_SIZE);
    //TFT_Print(buff, 0, 280, font_1, sprintf(buff, "Inference time: %u us", cnn_time));
    #endif

    //if(trigger_check() == 1)
    if(true)
    {
      // do a forward pass
      output = *run_cnn();
      memset(buff,32,TFT_BUFF_SIZE);
      
      TFT_Print(buff, 0, 0, font_1, sprintf(buff, "Class: %s", class_strings[output.output_class]));
      
      if(output.output_class != 5)
      {
        // bounding box
        top.x = (output.x >= 0 && output.x < SCREEN_W) ? output.x : 0;
        top.y = (output.y >= 0 && output.y < SCREEN_H) ? output.y : 0;
        top.w = (top.x+output.w) < SCREEN_W ? output.w : SCREEN_W-top.x-1;
        top.h = BB_W;

        // Format the bottom side of the bounding box
        // The bottom should not go off of the screen
        bottom.x = top.x;
        bottom.y = (output.y+output.h-BB_W) < SCREEN_H-BB_W ? output.y+output.h-BB_W : SCREEN_H-BB_W-1;
        bottom.w = top.w;
        bottom.h = BB_W;

        // Format the left side of the bounding box
        // The left should not go off of the screen
        left.x = top.x;
        left.y = top.y;
        left.w = BB_W;
        left.h = (left.y+output.h) < SCREEN_H ? output.h : SCREEN_H-left.y-1;

        // Format the right side of the bounding box
        // The right should not go off of the screen
        right.x = (output.x+output.w) < SCREEN_W-BB_W ? (output.x+output.w): SCREEN_W-BB_W-1;
        right.y = top.y;
        right.w = BB_W;
        right.h = left.h;

        // shift the box y-coordinates to the screen Y position
        top.y += SCREEN_Y;
        bottom.y += SCREEN_Y;
        left.y += SCREEN_Y;
        right.y += SCREEN_Y;

        // flip the box over horizontally and shift it to the screen X position
        top.x += SCREEN_X;
        bottom.x += SCREEN_X;
        left.x += SCREEN_X;
        right.x += SCREEN_X;

        // draw the box
        MXC_TFT_FillRect(&top, BB_COLOR);
        MXC_TFT_FillRect(&bottom, BB_COLOR);
        MXC_TFT_FillRect(&left, BB_COLOR);
        MXC_TFT_FillRect(&right, BB_COLOR);
      }
      printf("\033[0;0f");
    }
  }

  return 0;
}

/*
  SUMMARY OF OPS
  Hardware: 59,149,312 ops (58,402,816 macc; 746,496 comp; 0 add; 0 mul; 0 bitwise)
    Layer 0: 3,670,016 ops (3,538,944 macc; 131,072 comp; 0 add; 0 mul; 0 bitwise)
    Layer 1: 9,568,256 ops (9,437,184 macc; 131,072 comp; 0 add; 0 mul; 0 bitwise)
    Layer 2: 4,915,200 ops (4,718,592 macc; 196,608 comp; 0 add; 0 mul; 0 bitwise)
    Layer 3: 9,502,720 ops (9,437,184 macc; 65,536 comp; 0 add; 0 mul; 0 bitwise)
    Layer 4: 4,816,896 ops (4,718,592 macc; 98,304 comp; 0 add; 0 mul; 0 bitwise)
    Layer 5: 9,469,952 ops (9,437,184 macc; 32,768 comp; 0 add; 0 mul; 0 bitwise)
    Layer 6: 4,767,744 ops (4,718,592 macc; 49,152 comp; 0 add; 0 mul; 0 bitwise)
    Layer 7: 9,453,568 ops (9,437,184 macc; 16,384 comp; 0 add; 0 mul; 0 bitwise)
    Layer 8: 2,379,776 ops (2,359,296 macc; 20,480 comp; 0 add; 0 mul; 0 bitwise)
    Layer 9: 594,944 ops (589,824 macc; 5,120 comp; 0 add; 0 mul; 0 bitwise)
    Layer 10: 10,240 ops (10,240 macc; 0 comp; 0 add; 0 mul; 0 bitwise)

  RESOURCE USAGE
  Weight memory: 157,336 bytes out of 442,368 bytes total (36%)
  Bias memory:   336 bytes out of 2,048 bytes total (16%)
*/

