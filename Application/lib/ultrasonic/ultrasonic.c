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

mxc_gpio_cfg_t trigger;
mxc_gpio_cfg_t echo;

uint32_t volatile currentValue = 0;
uint32_t volatile lastValue = 0;
uint32_t volatile timeInterval = 0;

void echo_handler(void* cb_data)
{
    if(!currentValue)
    {
        currentValue = global_counter;
    }
    else
    {
        lastValue = currentValue;
        timeInterval = global_counter - lastValue;
        currentValue = 0;
    }
    if(timeInterval < 10)
    printf("OBJECT\n");
}

void init_echo()
{
    echo.port = MXC_GPIO0;
    echo.mask = MXC_GPIO_PIN_9;
    echo.func = MXC_GPIO_FUNC_IN;
    echo.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&echo);
    MXC_GPIO_RegisterCallback(&echo, echo_handler, NULL);
    MXC_GPIO_IntConfig(&echo, MXC_GPIO_INT_BOTH);
    MXC_GPIO_EnableInt(echo.port, echo.mask);
    NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(MXC_GPIO0)));
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
    
    tmr.pres = TMR_PRES_8; // 1 MHz
    tmr.mode = TMR_MODE_PWM;
    tmr.bitMode = TMR_BIT_MODE_32;    
    tmr.clock = MXC_TMR_8M_CLK;
    tmr.cmp_cnt = 50000; // 50ms period
    tmr.pol = 1;

    int err;
    if ((err = MXC_TMR_Init(PWM_TIMER, &tmr, true)) != E_NO_ERROR) {
        printf("Failed PWM timer Initialization: %d\n", err);
        return;
    }

    //MXC_GPIO2->vssel |= MXC_GPIO_VSSEL_VDDIOH;

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