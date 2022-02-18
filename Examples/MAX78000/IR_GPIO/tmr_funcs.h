// PWM
#define OST_CLOCK_SOURCE    MXC_TMR_8K_CLK       // \ref mxc_tmr_clock_t
#define PWM_CLOCK_SOURCE    MXC_TMR_32K_CLK      // \ref mxc_tmr_clock_t

#define OST_FREQ        1                   // (Hz)
#define OST_TIMER       MXC_TMR5            // Can be MXC_TMR0 through MXC_TMR5

#define FREQ            32000                // (Hz)
#define DUTY_CYCLE      50                  // (%)
#define PWM_TIMER       MXC_TMR4            // must change PWM_PORT and PWM_PIN if changed

// Check Frequency bounds
#if (FREQ == 0)
#error "Frequency cannot be 0."
#elif (FREQ > 100000)
#error "Frequency cannot be over 100000."
#endif

// Check duty cycle bounds
#if (DUTY_CYCLE < 0) || (DUTY_CYCLE > 100)
#error "Duty Cycle must be between 0 and 100."
#endif


extern int pause_ir_interrupts;


void PWMTimer();
void OneshotTimerHandler();
void OneshotTimer();
void PB1Handler();