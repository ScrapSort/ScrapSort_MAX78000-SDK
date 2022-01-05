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

/**
 * @file    camera_tft_funcs.c
 * @brief   camera and touch screen helper function
 * @details consolidate the Maxim Integrated provided functions into a separate file
 */

/***** Includes *****/
#include "camera_tft_funcs.h"
#include "tft_fthr.h"
#include "camera.h"

// variables
static int IMAGE_SIZE_X;
static int IMAGE_SIZE_Y;

mxc_gpio_cfg_t tft_reset_pin = {MXC_GPIO0, MXC_GPIO_PIN_19, MXC_GPIO_FUNC_OUT, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIOH};


/***** Camera Functions *****/

void set_image_dimensions(uint16_t x_dim, uint16_t y_dim)
{
    IMAGE_SIZE_X = x_dim;
    IMAGE_SIZE_Y = y_dim;
}


int get_image_x()
{
    return  IMAGE_SIZE_X;
}


int get_image_y()
{
    return  IMAGE_SIZE_Y;
}


void capture_camera_img(void) 
{
  // don't allow write image interrupt while starting the capture
  // otherwise we could set is_recv to 0 and then wait forever in the interrupt
  // because if we interrupt the while loop below, in the interrupt we still wait
  // till is_rcv then return here
  camera_start_capture_image();
  while (1) 
  {
    if (camera_is_image_rcv()) 
    {
      return;
    }
  }
}

void camera_stream_to_LCD(void)
{
  uint32_t imgLen;
  uint32_t width, height;
  uint8_t* data = NULL;
  uint8_t* data_ptr;

  camera_get_image(&data, &imgLen, &width, &height);
  camera_start_capture_image();

  for (int i = 0; i < height; i++) 
  {
    // Wait until camera streaming buffer is full
    while((data = get_camera_stream_buffer()) == NULL)
    {
      if(camera_is_image_rcv())
          break;
    };

    // data_ptr = data;
    // // Load image horizontal line
    // for (int j = 0; j < width; j++) 
    // {
    //   uint8_t ur, ug, ub;
    //   // Extract colors from RGB565 and convert to signed value
    //   ur = (*data_ptr & 0xF8) ^ 0x80; 
    //   ug = ((*data_ptr << 5) | ((*(data_ptr+1) & 0xE0) >> 3)) ^ 0x80;
    //   ub = (*(data_ptr+1) << 3) ^ 0x80;
    //   // Next pixel
    //   data_ptr += 2;
    //   // Wait for FIFO 0
    //   while (((*((volatile uint32_t*) 0x50000004) & 1)) != 0);  
    //   // Loading data into the CNN fifo in HWC format
    //   *((volatile uint32_t*) 0x50000008) = 0x00FFFFFF & ((ub << 16) | (ug << 8) | ur);
    // }
    
    // Optional: Draw one horizontal line of captured image on TFT
    // It increases latency and may require reducing image capture speed 
    MXC_TFT_ShowImageCameraRGB565(i, 1, data, width, 1);
      
    // Release streaming buffer
    release_camera_stream_buffer();
  }

  stream_stat_t* stat = get_camera_stream_statistic();
printf("DMA transfer count = %d", stat->dma_transfer_count);
// Check overflow condition
printf("OVERFLOW = %d", stat->overflow_count);
}


void process_RGB888_img(uint32_t *data0, uint32_t *data1, uint32_t *data2)
{
    uint8_t *frame_buffer;
    uint32_t imgLen;
    uint32_t w, h, x, y;
    uint8_t *ptr0;
    uint8_t *ptr1;
    uint8_t *ptr2;
    uint8_t *buffer;

    camera_get_image(&frame_buffer, &imgLen, &w, &h);
    ptr0 = (uint8_t *)data0;
    ptr1 = (uint8_t *)data1;
    ptr2 = (uint8_t *)data2;
    buffer = frame_buffer;
    for (y = 0; y < h; y++) 
    {
        for (x = 0; x < w; x++, ptr0++, ptr1++, ptr2++) 
        {
            *ptr0 = (*buffer); buffer++;
            *ptr1 = (*buffer); buffer++;
            *ptr2 = (*buffer); buffer++;
        }
    }
}


