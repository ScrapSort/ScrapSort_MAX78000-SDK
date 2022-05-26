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
#include "tmr_funcs.h"

#include <stdlib.h>

#define BLOCK_COEFFICIENT 0.45

Motor motor1;
Motor motor2;
Motor motor3;
Motor *motors[MotorParams__NUM_OF_MOTORS] = {&motor1, &motor2, &motor3};

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

void calibrate_motors(Motor *motors[], size_t num_of_motors){
    for(size_t motor_num = 0; motor_num < num_of_motors; motor_num++){
        set_motor_profile(motors[motor_num], MOTOR_PROFILE_CALIBRATE);
        MXC_Delay(MSEC(10));
    }
    for(size_t motor_num = 0; motor_num < num_of_motors; motor_num++){
        go_home_reverse(motors[motor_num]);
    }
    wait_for_homes(motors, num_of_motors, false);
    printf("First Home\n");
    for(size_t motor_num = 0; motor_num < num_of_motors; motor_num++){
        go_home_forward(motors[motor_num]);
    }
    wait_for_homes(motors, num_of_motors, true);
    for(size_t motor_num = 0; motor_num < num_of_motors; motor_num++){
        motors[motor_num]->currTarget = 0;
        motors[motor_num]->lastHome = global_counter;
    }
}

