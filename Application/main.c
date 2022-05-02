/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mxc_device.h"
#include "mxc_delay.h"
#include "nvic_table.h"
#include "i2c_regs.h"
#include "i2c.h"
#include "dma.h"
#include "pb.h"
#include "board.h"
#include "mxc.h"


// personal
#include "tmr_funcs.h"
#include "cnn_helper_funcs.h"
#include "camera_tft_funcs.h"


#define COLLECT_DATA
//#define STREAM_MODE

#ifdef COLLECT_DATA
#include "capture_button.h"
#endif

// *****************************************************************************
int main()
{
    // Switch to 100 MHz clock
    MXC_SYS_Clock_Select(MXC_SYS_CLOCK_IPO);
    SystemCoreClockUpdate();

    // initialize systick
    SysTick_Setup();

    #ifdef COLLECT_DATA
    init_card();
    init_class_button();
    init_capture_button();
    #endif

    LCD_Camera_Setup();

    // activate the first ultrasonic sensor
    activate_triggercam();

    // ======================== Main Loop =========================
    
    while(1) 
    {
        #ifdef COLLECT_DATA
        capture_camera_img();
      
        display_RGB565_img(56,96,NULL,false);
        if(get_capture_state() == 1)
        {
            capture();
        }
        if(get_switch_state() == 1)
        {
            switch_class();
        }
        #endif

        #ifdef STREAM_MODE
        output = *run_cnn();
        show_cnn_output(output);
        #endif

        // check if the next ultrasonic sensor should be triggered
        trigger();
    }
}
