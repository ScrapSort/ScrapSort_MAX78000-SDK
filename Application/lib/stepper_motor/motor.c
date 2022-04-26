#include <stdint.h>
#include <stdbool.h>
#include "motor_funcs.h"

typedef struct {
    //functions
    void (*block)(void *self);
    void (*close_ccw)(void *self);
    void (*close_cw)(void *self);


    int (*calibrate)(void *self, void *buff, size_t max_sz, size_t *p_act_sz);
    int (*write)(void *self, void *buff, size_t max_sz, size_t *p_act_sz);
    
    // Data
    uint32_t currentTargetPosition;
    MOTOR_PROFILE currentProfile;
    bool isReady;

} tMotorClass;