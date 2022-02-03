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
//volatile uint32_t cnn_time; // Stopwatch to time forward pass, not used at the moment
cnn_output_t output; // the output data of the CNN

static int32_t ml_data[NUM_CLASSES]; // classification output data
static q15_t ml_softmax[NUM_CLASSES]; // softmax output data

output_classes_t class_names[] = {CUP, TRAPEZOID, HEXAGON, CAN, BOTTLE, NONE};

// gpios for capturing interrupt
mxc_gpio_cfg_t trigger_gpio;
volatile int triggered = 0;

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
cnn_output_t* run_cnn()
{
    int digs, tens; // format probability
    int max = 0; // the current highest class probability
    int max_i = 0; // the class with the highest probability
    
    // cnn output
    static cnn_output_t output;

    // first get an image from the camera and load it into the CNN buffer
    capture_camera_img();
    display_RGB565_img(SCREEN_X,SCREEN_Y, cnn_buffer);
    
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

    printf("Classification results:\n");

    for (int i = 0; i < CNN_NUM_OUTPUTS; i++) 
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
    printf("\033[0;0f");
    // Disable CNN clock to save power
    cnn_stop();
    MXC_SYS_ClockDisable(MXC_SYS_PERIPH_CLOCK_CNN);

#ifdef CNN_INFERENCE_TIMER
    //printf("Approximate inference time: %u us\n\n", cnn_time);
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

void init_trigger()
{
    trigger_gpio.port = MXC_GPIO2;
    trigger_gpio.mask = MXC_GPIO_PIN_3;
    trigger_gpio.pad = MXC_GPIO_PAD_PULL_UP; // HI by default
    trigger_gpio.func = MXC_GPIO_FUNC_IN;
    trigger_gpio.vssel = MXC_GPIO_VSSEL_VDDIOH;

    MXC_GPIO_Config(&trigger_gpio);
    MXC_GPIO_RegisterCallback(&trigger_gpio, trigger_callback, NULL);
    MXC_GPIO_IntConfig(&trigger_gpio, MXC_GPIO_INT_FALLING);
    MXC_GPIO_EnableInt(MXC_GPIO2, MXC_GPIO_PIN_3);
    NVIC_EnableIRQ(GPIO2_IRQn);
}

void trigger_callback()
{
  printf("TRIGGERED!\n");
  triggered = 1;
}

int trigger_check()
{
  if(triggered == 1)
  {
      triggered = 0;
      return 1;
  }
  else
  {
      return 0;
  }
}