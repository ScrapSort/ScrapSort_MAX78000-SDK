// PWM - some of these are prolly unnecessary
#include <stdint.h>
#include "mxc_sys.h"
#include "lpgcr_regs.h"
#include "gcr_regs.h"
#include "pwrseq_regs.h"
#include "mxc.h"
#include "lp.h"
#include "led.h"

#include "tmr_funcs.h"
#include "motor.h"

#include "sorter.h"
#include "color_print.h"


/***** Globals *****/
int pause_camera_interrupts = 0;
int ost_counter = 0;
volatile uint32_t global_counter = 0;
uint32_t SYSTICK_PRESCALER = 10000;

/***** Functions *****/
void PWMTimer()
{
    // Declare variables
    mxc_tmr_cfg_t tmr;          // to configure timer
    unsigned int periodTicks = MXC_TMR_GetPeriod(PWM_TIMER, PWM_CLOCK_SOURCE, 16, FREQ);
    // unsigned int dutyTicks   = periodTicks * DUTY_CYCLE / 100;

    /*
    Steps for configuring a timer for PWM mode:
    1. Disable the timer
    2. Set the pre-scale value
    3. Set polarity, PWM parameters
    4. Configure the timer for PWM mode
    5. Enable Timer
    */
    
    MXC_TMR_Shutdown(PWM_TIMER);
    
    tmr.pres = TMR_PRES_16;
    tmr.mode = TMR_MODE_PWM;
    tmr.bitMode = TMR_BIT_MODE_32;    
    tmr.clock = PWM_CLOCK_SOURCE;
    tmr.cmp_cnt = periodTicks;
    tmr.pol = 1;
    
    if (MXC_TMR_Init(PWM_TIMER, &tmr, true) != E_NO_ERROR) {
        printf("Failed PWM timer Initialization.\n");
        return;
    }
    
    // if (MXC_TMR_SetPWM(PWM_TIMER, dutyTicks) != E_NO_ERROR) {
    //     printf("Failed TMR_PWMConfig.\n");
    //     return;
    // }
    
    MXC_TMR_Start(PWM_TIMER);
    
    printf("PWM started.\n\n");
}

void OneshotTimerHandler()
{
    // Clear interrupt
    MXC_TMR_ClearFlags(OST_TIMER);
    
    // Clear interrupt
    if (MXC_TMR5->wkfl & MXC_F_TMR_WKFL_A) {
        MXC_TMR5->wkfl = MXC_F_TMR_WKFL_A;

        if (ost_counter > 0) {

            printf("Oneshot timer expired!\n");
            pause_camera_interrupts = 0;
            // target_tics(0, 0);

            ost_counter = 0;
        } else {
            // printf("Reup oneshot timer\n");
            ost_counter ++;

            NVIC_SetVector(TMR5_IRQn, OneshotTimerHandler);
            NVIC_EnableIRQ(TMR5_IRQn);
            
            OneshotTimer();
        }
    }
}

void OneshotTimer()
{
    for(int i = 0; i < 5000; i++);      //Button debounce

    // Declare variables
    mxc_tmr_cfg_t tmr;
    uint32_t periodTicks = MXC_TMR_GetPeriod(OST_TIMER, OST_CLOCK_SOURCE, 1, OST_FREQ);
    // printf("\nperiodTicks: %u\n", periodTicks);

    /*
    Steps for configuring a timer for PWM mode:
    1. Disable the timer
    2. Set the prescale value
    3  Configure the timer for continuous mode
    4. Set polarity, timer parameters
    5. Enable Timer
    */
    
    MXC_TMR_Shutdown(OST_TIMER);
    
    tmr.pres = TMR_PRES_1;
    tmr.mode = TMR_MODE_ONESHOT;
    tmr.bitMode = TMR_BIT_MODE_32;
    tmr.clock = OST_CLOCK_SOURCE;
    tmr.cmp_cnt = periodTicks;      //SystemCoreClock*(1/interval_time);
    tmr.pol = 0;
    
    if (MXC_TMR_Init(OST_TIMER, &tmr, true) != E_NO_ERROR) {
        printf("Failed one-shot timer Initialization.\n");
        return;
    }
    
    MXC_TMR_EnableInt(OST_TIMER);
    
    // Enable wkup source in Poower seq register
    MXC_LP_EnableTimerWakeup(OST_TIMER);
    // Enable Timer wake-up source
    MXC_TMR_EnableWakeup(OST_TIMER, &tmr);
    
    // printf("Oneshot timer started.\n\n");
    
    MXC_TMR_Start(OST_TIMER);

    pause_camera_interrupts = 1;
}

void PB1Handler()
{
    // printf("Sorter:\n");
    // sorter__print(&scrappy);

    //printf("PWM button pressed\n");
    PWMTimer();    
}

void SysTick_Setup() {
    // setup the systick interrupt
    NVIC_SetVector(SysTick_IRQn, SysTick_Handler);
    NVIC_EnableIRQ(SysTick_IRQn);

    // make it high priority to avoid missing tick increments
    NVIC_SetPriority(SysTick_IRQn,0);

    // num ticks b/t interrupts = 100MHz/10k = 10k
    // 10k ticks of 100MHz clk = 10k/100MHz = 100us
    int err = SysTick_Config(SystemCoreClock/SYSTICK_PRESCALER);
    if(err != 0)
    {
        printf(ANSI_COLOR_RED "--> Systick Failed" ANSI_COLOR_RESET "\n");
    }
    else
    {
        printf(ANSI_COLOR_GREEN "--> Systick initialized" ANSI_COLOR_RESET "\n");
    }
}

void SysTick_Handler() {
    global_counter++;
}