void display_RGB565_img(int x_coord, int y_coord,uint32_t* cnn_buffer)
{
	uint8_t   *raw;
	uint32_t  imgLen;
	uint32_t  w, h;

  uint8_t ur,ug,ub;
  int8_t r,g,b;

  // Get the details of the image from the camera driver.
	camera_get_image(&raw, &imgLen, &w, &h);
  printf("\nimg len: %i\n",imgLen);
  // iterate over all pixels
  for(int i = 0; i < w; i++) // rows
  {
    for(int j = 0; j < h; j++) // cols
    {
      // extract the RGB values
      ur = (raw[2*(w*i+j)] & 0b11111000);
      ug = ((((raw[2*(w*i+j)] & 0b00000111)<<5) | ((raw[2*(w*i+j)+1] & 0b11100000)>>3)));
      ub = (((raw[2*(w*i+j)+1] & 0b00011111))<<3);

      // r = (raw[2*(w*i+j)+1] & 0b11111000);
      // g = ((raw[2*(w*i+j)] & 0b11100000)>>3) | ((raw[2*(w*i+j)+1] & 0b00000111)<<5);
      // b = (raw[2*(w*i+j)] & 0b00011111)<<3;

      r = ur-128;
      g = ug-128;
      b = ub-128;

      // RGB565 normally:   RRRRRGGG GGGBBBBB --> 16 bits
      // in little endian:  GGGBBBBB RRRRRGGG --> bytes swapped

      // r = (raw[2*(w*i+j)+1] & 0b11111000)>>3;
      // g = ((raw[2*(w*i+j)] & 0b00000111)>>5) | ((raw[2*(w*i+j)+1] & 0b11100000)<<3);
      // b = raw[2*(w*i+j)] & 0b00011111;
      
      cnn_buffer[w*i+j] = 0x00FFFFFF & ((((uint8_t)b) << 16) | (((uint8_t)g) << 8) | ((uint8_t)r));
      //(((uint32_t)(r)) << 16) | (((uint32_t)(g)) << 8) | ((uint32_t)(b));
    }
  }

  printf("\nr: %i g: %i b: %i--\n",r,g,b);
  printf("\nhex:%8X\n",cnn_buffer[w*127+127]);
	MXC_TFT_ShowImageCameraRGB565(x_coord, y_coord, raw, h, w);
}

/***** LCD Functions *****/
void init_LCD()
{
  printf("Init LCD.\n");
  MXC_TFT_Init(MXC_SPI0, 1, &tft_reset_pin, NULL);
  MXC_TFT_ClearScreen();
}

void reset()
{
  MXC_TFT_Init(MXC_SPI0, 1, &tft_reset_pin, NULL);
  //MXC_TFT_SetRotation(ROTATE_180);
}


void display_RGB888_img(uint32_t *data0, uint32_t *data1, uint32_t *data2, int length, int x_coord, int y_coord) 
{
  int i;
  int j;
  int x;
  int y;
  int r;
  int g;
  int b;
  int scale = 2.2;
    
  uint32_t color;
  uint8_t *ptr0;
  uint8_t *ptr1;
  uint8_t *ptr2;

  x = x_coord;
  y = y_coord;

  // iterate over buffer length (1024 items, each 4 bytes)
  for (i = 0; i < length; i++) 
  {
    // RGB byte pointers point to start of 4 byte chunk
    ptr0 = (uint8_t *)&data0[i];
    ptr1 = (uint8_t *)&data1[i];
    ptr2 = (uint8_t *)&data2[i];

    // get the pixels from the 4 byte chunk
    for (j = 0; j < 4; j++) 
    {
      r = ptr0[j];
      g = ptr1[j];
      b = ptr2[j];        
      //color  = (0x01000100 | ((b & 0xF8) << 13) | ((g & 0x1C) << 19) | ((g & 0xE0) >> 5) | (r & 0xF8));
      color = RGB(r, g, b); // convert to RGB565
      MXC_TFT_WritePixel(x * scale, y * scale, scale, scale, color);
      x += 1;
      if (x >= (IMAGE_SIZE_X + x_coord)) 
      {
        x = x_coord;
        y += 1;
        if ((y + 6) >= (IMAGE_SIZE_Y + y_coord))
        { 
            return;
        }
      }
    }
  }
}


void TFT_Print(char *str, int x, int y, int font, int length) 
{
  // fonts id
  text_t text;
  text.data = str;
  text.len = 36;

  MXC_TFT_PrintFont(x, y, font, &text, NULL);
}


