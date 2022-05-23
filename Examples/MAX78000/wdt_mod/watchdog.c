#include "watchdog.h"

/***** Globals *****/

static mxc_wdt_cfg_t cfg;
volatile int interrupt_count = 0;


/***** Functions *****/

// *****************************************************************************
void watchdogHandler()
{
    MXC_WDT_ClearIntFlag(MXC_WDT0);
    
    if (interrupt_count == 0) {
        printf("\nWatchdog has tripped!\n");
        printf("This is the first time, so we'll go ahead and reset it\n");
        printf("once it is within the proper window.\n");
        interrupt_count++;
    }
    else {
        printf("\nWatchdog has tripped!\n");
        printf("This is the not the first time.  What happens if we\n");
        printf("do not reset it?\n");
    }
}

// *****************************************************************************
void WDT0_IRQHandler(void)
{
    watchdogHandler();
}
// *****************************************************************************
void MXC_WDT_Setup()
{
    MXC_WDT_Disable(MXC_WDT0);
    MXC_WDT_ResetTimer(MXC_WDT0);
    // cfg.lowerResetPeriod = MXC_WDT_PERIOD_2_24;
    cfg.upperResetPeriod = MXC_WDT_PERIOD_2_28;
    MXC_WDT_SetResetPeriod(MXC_WDT0, &cfg);
    MXC_WDT_Enable(MXC_WDT0);
}