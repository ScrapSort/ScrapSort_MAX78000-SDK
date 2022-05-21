#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>
#include "tic.h"

typedef enum
{
    MOTOR_PROFILE_TORQUE = 0,
    MOTOR_PROFILE_SPEED,
    MOTOR_PROFILE_CALIBRATE,
    MOTOR_PROFILE_DEFAULT
} MOTOR_PROFILE;

typedef struct Motor_t{
    //DATA
    uint8_t i2c_slave_addr;
    uint32_t currTarget;
    
    uint32_t maxAccel;
    uint32_t maxDecel;
    uint32_t maxSpeed;
    uint32_t speedHomingTowards;
    uint32_t speedHomingAway;
    uint32_t startSpeed;

    uint8_t currentLimit;

    int32_t currPosition;

    TicStepMode stepMode;
    uint8_t microstepFactor;

    MOTOR_PROFILE currProfile;
    bool isOnTarget;
    bool isEnergized;

    uint32_t maxStep;
    uint32_t maxMicrostep;
} Motor;

// 200 steps/rev
typedef enum MotorParams
{
  MotorParams__STEPS_PER_REV = 200,
  MotorParams__NUM_OF_MOTORS = 3
}MotorParams;

extern Motor *motors[MotorParams__NUM_OF_MOTORS];

//used to figure out the number of usable steps between limit switches
// uint32_t calibrate(Motor *motor);
void calibrate_motors(Motor *motors[], size_t num_of_motors);
void wait_for_homes(Motor *motor[], size_t num_of_motors, bool store_max_step); 

void push_object(Motor *motor);
void pull_object(Motor *motor);
void block_object(Motor *motor);

void Debug_Motors(void);
void rotate_revs(Motor *motor, float rotations);
void target_tics(Motor *motor, int enc_tics);
void wait_for_target(Motor *motor);
uint32_t wait_for_home(Motor *motor);

void deenergize(Motor *motor);
void energize(Motor *motor);

float get_max_step(Motor *motor);

void set_target_velocity(Motor *motor, int32_t target_velocity);
void set_target_position(Motor *motor, int32_t new_position);
void set_accel_max(Motor *motor, uint32_t accel_max);
void set_decel_max(Motor *motor, uint32_t decel_max);
void set_speed_max(Motor *motor, uint32_t speed_max);

// void set_speed_homing_towards(Motor *motor, uint32_t speed_toward);
// void set_speed_homing_away(Motor *motor, uint32_t speed_away);


void set_speed_start(Motor *motor, uint32_t speed_start);
void set_step_mode(Motor *motor, uint8_t step_mode);
void set_current_limit(Motor *motor, uint8_t current_limit);
void set_decay_mode(Motor *motor, uint8_t decay_mode);
void set_motor_profile(Motor *motor, MOTOR_PROFILE motor_profile);
void set_angle(Motor *motor, float deg);

uint32_t get_speed_max(Motor *motor);
uint32_t get_speed_start(Motor *motor);
uint32_t get_speed_homing_towards(Motor *motor);
uint32_t get_speed_homing_away(Motor *motor);
uint32_t get_accel_max(Motor *motor);
uint32_t get_decel_max(Motor *motor);
uint8_t get_step_mode(Motor *motor);
uint8_t get_current_limit(Motor *motor);
float get_angle(Motor *motor);
uint8_t get_microstep_factor(Motor *motor);
int32_t get_current_position(Motor *motor);

void go_home_forward(Motor *motor);
void go_home_reverse(Motor *motor);

int Motor_Init_Settings(Motor *motors[], size_t num_of_motors);
void Motor_Init(Motor *motor, uint8_t i2c_slave_addr);

void motor_handler(Motor *motors[], size_t num_of_motors);

#endif