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
#define EV
/***** Includes *****/
#include "camera_tft_funcs.h"
#ifdef EV
#include "tft_fthr.h"
#endif
#include "camera.h"

// variables
static int IMAGE_SIZE_X;
static int IMAGE_SIZE_Y;

#ifdef EV
mxc_gpio_cfg_t tft_reset_pin = {MXC_GPIO0, MXC_GPIO_PIN_19, MXC_GPIO_FUNC_OUT, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIOH};
#endif
static const uint8_t camera_settings[][2] = {
    {0x0e, 0x08}, // Sleep mode
    {0x69, 0x52}, // BLC window selection, BLC enable (default is 0x12)
    {0x1e, 0xb3}, // AddLT1F (default 0xb1)
    {0x48, 0x42},
    {0xff, 0x01}, // Select MIPI register bank
    {0xb5, 0x30},
    {0xff, 0x00}, // Select system control register bank
    {0x16, 0x03}, // (default)
    {0x62, 0x10}, // (default)
    {0x12, 0x01}, // Select Bayer RAW
    {0x17, 0x65}, // Horizontal Window Start Point Control (LSBs), default is 0x69
    {0x18, 0xa4}, // Horizontal sensor size (default)
    {0x19, 0x0c}, // Vertical Window Start Line Control (default)
    {0x1a, 0xf6}, // Vertical sensor size (default)
    {0x37, 0x04}, // PCLK is double system clock (default is 0x0c)
    {0x3e, 0x20}, // (default)
    {0x81, 0x3f}, // sde_en, uv_adj_en, scale_v_en, scale_h_en, uv_avg_en, cmx_en
    {0xcc, 0x02}, // High 2 bits of horizontal output size (default)
    {0xcd, 0x80}, // Low 8 bits of horizontal output size (default)
    {0xce, 0x01}, // Ninth bit of vertical output size (default)
    {0xcf, 0xe0}, // Low 8 bits of vertical output size (default)
    {0x82, 0x01}, // 01: Raw from CIP (default is 0x00)
    {0xc8, 0x02},
    {0xc9, 0x80},
    {0xca, 0x01},
    {0xcb, 0xe0},
    {0xd0, 0x28},
    {0x0e, 0x00}, // Normal mode (not sleep mode)
    {0x70, 0x00},
    {0x71, 0x34},
    {0x74, 0x28},
    {0x75, 0x98},
    {0x76, 0x00},
    {0x77, 0x64},
    {0x78, 0x01},
    {0x79, 0xc2},
    {0x7a, 0x4e},
    {0x7b, 0x1f},
    {0x7c, 0x00},
    {0x11, 0x01}, // CLKRC, Internal clock pre-scalar divide by 2 (default divide by 1)
    {0x20, 0x00}, // Banding filter (default)
    {0x21, 0x57}, // Banding filter (default is 0x44)
    {0x50, 0x4d},
    {0x51, 0x40}, // 60Hz Banding AEC 8 bits (default 0x80)
    {0x4c, 0x7d},
    {0x0e, 0x00},
    {0x80, 0x7f},
    {0x85, 0x00},
    {0x86, 0x00},
    {0x87, 0x00},
    {0x88, 0x00},
    {0x89, 0x2a},
    {0x8a, 0x22},
    {0x8b, 0x20},
    {0xbb, 0xab},
    {0xbc, 0x84},
    {0xbd, 0x27},
    {0xbe, 0x0e},
    {0xbf, 0xb8},
    {0xc0, 0xc5},
    {0xc1, 0x1e},
    {0xb7, 0x05},
    {0xb8, 0x09},
    {0xb9, 0x00},
    {0xba, 0x18},
    {0x5a, 0x1f},
    {0x5b, 0x9f},
    {0x5c, 0x69},
    {0x5d, 0x42},
    {0x24, 0x78}, // AGC/AEC
    {0x25, 0x68}, // AGC/AEC
    {0x26, 0xb3}, // AGC/AEC
    {0xa3, 0x0b},
    {0xa4, 0x15},
    {0xa5, 0x29},
    {0xa6, 0x4a},
    {0xa7, 0x58},
    {0xa8, 0x65},
    {0xa9, 0x70},
    {0xaa, 0x7b},
    {0xab, 0x85},
    {0xac, 0x8e},
    {0xad, 0xa0},
    {0xae, 0xb0},
    {0xaf, 0xcb},
    {0xb0, 0xe1},
    {0xb1, 0xf1},
    {0xb2, 0x14},
    {0x8e, 0x92},
    {0x96, 0xff},
    {0x97, 0x00},
    {0x14, 0x3b}, 	// AGC value, manual, set banding (default is 0x30)
    {0x0e, 0x00},
    {0x0c, 0xd6},
    {0x82, 0x3},
    {0x11, 0x00},  	// Set clock prescaler
    {0x12, 0x6},
    {0x61, 0x0},
    {0x64, 0x11},
    {0xc3, 0x80},
    {0x81, 0x3f},
    {0x16, 0x3},
    {0x37, 0xc},
    {0x3e, 0x20},
    {0x5e, 0x0},
    {0xc4, 0x1},
    {0xc5, 0x80},
    {0xc6, 0x1},
    {0xc7, 0x80},
    {0xc8, 0x2},
    {0xc9, 0x80},
    {0xca, 0x1},
    {0xcb, 0xe0},
    {0xcc, 0x0},
    {0xcd, 0x40}, 	// Default to 64 line width
    {0xce, 0x0},
    {0xcf, 0x40}, 	// Default to 64 lines high
    {0x1c, 0x7f},
    {0x1d, 0xa2},
	  {0xee, 0xee}  // End of register list marker 0xee
};

