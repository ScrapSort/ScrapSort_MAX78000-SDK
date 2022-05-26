#include "heartbeat.h"
#include "led.h"
#include "tmr_funcs.h"

bool heartbeatToggle = true;

void heartbeat(){
    if(heartbeatToggle && global_counter % (uint32_t)(1*1000) == 0){
        LED_Toggle(LED_GREEN);
        heartbeatToggle = false;
    }
    else{
        heartbeatToggle = true;
    }
}