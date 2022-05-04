/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mxc_device.h"
#include "mxc_delay.h"
#include "nvic_table.h"
#include "i2c_regs.h"
#include "i2c.h"
#include "dma.h"
#include "led.h"

// personal
#include "motor.h"
#include "I2C_funcs.h"
#include "tic.h"

Motor *motors[MotorParams__NUM_OF_MOTORS];

uint32_t get_variable_32(Motor *motor, TicVarOffset offset){
    txdata[0] = TicCommand__GetVariable;
    txdata[1] = offset;

    I2C_Send_Message(motor->i2c_slave_addr, 2, 4, 0);
    return *(uint32_t*)rxdata;
}

uint16_t get_variable_16(Motor *motor, TicVarOffset offset){
    txdata[0] = TicCommand__GetVariable;
    txdata[1] = offset;

    I2C_Send_Message(motor->i2c_slave_addr, 2, 2, 0);
    return *(uint16_t*)rxdata;
}

uint8_t get_variable_8(Motor *motor, TicVarOffset offset){
    txdata[0] = TicCommand__GetVariable;
    txdata[1] = offset;

    I2C_Send_Message(motor->i2c_slave_addr, 2, 1, 0);
    return *(uint8_t*)rxdata;
}

// TODO Replace
// void Debug_Motors(void) {
//     // PRINT OUT STATUS/ERROR VARS FOR DEBUG
//     printf("\n#######  DEBUG VARS  #######\n\n");

//     for (int slave_addr = START_SLAVE_ADDR; slave_addr < START_SLAVE_ADDR + NUM_SLAVES; slave_addr++) {
        
//         // GET OPERATION STATE
//         printf("Operation Status: ");
//         txdata[0] = TicCommand__GetVariable;
//         txdata[1] = TicVarOffset__OperationState;

//         I2C_Send_Message(motor->i2c_slave_addr, 2, 1, 0);


//         // GET MISC FLAGS
//         printf("Misc Flags: ");
//         txdata[0] = TicCommand__GetVariable;
//         txdata[1] = TicVarOffset__MiscFlags1;

//         I2C_Send_Message(motor->i2c_slave_addr, 2, 1, 0);

//         // GET ERROR STATUS
//         printf("Error Status: ");
//         txdata[0] = TicCommand__GetVariable;
//         txdata[1] = TicVarOffset__ErrorStatus;

//         I2C_Send_Message(motor->i2c_slave_addr, 2, 2, 0);

//         // GET ERRORS OCCURRED
//         printf("Errors Occurred: ");
//         txdata[0] = TicCommand__GetVariable;
//         txdata[1] = TicVarOffset__ErrorsOccurred;

//         I2C_Send_Message(motor->i2c_slave_addr, 2, 4, 0);

//         // GET CURRENT POSITION
//         printf("Current Position: ");
//         txdata[0] = TicCommand__GetVariable;
//         txdata[1] = TicVarOffset__CurrentPosition;

//         I2C_Send_Message(motor->i2c_slave_addr, 2, 4, 0);

//         printf("\n############\n");

        
//     }
// }

//TODO
float get_max_microstep(Motor *motor){
    //Function that first sets motor to smallest step size
    //Motor is then homed and then reversed home 
    //The microstep position is returned 

    set_motor_profile(motor, MOTOR_PROFILE_CALIBRATE);
    go_home_reverse(motor);
    wait_for_home(motor);
    go_home_forward(motor);
    wait_for_home(motor);
    return 0;
}

void wait_for_target(Motor *motor){
    //Blocks until the stepper is finished
    while(get_current_position(motor) != motor->currTarget){
        MXC_DELAY_MSEC(100);
    }
    return; 
}

void wait_for_home(Motor *motor){
    //Blocks until the stepper is finished
    while(get_current_position(motor) != 0){
        MXC_DELAY_MSEC(100);
    }
    return; 
}

