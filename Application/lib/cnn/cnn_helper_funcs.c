// ========================================================================================= //
// ======================================= HEADERS ========================================= //
// ========================================================================================= //

#include "cnn_helper_funcs.h"
#include "mxc_delay.h"
#include "mxc_device.h"
#include "mxc_sys.h"
#include "bbfc_regs.h"
#include "fcr_regs.h"
#include "cnn.h"
#include "camera_tft_funcs.h"
#include "tft_fthr.h"
#include "mxc.h"
#include "camera.h"
#include "camera_tft_funcs.h"
#include "string.h"


// ========================================================================================= //
// ======================================= MACROS ========================================== //
// ========================================================================================= //

#define SCREEN_W 128 // image output width
#define SCREEN_H 128 // image output height
#define SCREEN_X 56 // image output top left corner
#define SCREEN_Y 140 // image output top left corner
#define TFT_BUFF_SIZE   50 // TFT text buffer size
#define NUM_CLASSES 4 // number of output classes
#define NUM_OUTPUTS 4 // number of output neurons
#define CNN_INPUT_SIZE 16384 // data is 128 x 128 px = 16,384 px each word is 0RGB, byte for each

#define TFT_BUFF_SIZE   50    // TFT buffer size
#define BB_COLOR YELLOW // the bounding box color
#define BB_W 2 // the bounding box width in pixels

// ========================================================================================= //
// ================================== GLOBAL VARIABLES ===================================== //
// ========================================================================================= //

uint32_t cnn_buffer[CNN_INPUT_SIZE]; // the input image data into the CNN (80*80 = 6400 bytes = 1600 words)
//volatile uint32_t cnn_time; // Stopwatch to time forward pass, not used at the moment
cnn_output_t output; // the output data of the CNN

static int32_t ml_data[NUM_OUTPUTS]; // output data
static q15_t ml_softmax[NUM_CLASSES]; // softmax output data

output_classes_t class_names[] = {PLASTIC, PAPER, NONE, METAL};


char buff[TFT_BUFF_SIZE];
int font_1 = (int)&Arial12x12[0];
volatile uint32_t cnn_time; // Stopwatch
char* class_strings[] = {"PLASTIC","PAPER","NONE","METAL"};

// ========================================================================================= //
// ================================ FUNCTION DEFINITIONS =================================== //
// ========================================================================================= //

// this function loads the image data into the input layer's data memory instance
void load_input(void)
{
  int i;
  const uint32_t *in0 = cnn_buffer;

  for (i = 0; i < 16384; i++) {
    while (((*((volatile uint32_t *) 0x50000004) & 1)) != 0); // Wait for FIFO 0
    *((volatile uint32_t *) 0x50000008) = *in0++; // Write FIFO 0
  }
}


// ========================================================================================= //


// this function gets the classification data from the output layer
// data memory instance and passes it to the auto-generated softmax function
void softmax_layer(void)
{
  cnn_unload((uint32_t *) ml_data);
  softmax_q17p14_q15((const q31_t *) ml_data, NUM_CLASSES, ml_softmax);
}


// ========================================================================================= //


// simple getter function for the CNN input data buffer
uint32_t* get_cnn_buffer()
{
    return cnn_buffer;
}


// ========================================================================================= //


// this function does a forward pass through the CNN 
cnn_output_t* run_cnn()
{
    int digs, tens; // format probability
    int max = 0; // the current highest class probability
    int max_i = 0; // the class with the highest probability
    
    // cnn output
    static cnn_output_t output;

    // first get an image from the camera and load it into the CNN buffer
    capture_camera_img();
    display_RGB565_img(SCREEN_X,SCREEN_Y, cnn_buffer,true);

    // Enable CNN clock
    MXC_SYS_ClockEnable(MXC_SYS_PERIPH_CLOCK_CNN);

    cnn_init(); // Bring state machine into consistent state
    cnn_configure(); // Configure state machine

    cnn_start();
    load_input();
        
    while (cnn_time == 0)
    __WFI(); // Wait for CNN

    // classify the output
    softmax_layer();

    //printf("Classification results:\n");

    for (int i = 0; i < NUM_CLASSES; i++) 
    {
      // softmax output is in fixed point so need to convert to percentage
      digs = (1000 * ml_softmax[i] + 0x4000) >> 15;
      tens = digs % 10; // get the fractional part
      digs = digs / 10; // get the integer part
      printf("[%7d] -> Class %d: %d.%d%%\n", ml_data[i], i, digs, tens);
      // keep track of the max class
      if(digs > max)
      {
          max = digs;
          max_i = i;
      }
    }
    //printf("BB coords:\n");
    // output.x = ml_data[6]/29000;
    // output.y = ml_data[7]/29000;
    // output.w = ml_data[8]/33000;
    // output.h = ml_data[9]/33000;
    //printf("x: %i\ny: %i\nw: %i\nh: %i\n",output.x,output.y,output.w,output.h);
    //printf("\033[0;0f");
    // Disable CNN clock to save power
    cnn_stop();
    MXC_SYS_ClockDisable(MXC_SYS_PERIPH_CLOCK_CNN);

#ifdef CNN_INFERENCE_TIMER
    printf("Approximate inference time: %u us\n\n", cnn_time);
#endif
    
    // cnn output class
    output.output_class = class_names[max_i];
    return &output;
}