void wait_for_homes(Motor *motors[], size_t num_of_motors, bool store_max_step){
    uint32_t max_positions[10];
    memset(max_positions,0,4*10);
    uint32_t curr_positions[10];
    memset(curr_positions, 0, 4*10);
    bool all_homed = false;
    while(!all_homed){
        // printf("New loop\n");
        all_homed = true;
        for(size_t motor_num = 0; motor_num < num_of_motors; motor_num++){
            curr_positions[motor_num] = abs(get_current_position(motors[motor_num]));
            if(curr_positions[motor_num] > max_positions[motor_num]){
                max_positions[motor_num] = curr_positions[motor_num]; 
            }
            // printf("Position: %d\n", curr_positions[motor_num]);
            all_homed = all_homed && (curr_positions[motor_num] == 0);
            MXC_Delay(MXC_DELAY_MSEC(10));
        }
    }
    printf("About to store: \n");
    if(store_max_step){
        for(size_t motor_num = 0; motor_num < num_of_motors; motor_num++){
            motors[motor_num]->maxStep = (uint32_t)(max_positions[motor_num]/2.0);
            motors[motor_num]->maxMicrostep = (uint32_t)(max_positions[motor_num]);
            printf("%d ", motors[motor_num]->maxStep);
        }
    }
    printf("\n");
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
float get_max_step(Motor *motor){
    //Function that first sets motor to smallest step size
    //Motor is then homed and then reversed home 
    //The microstep position is returned 

    set_motor_profile(motor, MOTOR_PROFILE_CALIBRATE);
    go_home_reverse(motor);
    MXC_Delay(MSEC(250));
    wait_for_home(motor);
    go_home_forward(motor);
    MXC_Delay(MSEC(250));
    
    motor->maxMicrostep = wait_for_home(motor);
    //Divide by microstep factor
    motor->maxStep = motor->maxMicrostep/2.0;
    return motor->maxStep;
}

void wait_for_target(Motor *motor){
    //Blocks until the stepper is finished
    while(get_current_position(motor) != motor->currTarget){
        MXC_Delay(MXC_DELAY_MSEC(100));
    }
    return; 
}

void block_object(Motor *motor){
    motor->homed = true;
    set_motor_profile(motor, MOTOR_PROFILE_SPEED);
    get_microstep_factor(motor);
    set_target_position(motor, -(int32_t)(motor->maxStep*motor->microstepFactor*BLOCK_COEFFICIENT));
    motor->lastBlock = global_counter;
    printf("Block Motor Current Position: %d\n", get_current_position(motor));
}

void pull_object(Motor *motor){
    set_motor_profile(motor, MOTOR_PROFILE_TORQUE);
    get_microstep_factor(motor);
    set_target_position(motor, (int32_t)(motor->maxStep*motor->microstepFactor*BLOCK_COEFFICIENT));
    motor->currTarget = 0;
    motor->lastHome = global_counter;
    motor->homed = false;
    printf("Pull Motor Current Position: %d\n", get_current_position(motor));
}

void motor_handler(Motor *motors[], size_t num_of_motors){
    if(global_counter % ((uint32_t)(1*10000)) == 0){
        for(size_t motor_num = 0; motor_num < num_of_motors; motor_num++){
            if(!motors[motor_num]->homed && ((global_counter - motors[motor_num]->lastHome) > (.5*10000))){
                printf("################################Go home %d\n", motor_num);
                go_home_forward(motors[motor_num]);
                motors[motor_num]->homed = true;
            }
        }
    }
}

void push_object(Motor *motor){
    set_motor_profile(motor, MOTOR_PROFILE_TORQUE);
    // MXC_Delay(MSEC(1));
    get_microstep_factor(motor);
    set_target_position(motor, -(int32_t)(motor->maxStep*motor->microstepFactor));
}

int32_t halt_and_set_position(Motor *motor, int32_t position){
    txdata[0] = TicCommand__HaltAndSetPosition;
    fill_tx_32b(position);
    I2C_Send_Message(motor->i2c_slave_addr, 5, 0, 0);
    motor->currPosition = get_current_position(motor);
    return motor->currPosition;
}

uint32_t wait_for_home(Motor *motor){
    uint32_t max_position = 0, curr_position = 0;
    //Blocks until the stepper is finished
    while(get_current_position(motor) != 0){
        curr_position = abs(get_current_position(motor));
        if(curr_position > max_position){
            max_position = curr_position;
        }
        MXC_Delay(MXC_DELAY_MSEC(100));
    }
    return max_position; 
}


void set_motor_profile(Motor *motor, MOTOR_PROFILE profile){
    uint32_t profile_accel_max, profile_decel_max, profile_speed_max, profile_speed_start, profile_speed_homing_towards, profile_speed_homing_away;
    uint8_t profile_step_mode; 

    if(profile==MOTOR_PROFILE_TORQUE){
        profile_accel_max = 2*8*200*100;
        profile_decel_max = 4*200*100;
        profile_speed_max = 2*8*400*10000;
        // profile_speed_homing_towards = 50000;
        // profile_speed_homing_away = 50000;
        // profile_speed_max = 900*10000;
        profile_speed_start = 0;
        profile_step_mode = TicStepMode__Microstep2;
    }
    else if(profile == MOTOR_PROFILE_SPEED){
        profile_accel_max = 2*8*200*100;
        profile_decel_max = 4*200*100;
        profile_speed_max = 2*8*400*10000;
        // profile_speed_homing_towards = 50000;
        // profile_speed_homing_away = 50000;
        profile_speed_start = 0;
        profile_step_mode = TicStepMode__Microstep1;
    }
    // else if(profile == MOTOR_PROFILE_DEFAULT){
    //     profile_accel_max = 2000000;
    //     profile_decel_max = profile_accel_max;
    //     profile_speed_max = 40000;
    //     profile_speed_start = 0;
    //     profile_step_mode = TicStepMode__Microstep1;
    // }
    else if(profile == MOTOR_PROFILE_DEFAULT){
        profile_accel_max = 20000000;
        profile_decel_max = profile_accel_max;
        profile_speed_max = 4000000;
        profile_speed_homing_towards = 40000;
        profile_speed_homing_away = 40000;
        profile_speed_start = 0;
        profile_step_mode = TicStepMode__Microstep2;
    }

    else if(profile == MOTOR_PROFILE_CALIBRATE){
        profile_accel_max = 5000*100;
        profile_decel_max = profile_accel_max;
        profile_speed_max = 2000*10000;
        profile_speed_homing_towards = 50000;
        profile_speed_homing_away = 50000;
        profile_speed_start = 0;
        profile_step_mode = TicStepMode__Microstep2;
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
    halt_and_set_position(motor, motor->currPosition);
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

uint8_t get_input_state(Motor *motor){
    motor->inputState = get_variable_8(motor, TicVarOffset__InputState);
    return motor->inputState;
}

uint32_t get_speed_homing_towards(Motor *motor){
    motor->speedHomingTowards = get_variable_32(motor, TicVarOffset__HomingSpeedTowards);
    return motor->speedHomingTowards;
}

uint32_t get_speed_homing_away(Motor *motor){
    motor->speedHomingAway = get_variable_32(motor, TicVarOffset__HomingSpeedAway);
    return motor->speedHomingAway;
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
    int32_t new_position = 0;
    get_microstep_factor(motor);
    new_position = (uint32_t)((deltaDeg * MotorParams__STEPS_PER_REV * motor->microstepFactor)/360); 
    printf("New Position: %d\n", new_position);
    printf("DDeg: %f\n", deltaDeg);
    set_target_position(motor, -new_position); 
}

void set_target_position(Motor *motor, int32_t position){
    txdata[0] = TicCommand__SetTargetPosition;
    fill_tx_32b(position);
    I2C_Send_Message(motor->i2c_slave_addr, 5, 0, 0);
    printf("set Postion: %d\n", position);
    motor->currTarget += position;
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

int32_t get_current_position(Motor *motor){
    //in microsteps
    motor->currPosition = get_variable_32(motor, TicVarOffset__CurrentPosition);
    return motor->currPosition;
}

uint8_t get_planning_mode(Motor *motor){
    motor->planningMode = get_variable_8(motor, TicVarOffset__PlanningMode);
    return motor->planningMode;
}

//TODO Replace
// void rotate_revs(Motor *motor, float rotations) {
    

//     // GET CURRENT POSITION
//     txdata[0] = TicCommand__GetVariable;
//     txdata[1] = TicVarOffset__CurrentPosition;

//     I2C_Send_Message(motor->i2c_slave_addr, 2, 4, 0);

//     int curr_pos = rxdata[0] + (rxdata[1] << 8) + (rxdata[2] << 16) + (rxdata[3] << 24);

//     // SET TARGET POSITION
//     // full rotation = 200 encoder ticks

//     int enc_target = (int)(rotations * 200 + curr_pos);

//     // printf("")

//     txdata[0] = TicCommand__SetTargetPosition;
//     fill_tx_32b(enc_target);

//     I2C_Send_Message(motor->i2c_slave_addr, 5, 0, 0);
// }

void go_home_forward(Motor *motor){
    printf("Addr: %d\n", motor->i2c_slave_addr);
    txdata[0] = TicCommand__GoHome;
    fill_tx_32b(1);
    I2C_Send_Message(motor->i2c_slave_addr, 2, 0, 0);
}

void go_home_reverse(Motor *motor){
    txdata[0] = TicCommand__GoHome;
    fill_tx_32b(0);
    I2C_Send_Message(motor->i2c_slave_addr, 2, 0, 0);
}

void Motor_Init(Motor *motor, uint8_t new_i2c_slave_addr){
    motor->i2c_slave_addr = new_i2c_slave_addr;
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

        set_current_limit(motors[motor_num], 13);
        motors[motor_num]->lastBlock = 0;
        
    }

    calibrate_motors(motors, motors_size);
    MXC_Delay(SEC(3));
    printf("Motors Calibrated\n");

    return E_NO_ERROR;


}