void set_motor_profile(Motor *motor, MOTOR_PROFILE profile){
    uint32_t profile_accel_max, profile_decel_max, profile_speed_max, profile_speed_start;
    uint8_t profile_step_mode; 

    if(profile==MOTOR_PROFILE_TORQUE){
        profile_accel_max = 200*100;
        profile_decel_max = profile_accel_max;
        profile_speed_max = 400*10000;
        // profile_speed_max = 900*10000;
        profile_speed_start = 0;
        profile_step_mode = TicStepMode__Microstep1;
    }
    else if(profile == MOTOR_PROFILE_SPEED){
        profile_accel_max = 5000*100;
        profile_decel_max = profile_accel_max;
        profile_speed_max = 2000*10000;
        profile_speed_start = 0;
        profile_step_mode = TicStepMode__Microstep1;
    }
    else if(profile == MOTOR_PROFILE_CALIBRATE){
        profile_accel_max = 5000*100;
        profile_decel_max = profile_accel_max;
        profile_speed_max = 2000*10000;
        profile_speed_start = 0;
        profile_step_mode = TicStepMode__Microstep8;
    }
    else{
        printf("Invalid Motor Profile Set");
        return;
    }

    set_accel_max(motor, profile_accel_max);
    set_decel_max(motor, profile_decel_max);
    set_speed_max(motor, profile_speed_max);
    set_speed_start(motor, profile_speed_start);
    set_step_mode(motor, profile_step_mode);
    get_microstep_factor(motor);
}


void deenergize(Motor *motor){
    txdata[0] = TicCommand__Deenergize;
    I2C_Send_Message(motor->i2c_slave_addr, 1, 0, 0);
    motor->isEnergized = false;
}

void energize(Motor *motor){
    txdata[0] = TicCommand__Energize;
    I2C_Send_Message(motor->i2c_slave_addr, 1, 0, 0);
    motor->isEnergized = true;
}

void set_speed_max(Motor *motor, uint32_t speed_max){
    txdata[0] = TicCommand__SetSpeedMax;
    fill_tx_32b(speed_max);
    I2C_Send_Message(motor->i2c_slave_addr, 5, 0, 0);
    get_speed_max(motor);
}

uint32_t get_speed_max(Motor *motor){
    motor->maxSpeed = get_variable_32(motor, TicVarOffset__SpeedMax);
    return motor->maxSpeed;
}

void set_speed_start(Motor *motor, uint32_t speed_start){
    txdata[0] = TicCommand__SetStartingSpeed;
    fill_tx_32b(speed_start);
    I2C_Send_Message(motor->i2c_slave_addr, 5, 0, 0);
    get_speed_start(motor);
}

uint32_t get_speed_start(Motor *motor){
    motor->startSpeed = get_variable_32(motor, TicVarOffset__StartingSpeed);
    return motor->startSpeed;
}

void set_accel_max(Motor *motor, uint32_t accel_max){
    txdata[0] = TicCommand__SetAccelMax;
    fill_tx_32b(accel_max);
    I2C_Send_Message(motor->i2c_slave_addr, 5, 0, 0);
    get_accel_max(motor);
}

uint32_t get_accel_max(Motor *motor){
    motor->maxAccel = get_variable_32(motor, TicVarOffset__AccelMax);
    return motor->maxAccel;
}

void set_decel_max(Motor *motor, uint32_t decel_max){
    txdata[0] = TicCommand__SetDecelMax;
    fill_tx_32b(decel_max);
    I2C_Send_Message(motor->i2c_slave_addr, 5, 0, 0);
    get_decel_max(motor);
}

uint32_t get_decel_max(Motor *motor){
    motor->maxDecel = get_variable_32(motor, TicVarOffset__DecelMax);
    return motor->maxDecel;
}

void set_step_mode(Motor *motor, uint8_t step_mode){
    txdata[0] = TicCommand__SetStepMode;
    fill_tx_32b(step_mode);
    I2C_Send_Message(motor->i2c_slave_addr, 5, 0, 0);
    get_step_mode(motor);
}

uint8_t get_step_mode(Motor *motor){
    motor->stepMode = get_variable_8(motor, TicVarOffset__StepMode);
    return motor->stepMode;
}

void set_current_limit(Motor *motor, uint8_t current_limit){
    txdata[0] = TicCommand__SetCurrentLimit;
    fill_tx_32b(current_limit);
    I2C_Send_Message(motor->i2c_slave_addr, 5, 0, 0);
    get_current_limit(motor);
}

uint8_t get_current_limit(Motor *motor){
    motor->currentLimit = get_variable_8(motor, TicVarOffset__CurrentLimit);
    return motor->currentLimit;
}

void set_decay_mode(Motor *motor, uint8_t decay_mode){
    txdata[0] = TicCommand__SetDecayMode;
    fill_tx_32b(decay_mode);
    I2C_Send_Message(motor->i2c_slave_addr, 5, 0, 0);
}

