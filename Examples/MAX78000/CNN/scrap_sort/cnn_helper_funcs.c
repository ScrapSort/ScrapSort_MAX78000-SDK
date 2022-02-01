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


// ========================================================================================= //
// ======================================= MACROS ========================================== //
// ========================================================================================= //

#define SCREEN_W 128 // image output width
#define SCREEN_H 128 // image output height
#define SCREEN_X 56 // image output top left corner
#define SCREEN_Y 140 // image output top left corner
#define TFT_BUFF_SIZE   50 // TFT text buffer size
#define NUM_CLASSES 6 // number of output classes
#define CNN_INPUT_SIZE 16384 // data is 128 x 128 px = 16,384 px each word is 0RGB, byte for each

// ========================================================================================= //
// ================================== GLOBAL VARIABLES ===================================== //
// ========================================================================================= //

uint32_t cnn_buffer[CNN_INPUT_SIZE]; // the input image data into the CNN (80*80 = 6400 bytes = 1600 words)
volatile uint32_t cnn_time; // Stopwatch to time forward pass, not used at the moment
cnn_output_t output; // the output data of the CNN

static int32_t ml_data[NUM_CLASSES]; // classification output data
static q15_t ml_softmax[NUM_CLASSES]; // softmax output data

// buffer for touch screen text
char buff[TFT_BUFF_SIZE];

// font id
int font_1 = (int)&Arial12x12[0];

char* class_names[] = {"CUP", "TRAPEZOID", "HEXAGON","CAN","BOTTLE","NONE"};

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
  softmax_q17p14_q15((const q31_t *) ml_data, CNN_NUM_OUTPUTS, ml_softmax);
}


// ========================================================================================= //


// simple getter function for the CNN input data buffer
uint32_t* get_cnn_buffer()
{
    return cnn_buffer;
}


// ========================================================================================= //


// this function does a forward pass through the CNN 
// (displaying text is now handled by the external function that calls this)
cnn_output_t* run_cnn(int display_txt, int display_bb)
{
    int digs, tens; // format probability
    int max = 0; // the current highest class probability
    int max_i = 0; // the class with the highest probability (0 = face, 1 = no face)

    // first get an image from the camera and load it into the CNN buffer
    capture_camera_img();
    display_RGB565_img(SCREEN_X,SCREEN_Y, cnn_buffer);
    
    // Enable CNN clock
    MXC_SYS_ClockEnable(MXC_SYS_PERIPH_CLOCK_CNN);

    cnn_init(); // Bring state machine into consistent state
    cnn_configure(); // Configure state machine

    load_input();
    cnn_start();
        
    while (cnn_time == 0)
    __WFI(); // Wait for CNN

    // classify the output
    softmax_layer();


    printf("Classification results:\n");

    for (int i = 0; i < CNN_NUM_OUTPUTS; i++) {
      digs = (1000 * ml_softmax[i] + 0x4000) >> 15;
      tens = digs % 10;
      digs = digs / 10;
      printf("[%7d] -> Class %d: %d.%d%%\n", ml_data[i], i, digs, tens);
      if(digs > max)
      {
          max = digs;
          max_i = i;
      }
      memset(buff,32,TFT_BUFF_SIZE);
      //MXC_TFT_FillRect(&clear_word, 4);
      //TFT_Print(buff, 0, 26+16*i, font_1, sprintf(buff, "%s:%d.%d%%\n", class_names[i], digs, tens));
    }

    memset(buff,32,TFT_BUFF_SIZE);
    //MXC_TFT_FillRect(&clear_word, 4);
    TFT_Print(buff, 0, 0, font_1, sprintf(buff, "Class: %s", class_names[max_i]));
    printf("\033[0;0f");

    cnn_stop();
    // Disable CNN clock to save power
    MXC_SYS_ClockDisable(MXC_SYS_PERIPH_CLOCK_CNN);

#ifdef CNN_INFERENCE_TIMER
    //printf("Approximate inference time: %u us\n\n", cnn_time);
#endif
    
    // printf("Classification results:\n");
    for (int i = 0; i < NUM_CLASSES; i++) 
    {
        digs = (1000 * ml_softmax[i] + 0x4000) >> 15;
        tens = digs % 10;
        digs = digs / 10;
        if(digs > max)
        {
            max = digs;
            max_i = i;
        }
        //printf("[%7d] -> Class %d: %d.%d%%\n", ml_data[i], i, digs, tens);
    }
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