// ========================================================================================= //


void startup_cnn()
{
    // Enable peripheral, enable CNN interrupt, turn on CNN clock
    // CNN clock: 50 MHz div 1
    cnn_enable(MXC_S_GCR_PCLKDIV_CNNCLKSEL_PCLK, MXC_S_GCR_PCLKDIV_CNNCLKDIV_DIV1);

    cnn_init(); // Bring state machine into consistent state
    cnn_load_weights(); // Load kernels
    cnn_load_bias();
    cnn_configure(); // Configure state machine
}


// ========================================================================================= //


void show_cnn_output(cnn_output_t output)
{
  static area_t top = {56, 140, 4, 4};
  static area_t left = {56, 140, 4, 4};
  static area_t bottom = {56, 140, 4, 4};
  static area_t right = {56, 140, 4, 4};

  memset(buff,32,TFT_BUFF_SIZE);
  
  TFT_Print(buff, 0, 0, font_1, sprintf(buff, "Class: %s", class_strings[output.output_class]));
  
  // if(output.output_class != 5)
  // {
  //     // bounding box
  //     top.x = (output.x >= 0 && output.x < SCREEN_W) ? output.x : 0;
  //     top.y = (output.y >= 0 && output.y < SCREEN_H) ? output.y : 0;
  //     top.w = (top.x+output.w) < SCREEN_W ? output.w : SCREEN_W-top.x-1;
  //     top.h = BB_W;

  //     // Format the bottom side of the bounding box
  //     // The bottom should not go off of the screen
  //     bottom.x = top.x;
  //     bottom.y = (output.y+output.h-BB_W) < SCREEN_H-BB_W ? output.y+output.h-BB_W : SCREEN_H-BB_W-1;
  //     bottom.w = top.w;
  //     bottom.h = BB_W;

  //     // Format the left side of the bounding box
  //     // The left should not go off of the screen
  //     left.x = top.x;
  //     left.y = top.y;
  //     left.w = BB_W;
  //     left.h = (left.y+output.h) < SCREEN_H ? output.h : SCREEN_H-left.y-1;

  //     // Format the right side of the bounding box
  //     // The right should not go off of the screen
  //     right.x = (output.x+output.w) < SCREEN_W-BB_W ? (output.x+output.w): SCREEN_W-BB_W-1;
  //     right.y = top.y;
  //     right.w = BB_W;
  //     right.h = left.h;

  //     // shift the box y-coordinates to the screen Y position
  //     top.y += SCREEN_Y;
  //     bottom.y += SCREEN_Y;
  //     left.y += SCREEN_Y;
  //     right.y += SCREEN_Y;

  //     // flip the box over horizontally and shift it to the screen X position
  //     top.x += SCREEN_X;
  //     bottom.x += SCREEN_X;
  //     left.x += SCREEN_X;
  //     right.x += SCREEN_X;

  //     // draw the box
  //     // MXC_TFT_FillRect(&top, BB_COLOR);
  //     // MXC_TFT_FillRect(&bottom, BB_COLOR);
  //     // MXC_TFT_FillRect(&left, BB_COLOR);
  //     // MXC_TFT_FillRect(&right, BB_COLOR);
  //   }
    // printf("\033[0;0f");
}


// ========================================================================================= //