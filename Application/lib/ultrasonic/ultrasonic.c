#include "ultrasonic.h"
#include "mxc_device.h"
#include "mxc_delay.h"
#include "nvic_table.h"
#include "board.h"
#include "gpio.h"
#include "mxc.h"
#include "mxc_sys.h"
#include "nvic_table.h"
#include "gcr_regs.h"

// thresholds for ultrasonic distance
#define CLOSE_THRESH 2
#define FAR_THRESH 5

typedef enum
{
    CAM = 0,
    FLIPPER0,
    FLIPPER1,
    FLIPPER2
} ultrasonic_t;

ultrasonic_t camera_idx = CAM;
ultrasonic_t flipper0_idx = FLIPPER0;
ultrasonic_t flipper1_idx = FLIPPER1;
ultrasonic_t flipper2_idx = FLIPPER2;

// gpios for trigger and echos
mxc_gpio_cfg_t trigger_gpio;
mxc_gpio_cfg_t echo_cam_gpio;
mxc_gpio_cfg_t echo_flipper0_gpio;
mxc_gpio_cfg_t echo_flipper1_gpio;
mxc_gpio_cfg_t echo_flipper2_gpio;

// state variables for echo pulse
uint16_t volatile current_pulse_values[] = {0,0,0,0};
uint16_t volatile time_intervals[] = {0,0,0,0};
uint16_t volatile object_statuses[] = {0,0,0,0};


void echo_handler(void* cb_data)
{
    // get the sensor idx from the callback data
    ultrasonic_t sensor_idx = *(ultrasonic_t*)(cb_data);

    // first interrupt (rising edge)
    if(!current_pulse_values[sensor_idx])
    {
        // store the start time
        current_pulse_values[sensor_idx] = global_counter;
    }
    // second interrupt (falling edge)
    else
    {
        // find the pulse length in ms, reset the current value
        time_intervals[sensor_idx] = global_counter - current_pulse_values[sensor_idx];
        current_pulse_values[sensor_idx] = 0;
    }

    // interval less than 10ms means object detected (don't want repeated detection)
    if(!object_statuses[sensor_idx] && time_intervals[sensor_idx] < CLOSE_THRESH)
    {
        // there is an object in front of the sensor
        object_statuses[sensor_idx] = 1;
        printf("object %d cam present\n",sensor_idx);
    }
    else if(object_statuses[sensor_idx] && time_intervals[sensor_idx] >= FAR_THRESH)
    {
        // reset the state
        object_statuses[sensor_idx] = 0;
        printf("object %d cam left\n", sensor_idx);
    }
}

void init_ultrasonic_gpios()
{
    // cam echo gpio
    echo_cam_gpio.port = MXC_GPIO1;
    echo_cam_gpio.mask = MXC_GPIO_PIN_6;
    echo_cam_gpio.func = MXC_GPIO_FUNC_IN;
    echo_cam_gpio.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&echo_cam_gpio);
    MXC_GPIO_RegisterCallback(&echo_cam_gpio, echo_handler, (void*)&camera_idx);
    MXC_GPIO_IntConfig(&echo_cam_gpio, MXC_GPIO_INT_BOTH);
    MXC_GPIO_EnableInt(echo_cam_gpio.port, echo_cam_gpio.mask);
    NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(MXC_GPIO1)));

    // flipper 0 echo gpio
    echo_flipper0_gpio.port = MXC_GPIO2;
    echo_flipper0_gpio.mask = MXC_GPIO_PIN_3;
    echo_flipper0_gpio.func = MXC_GPIO_FUNC_IN;
    echo_flipper0_gpio.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&echo_flipper0_gpio);
    MXC_GPIO_RegisterCallback(&echo_flipper0_gpio, echo_handler, (void*)&flipper0_idx);
    MXC_GPIO_IntConfig(&echo_flipper0_gpio, MXC_GPIO_INT_BOTH);
    MXC_GPIO_EnableInt(echo_flipper0_gpio.port, echo_flipper0_gpio.mask);
    NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(MXC_GPIO2)));

    // flipper 1 echo gpio
    echo_flipper1_gpio.port = MXC_GPIO2;
    echo_flipper1_gpio.mask = MXC_GPIO_PIN_7;
    echo_flipper1_gpio.func = MXC_GPIO_FUNC_IN;
    echo_flipper1_gpio.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&echo_flipper1_gpio);
    MXC_GPIO_RegisterCallback(&echo_flipper1_gpio, echo_handler, (void*)&flipper1_idx);
    MXC_GPIO_IntConfig(&echo_flipper1_gpio, MXC_GPIO_INT_BOTH);
    MXC_GPIO_EnableInt(echo_flipper1_gpio.port, echo_flipper1_gpio.mask);
    NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(MXC_GPIO2)));

    // flipper 2 echo gpio
    echo_flipper2_gpio.port = MXC_GPIO1;
    echo_flipper2_gpio.mask = MXC_GPIO_PIN_1;
    echo_flipper2_gpio.func = MXC_GPIO_FUNC_IN;
    echo_flipper2_gpio.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&echo_flipper2_gpio);
    MXC_GPIO_RegisterCallback(&echo_flipper2_gpio, echo_handler, (void*)&flipper2_idx);
    MXC_GPIO_IntConfig(&echo_flipper2_gpio, MXC_GPIO_INT_BOTH);
    MXC_GPIO_EnableInt(echo_flipper2_gpio.port, echo_flipper2_gpio.mask);
    NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(MXC_GPIO1)));
}

void init_trigger()
{
    // trigger.port = MXC_GPIO2;
    // trigger.mask = MXC_GPIO_PIN_4;
    // trigger.func = MXC_GPIO_FUNC_OUT;
    // trigger.vssel = MXC_GPIO_VSSEL_VDDIOH;
    // MXC_GPIO_Config(&trigger);
    

    mxc_tmr_cfg_t tmr;   
    
    MXC_TMR_Shutdown(PWM_TIMER);
    
    tmr.pres = TMR_PRES_8; // 1 MHz --> 8MHz/8
    tmr.mode = TMR_MODE_PWM;
    tmr.bitMode = TMR_BIT_MODE_32;    
    tmr.clock = MXC_TMR_8M_CLK; // 8MHz
    tmr.cmp_cnt = 50000; // 50ms period
    tmr.pol = 1;

    int err;
    if ((err = MXC_TMR_Init(PWM_TIMER, &tmr, true)) != E_NO_ERROR) {
        printf("Failed PWM timer Initialization: %d\n", err);
        return;
    }

    //MXC_GPIO2->vssel |= MXC_GPIO_VSSEL_VDDIOH;

    // trigger for 10 us
    if ((err = MXC_TMR_SetPWM(PWM_TIMER, 10)) != E_NO_ERROR) {
        printf("Failed TMR_PWMConfig: %d\n", err);
        return;
    }
    
    MXC_TMR_Start(PWM_TIMER);
    
    printf("PWM started.\n\n");
}

void trigger_high()
{
    MXC_GPIO_OutSet(MXC_GPIO2, MXC_GPIO_PIN_4);
}

void trigger_low()
{
    MXC_GPIO_OutClr(MXC_GPIO2, MXC_GPIO_PIN_4);
}