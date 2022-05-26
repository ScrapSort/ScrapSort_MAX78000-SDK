#include "watchdog.h"

/***** Globals *****/
static mxc_wdt_cfg_t cfg;

/***** Functions *****/

void watchdogHandler()
{
    MXC_WDT_ClearIntFlag(MXC_WDT0);
    printf("\n\n----- Watchdog has tripped! -----\n\n");

    // printf("### Debug Info ###\n");
    
    // printf("Attempting to rectify the situation with software\n");

    // RESET I2C

    // RESET GPIO

    // RESET TIMERS
    
}

// *****************************************************************************

void WDT0_IRQHandler(void)
{
    watchdogHandler();
}

// *****************************************************************************

void MXC_WDT_Setup()
{
    printf("\nConfiguring WDT . . .\n");

    MXC_WDT_Disable(MXC_WDT0);
    MXC_WDT_ResetTimer(MXC_WDT0);

    cfg.upperResetPeriod = WATCHDOG_RESET_PERIOD;
    MXC_WDT_SetResetPeriod(MXC_WDT0, &cfg);
    MXC_WDT_EnableReset(MXC_WDT0);
    printf("Reset period: 2^%d tics = about %.2f sec\n", 31 - WATCHDOG_RESET_PERIOD, 70.0 / (1UL << WATCHDOG_RESET_PERIOD ));

    cfg.upperIntPeriod = WATCHDOG_INTERRUPT_PERIOD;
    MXC_WDT_SetIntPeriod(MXC_WDT0, &cfg);
    MXC_WDT_EnableInt(MXC_WDT0);
    printf("Interrupt period: 2^%d tics = about %.2f sec\n", 31 - WATCHDOG_INTERRUPT_PERIOD, 70.0 / (1UL << WATCHDOG_INTERRUPT_PERIOD ));

    NVIC_SetVector(WDT0_IRQn, WDT0_IRQHandler);
    NVIC_EnableIRQ(WDT0_IRQn);

    MXC_WDT_Enable(MXC_WDT0);
    printf(". . . Setup complete!\n\n");
}

// *****************************************************************************

void assess_prev_reset () {
    cfg.mode = MXC_WDT_COMPATIBILITY;
    MXC_WDT_Init(MXC_WDT0, &cfg);

    // DISPLAY STATUS OF PREVIOUS RESET
    uint32_t resetFlags = MXC_WDT_GetResetFlag(MXC_WDT0);
    printf("\nPrevious reset caused by: ");

    if (resetFlags != 0) {
        printf("WATCHDOG.\n");
    } else {
        printf("NOT WATCHDOG.\n");
    }
    MXC_WDT_ClearResetFlag(MXC_WDT0);
    MXC_WDT_ClearIntFlag(MXC_WDT0);
}

// *****************************************************************************

void pat_the_dog() {
    MXC_WDT_ResetTimer(MXC_WDT0);
}