/*******************************************************************************
* Copyright (C) Maxim Integrated Products, Inc., All rights Reserved.
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

// simplesort
// Created using ./ai8xize.py --verbose --log --test-dir sdk/Examples/MAX78000/CNN --prefix simplesort --no-bias 0,1,2 --checkpoint-file trained/simplesort_qat-q.pth.tar --config-file networks/simplesorting.yaml --device MAX78000 --softmax --compact-data --mexpress --timer 0 --fifo --display-checkpoint

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "mxc.h"
#include "cnn.h"
#include "sampledata.h"
#include "sampleoutput.h"
#include "camera.h"
#include "camera_tft_funcs.h"
#include "tft_fthr.h"

volatile uint32_t cnn_time; // Stopwatch
#define TFT_BUFF_SIZE   50    // TFT buffer size
#define CAMERA_FREQ   (10 * 1000 * 1000)
int font_1 = (int)&Arial12x12[0];

/***** Globals *****/
// data is 128 x 128 px = 16,384 px each word is 0RGB, byte for each
uint32_t cnn_buffer[16384];

// buffer for touch screen text
char buff[TFT_BUFF_SIZE];

void load_input(void)
{
  // This function loads the sample data input -- replace with actual data

  int i;
  //const uint32_t *in0 = input_0;
  const uint32_t *in0 = cnn_buffer;

  for (i = 0; i < 16384; i++) {
    while (((*((volatile uint32_t *) 0x50000004) & 1)) != 0); // Wait for FIFO 0
    *((volatile uint32_t *) 0x50000008) = *in0++; // Write FIFO 0
  }
}


// Classification layer:
static int32_t ml_data[CNN_NUM_OUTPUTS];
static q15_t ml_softmax[CNN_NUM_OUTPUTS];

void softmax_layer(void)
{
  cnn_unload((uint32_t *) ml_data);
  softmax_q17p14_q15((const q31_t *) ml_data, CNN_NUM_OUTPUTS, ml_softmax);
}

int main(void)
{
  int i;
  int digs, tens;

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

  /* Set the screen rotation because camera flipped*/
	//MXC_TFT_SetRotation(ROTATE_180);

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

  // Enable peripheral, enable CNN interrupt, turn on CNN clock
  // CNN clock: 50 MHz div 1
  cnn_enable(MXC_S_GCR_PCLKDIV_CNNCLKSEL_PCLK, MXC_S_GCR_PCLKDIV_CNNCLKDIV_DIV1);

  printf("\n*** CNN Inference Test ***\n");

  cnn_init(); // Bring state machine into consistent state
  cnn_load_weights(); // Load kernels
  cnn_load_bias();
  cnn_configure(); // Configure state machine
  printf("\nready\n");

  area_t clear_word = {0, 70, 120, 20};
  char* class_names[] = {"CUP", "TRAPEZOID", "HEXAGON","CAN","BOTTLE","NONE"};

  while(true)
  {
    capture_camera_img();
    display_RGB565_img(56,96, cnn_buffer);

    // Enable CNN clock
    MXC_SYS_ClockEnable(MXC_SYS_PERIPH_CLOCK_CNN);

    cnn_init(); // Bring state machine into consistent state
    cnn_configure(); // Configure state machine

    cnn_start();
    load_input();
    

    while (cnn_time == 0)
    __WFI(); // Wait for CNN

    softmax_layer();

    cnn_stop();
    // Disable CNN clock to save power
    MXC_SYS_ClockDisable(MXC_SYS_PERIPH_CLOCK_CNN);

    #ifdef CNN_INFERENCE_TIMER
    printf("Approximate inference time: %u us\n\n", cnn_time);
    #endif

    printf("Classification results:\n");
    int max = 0;
    int max_i = 0;
    for (i = 0; i < CNN_NUM_OUTPUTS; i++) {
      digs = (1000 * ml_softmax[i] + 0x4000) >> 15;
      tens = digs % 10;
      digs = digs / 10;
      printf("[%7d] -> Class %d: %d.%d%%\n", ml_data[i], i, digs, tens);
      if(digs > max)
      {
          max = digs;
          max_i = i;
      }
    }

    memset(buff,32,TFT_BUFF_SIZE);
    MXC_TFT_FillRect(&clear_word, 4);
    TFT_Print(buff, 0, 0, font_1, sprintf(buff, "Class: %s", class_names[max_i]));
    printf("\033[0;0f");
  }

  return 0;
}

/*
  SUMMARY OF OPS
  Hardware: 59,151,432 ops (58,404,924 macc; 746,508 comp; 0 add; 0 mul; 0 bitwise)
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
    Layer 10: 12,300 ops (12,288 macc; 12 comp; 0 add; 0 mul; 0 bitwise)
    Layer 11: 60 ops (60 macc; 0 comp; 0 add; 0 mul; 0 bitwise)

  RESOURCE USAGE
  Weight memory: 159,444 bytes out of 442,368 bytes total (36%)
  Bias memory:   353 bytes out of 2,048 bytes total (17%)
*/

