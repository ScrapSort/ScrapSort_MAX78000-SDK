#ifndef MOTOR_FUNCS_H
#define MOTOR_FUNCS_H

typedef enum
{
    MOTOR_PROFILE_TORQUE = 0,
    MOTOR_PROFILE_SPEED
} MOTOR_PROFILE;

void Debug_Motors(void);
void rotate_revs(int slave_addr, float rotations);
void target_tics(int slave_addr, int enc_tics);
void deenergize(uint8_t slave_addr);
void energize(uint8_t slave_addr);
void set_target_velocity(uint8_t slave_addr, uint32_t target_velocity);
void set_accel_max(uint8_t slave_addr, uint32_t accel_max);
void set_decel_max(uint8_t slave_addr, uint32_t decel_max);
void set_speed_max(uint8_t slave_addr, uint32_t speed_max);
void set_speed_start(uint8_t slave_addr, uint32_t speed_start);
void set_step_mode(uint8_t slave_addr, uint8_t step_mode);
void set_current_limit(uint8_t slave_addr, uint8_t current_limit);
void set_decay_mode(uint8_t slave_addr, uint8_t decay_mode);
void set_motor_profile(uint8_t slave_addr, MOTOR_PROFILE motor_profile);
int Motor_Init_Settings();


#endif