/***** Camera Functions *****/

void set_image_dimensions(uint16_t x_dim, uint16_t y_dim)
{
    IMAGE_SIZE_X = x_dim;
    IMAGE_SIZE_Y = y_dim;

    // set camera registers with default values
    for (int i = 0; (camera_settings[i][0] != 0xee); i++) 
    {
      camera_write_reg(camera_settings[i][0], camera_settings[i][1]);
    }
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
  camera_start_capture_image();
  while (1) 
  {
    if (camera_is_image_rcv()) 
    {
      return;
    }
  }
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


void display_RGB565_img(int x_coord, int y_coord)
{
	uint8_t   *raw;
	uint32_t  imgLen;
	uint32_t  w, h;

  // Get the details of the image from the camera driver.
	camera_get_image(&raw, &imgLen, &w, &h);
  #ifdef EV
	MXC_TFT_ShowImageCameraRGB565(x_coord, y_coord, raw, h, w);
  #endif
}

/***** LCD Functions *****/
#ifdef EV
void init_LCD()
{
  printf("Init LCD.\n");
  MXC_TFT_Init(MXC_SPI0, 1, &tft_reset_pin, NULL);
  MXC_TFT_ClearScreen();
}
#endif

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
      color  = (0x01000100 | ((b & 0xF8) << 13) | ((g & 0x1C) << 19) | ((g & 0xE0) >> 5) | (r & 0xF8));
      #ifdef EV
      MXC_TFT_WritePixel(x * scale, y * scale, scale, scale, color);
      #endif
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


void display_grayscale_img(int x_coord, int y_coord, int8_t* cnn_buffer)
{
  uint8_t   *raw; // pointer to raw frame buffer
	uint32_t  imgLen; // number of bytes, not pixels
	uint32_t  w, h; // width and height in pixels

  // Get the details of the image from the camera driver.
	camera_get_image(&raw, &imgLen, &w, &h);

  // details for converting from YUV422 to RGB565 in gray scale
  /*
    YUV422 format:

    2 px 2px
    UYVY UYVY ...   1px = 2byte, each Y, U, V is 1 byte

    Y = luminance (grayscale), lower 8 bits --> Y = YUV_PX & 0x00FF

    RGB to grayscale means R=G=B, in RGB565 2R=G=2B because G has an extra bit
    R = B = Y & 0xF8 (5 bit) G = Y & FC (6 bit)
    
    the camera returns each 2 byte pixel in little endian, the tft lib function accounts for this already
    so need to place bits in little endian format

    RGB565 normally:   RRRRRGGG GGGBBBBB --> 16 bits
    in little endian:  GGGBBBBB RRRRRGGG --> bytes swapped
  */
  
  #define RED_PX 0x00F8;
  #define GREEN_PX 0xE003;
  #define BLUE_PX 0x1F00;

  // iterate over all pixels
  for(int i = 0; i < w; i++) // rows
  {
    for(int j = 0; j < h; j++) // cols
    {
      // extract luminance from the YUV pixel which is 16 bits
      uint8_t Y = (((uint16_t*)raw)[w*i+j] & 0x00FF);
      
      // binary threshold, send pixels to blakc or white
      if(Y > 70)
      {
        Y = 255;
      }
      else
      {
        Y = 0;
      }

      Y = 255 - Y;
      
      // represent luminance using RGB565
      uint16_t R = (Y & 0x00F8);
      uint16_t G = (Y & 0x00FC);
      G = (((G & 0xE0) >> 5) | ((G & 0x1C) << 11));
      uint16_t B = ((Y & 0x00F8) << 5);
      ((uint16_t*)raw)[w*i+j] = (R | G | B); // edit the raw frame buffer to grayscale, no downsampling

      // decimate by getting pixel from every other row/col (even indices)
      if(((i&1) == 0) && ((j&1) == 0))
      {
        // store the pixel to the CNN buffer, make sure to divide the width by 2
        // also divide the indices by two because resulting image has 1/2 the dimensions
        cnn_buffer[(w>>1)*(i>>1)+(j>>1)] = Y-128; // convert to signed

        // col is divisible by 8, we only get every other index so we have extracted 4 bytes
        if((j&7) == 0)
        {
          //printf("%08X ", ((uint32_t*)cnn_buffer)[(w>>1)*(i>>3)+(j>>3)]);
        }
        // uncomment this to visualize downsampling
        //((uint16_t*)raw)[(w>>1)*(i>>1)+(j>>1)] = (R | G | B);
      }
    }
    // only even columns
    if(((i&1) == 0))
    {
      //printf("\n");
    }
  }
  //printf("\033[0;0f"); // escape sequence to move cursor to top left corner, keeps stdout fixed
  
  // display the image
  //MXC_TFT_ShowImageCameraRGB565(x_coord, y_coord, raw, h, w);
  #ifdef EV
  MXC_TFT_ShowImageCameraRGB565(x_coord, y_coord, raw, h, w);
  #endif
}

#ifdef EV
void TFT_Print(char *str, int x, int y, int font, int length) 
{
  // fonts id
  text_t text;
  text.data = str;
  text.len = 36;

  MXC_TFT_PrintFont(x, y, font, &text, NULL);
}
#endif