void set_angle(Motor *motor, float deg){
    float deltaDeg = deg - get_angle(motor);
    uint32_t new_position = 0;
    get_microstep_factor(motor);
    new_position = (uint32_t)((deltaDeg * MotorParams__STEPS_PER_REV * motor->microstepFactor)/360); 
    set_target_position(motor, new_position); 
}

void set_target_position(Motor *motor, uint32_t position){
    txdata[0] = TicCommand__SetTargetPosition;
    fill_tx_32b(position);
    I2C_Send_Message(motor->i2c_slave_addr, 5, 0, 0);
}


uint8_t get_microstep_factor(Motor *motor){
    uint8_t microstepFactor = 1;
    switch(get_step_mode(motor)){
        case TicStepMode__Full:
            microstepFactor = 1;
            break;
        case TicStepMode__Half:
            microstepFactor = 2;
            break;
        case TicStepMode__Microstep4:
            microstepFactor = 4;
            break;
        case TicStepMode__Microstep8:
            microstepFactor = 8;
            break;
    }
    motor->microstepFactor = microstepFactor;
    return motor->microstepFactor;
}

float get_angle(Motor *motor){
    get_microstep_factor(motor);
    get_current_position(motor);
    return 360.0 * (motor->currPosition)/(MotorParams__STEPS_PER_REV * motor->microstepFactor);
}

uint32_t get_current_position(Motor *motor){
    //in microsteps
    motor->currPosition = get_variable_32(motor, TicVarOffset__CurrentPosition);
    return motor->currPosition;
}

//TODO Replace
void rotate_revs(Motor *motor, float rotations) {
    

    // GET CURRENT POSITION
    txdata[0] = TicCommand__GetVariable;
    txdata[1] = TicVarOffset__CurrentPosition;

    I2C_Send_Message(motor->i2c_slave_addr, 2, 4, 0);

    int curr_pos = rxdata[0] + (rxdata[1] << 8) + (rxdata[2] << 16) + (rxdata[3] << 24);

    // SET TARGET POSITION
    // full rotation = 200 encoder ticks

    int enc_target = (int)(rotations * 200 + curr_pos);

    // printf("")

    txdata[0] = TicCommand__SetTargetPosition;
    fill_tx_32b(enc_target);

    I2C_Send_Message(motor->i2c_slave_addr, 5, 0, 0);
}

void go_home_forward(Motor *motor){
    txdata[0] = TicCommand__GoHome;
    fill_tx_32b(1);
    I2C_Send_Message(motor->i2c_slave_addr, 2, 0, 0);
}

void go_home_reverse(Motor *motor){
    txdata[0] = TicCommand__GoHome;
    fill_tx_32b(0);
    I2C_Send_Message(motor->i2c_slave_addr, 2, 0, 0);
}

//TODO 
int Motor_Init_Settings(Motor **motors, size_t motors_size) {

    //TODO Check if this does anything...
    // RESET COMMAND TIMEOUT
    txdata[0] = TicCommand__ResetCommandTimeout;

    I2C_Broadcast_Message(1, 0, 0);

    //TODO Check if this does anything...
    // RESET 
    txdata[0] = TicCommand__Reset;
    I2C_Broadcast_Message(1, 0, 0);

    MXC_Delay(1000);

    //TODO Check if this does anything...
    // EXIT SAFE START
    txdata[0] = TicCommand__ExitSafeStart;
    I2C_Broadcast_Message(1, 0, 0);

    //TODO Check if this does anything...
    // ENERGIZE
    txdata[0] = TicCommand__Energize;
    I2C_Broadcast_Message(1, 0, 0);

    for (size_t motor_num = 0; motor_num < motors_size; motor_num++){
        // GET VARIABLE: OPERATION STATE
        txdata[0] = TicCommand__GetVariable;
        txdata[1] = TicVarOffset__OperationState;

        I2C_Send_Message(motors[motor_num]->i2c_slave_addr, 2, 1, 0);

        if (rxdata[0] != 10) { // normal operation state
            printf("ERROR could not init motor %d\n", motor_num);
            // GET ERROR STATUS
            txdata[0] = TicCommand__GetVariableAndClearErrorsOccurred;
            txdata[1] = TicVarOffset__ErrorStatus;

            I2C_Send_Message(motors[motor_num]->i2c_slave_addr, 2, 1, 0);
            printf("ERROR CODE: %d\n", rxdata[0]);
            return -1;
        }
    }

    return E_NO_ERROR;


}