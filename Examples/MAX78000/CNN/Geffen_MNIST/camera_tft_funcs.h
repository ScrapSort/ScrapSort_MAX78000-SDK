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

/*
    This file contains helper functions for using the camera and LCD (tft = LCD)
    peripherals. The code itself is mainly taken from the given examples
    from Maxim Integrated but reorganizes and explains how it works.

    Other useful files:
    Libraries/Boards/MAX78000/Source/Camera.c
    Libraries/Boards/MAX78000/Source/ov7692.c
    Libraries/Boards/MAX78000/Source/tft.c

    Other useful functions:
    tft.c/MXC_TFT_ShowImageCameraRGB565()
    camera.c/camera_get_image()

    Note:
    -the camera is flipped so you can rotate the LCD display, however
     this will ruin how text and images are displayed so may need to set normal
    -if using CNN then need to write to the intermediate fifos
*/

#ifndef CAMERA_TFT_FUNCS
#define CAMERA_TFT_FUNCS

/***** Includes *****/
#include <stdint.h>

/***** Camera Functions *****/

/*
    Description: This function sets the image dimensions to get from the
                 camera. The max is (X, Y) = (319, 239). Beyond, (200, 150)
                 there is noticeable update latency. This function also sets
                 the camera registers with the default values.

    Parameters: The desired X and Y dimensions

    Return: none
*/
void set_image_dimensions(uint16_t x_dim, uint16_t y_dim);


/*
    Description: get the x dimension of the image

    Parameters: none

    Return: x dimension of the image
*/
int get_image_x();


/*
    Description: get the y dimension of the image

    Parameters: none

    Return: y dimension of the image
*/
int get_image_y();


/*
    Description: This is a blocking function that starts and image
                 capture and only returns once the camera has
                 captured the image

    Parameters: none

    Return: none
*/
void capture_camera_img(void);


/*
    Description: This function is called once the camera has captured
                 an image. It gets a pointer the raw frame buffer
                 and extracts and writes the RGB components to seperate
                 RGB buffers that will be used to display to the LCD. These
                 buffer are also the CNN data buffers. RGB888 means that
                 each color channel is 8 bits so each pixel is 24 bits.

    Parameters: RGB buffers to fill, these will be used to display to the LCD

    Return: none
*/
void process_RGB888_img(uint32_t *r_buffer, uint32_t *g_buffer, uint32_t *b_buffer);



/***** LCD Functions *****/

/*
    Description: This function initializes the LCD and clears the screen

    Parameters: None

    Return: none
*/
void init_LCD();

/*
    Description: This function writes the RGB buffers to the LCD using
                 MXC_TFT_WritePixel().

    Parameters: The RGB buffers, length of the buffers, and the location of the image (top left corner)

    Return: none
*/
void display_RGB888_img(uint32_t *r_buff, uint32_t *g_buff, uint32_t *b_buff, int length, int x_coord, int y_coord);

/*
    Description: This function is called once the camera has captured
                 an image. It gets a pointer the raw frame buffer
                 and passes it to MXC_TFT_ShowImageCameraRGB565() which
                 writes the image data to the LCD display using RGB565. RGB565
                 means that the RGB channels are 5, 6, and 5 bits accordingly.
                 Overall this is an even 16 bits (2 bytes). This method is much
                 faster than RGB888, use this for continuous streaming.

    Parameters: The location to display the image on the LCD (top left corner)

    Return: none
*/
void display_RGB565_img(int x_coord, int y_coord);

/*
    Description: This function is called once the camera has captured
                 an image. It gets a pointer the raw frame buffer
                 and ...

    Parameters: The location to display the image on the LCD (top left corner)

    Return: none
*/
void display_grayscale_img(int x_coord, int y_coord, int8_t* cnn_buffer);


/*
    Description: This function writes text to the LCD

    Parameters: Pointer to the text buffer, location, and font

    Return: none
*/
void TFT_Print(char *str, int x, int y, int font, int length);

#endif