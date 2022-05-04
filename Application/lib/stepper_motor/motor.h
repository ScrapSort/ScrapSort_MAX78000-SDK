#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>
#include "tic.h"

typedef struct Motor_t{
    //DATA
    uint8_t i2c_slave_addr;
    uint32_t currTarget;
    
    uint32_t maxAccel;
    uint32_t maxDecel;
    uint32_t maxSpeed;
    uint32_t startSpeed;

    uint8_t currentLimit;

    uint32_t currPosition;

    TicStepMode stepMode;
    uint8_t microstepFactor;

    MOTOR_PROFILE currProfile;
    bool isOnTarget;
    bool isEnergized;

} Motor;



typedef enum
{
    MOTOR_PROFILE_TORQUE = 0,
    MOTOR_PROFILE_SPEED,
    MOTOR_PROFILE_CALIBRATE
} MOTOR_PROFILE;

// 200 steps/rev
typedef enum MotorParams
{
  MotorParams__STEPS_PER_REV = 200
}MotorParams;

void Debug_Motors(void);
void rotate_revs(Motor *motor, float rotations);
void target_tics(Motor *motor, int enc_tics);
void wait_for_target(Motor *motor);

void deenergize(Motor *motor);
void energize(Motor *motor);

float get_max_microstep(Motor *motor);

void set_target_velocity(Motor *motor, uint32_t target_velocity);
void set_accel_max(Motor *motor, uint32_t accel_max);
void set_decel_max(Motor *motor, uint32_t decel_max);
void set_speed_max(Motor *motor, uint32_t speed_max);
void set_speed_start(Motor *motor, uint32_t speed_start);
void set_step_mode(Motor *motor, uint8_t step_mode);
void set_current_limit(Motor *motor, uint8_t current_limit);
void set_decay_mode(Motor *motor, uint8_t decay_mode);
void set_motor_profile(Motor *motor, MOTOR_PROFILE motor_profile);

void go_home_forward(Motor *motor);
void go_home_reverse(Motor *motor);

int Motor_Init_Settings();


#endif