

/***** Definitions *****/

// GPIO 
#define MXC_GPIO_PORT_INTERRUPT_IN      MXC_GPIO1
#define MXC_GPIO_PIN_INTERRUPT_IN       MXC_GPIO_PIN_6

#define MXC_GPIO_PORT_INTERRUPT_STATUS  MXC_GPIO2
#define MXC_GPIO_PIN_INTERRUPT_STATUS   MXC_GPIO_PIN_1

void gpio_isr(void* cbdata);