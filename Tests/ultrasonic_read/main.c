#define FIRST_MOTOR_TEST_NUM 0
#define LAST_MOTOR_TEST_NUM 0
#define IN_TIC_NUM 10
#define OUT_TIC_NUM -110

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
#include "I2C_funcs.h"
#include "motor_funcs.h"
#include "tmr_funcs.h"
#include "cnn_helper_funcs.h"
#include "ultrasonic.h"
#include "camera_tft_funcs.h"



// *****************************************************************************
int main(){
    // Ultrasonic sensors[4];
    // Ultrasonic sensor_cam = sensors[0];
    // Ultrasonic sensor_flipper0 = sensors[1];
    // Ultrasonic sensor_flipper1 = sensors[2];
    // Ultrasonic sensor_flipper2 = sensors[3];

    MXC_ICC_Enable(MXC_ICC0); // Enable cache

    // Switch to 100 MHz clock
    MXC_SYS_Clock_Select(MXC_SYS_CLOCK_IPO);
    SystemCoreClockUpdate();

    // initialize systick
    SysTick_Setup();

    // LCD_Camera_Setup();

    // startup_cnn();
    
    // initialize the ultrasonic sensor pins
    // init_ultrasonic_sensor(&sensor_cam, MXC_GPIO2, MXC_GPIO_PIN_4, MXC_GPIO1, MXC_GPIO_PIN_6);
    Ultrasonic sensor_flipper0;
    init_ultrasonic_sensor(&sensor_flipper0, MXC_GPIO1, MXC_GPIO_PIN_0, MXC_GPIO2, MXC_GPIO_PIN_3);
    // init_ultrasonic_sensor(&sensor_flipper1, MXC_GPIO3, MXC_GPIO_PIN_1, MXC_GPIO2, MXC_GPIO_PIN_7);
    // init_ultrasonic_sensor(&sensor_flipper2, MXC_GPIO2, MXC_GPIO_PIN_6, MXC_GPIO1, MXC_GPIO_PIN_1);
    
  

    // ======================== Main Loop =========================
    while(1) 
    {   
        
        activate_trigger(&sensor_flipper0);
        MXC_Delay(MXC_DELAY_SEC(1));
        printf("Flipper 0 Distance (cm): %d\n", sensor_flipper0.curr_raw_distance_cm);
    }
}
