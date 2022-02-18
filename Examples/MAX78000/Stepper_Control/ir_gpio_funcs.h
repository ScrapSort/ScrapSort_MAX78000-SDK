

/***** Definitions *****/

// CAMERA IR
#define IR_CAMERA_PORT      MXC_GPIO0
#define IR_CAMERA_PIN       MXC_GPIO_PIN_19


// MOTOR IR 
#define IR_MOTOR_PORT_0      MXC_GPIO2
#define IR_MOTOR_PIN_0       MXC_GPIO_PIN_3



#define MXC_GPIO_PORT_INTERRUPT_STATUS  MXC_GPIO2
#define MXC_GPIO_PIN_INTERRUPT_STATUS   MXC_GPIO_PIN_1

void gpio_init(void);
void ir_motor_handler_0(void* cbdata);
void ir_camera_handler(void* cbdata);