#include "heartbeat.h"
#include "led.h"
#include "tmr_funcs.h"

void heartbeat(){
    if(global_counter % (uint32_t)(0.5*1000) == 0){
        LED_Toggle(LED_GREEN);
    }
}