#ifndef MOTOR_FUNCS_H
#define MOTOR_FUNCS_H

void Debug_Motors(void);
void rotate_revs(int slave_addr, float rotations);
void target_tics(int slave_addr, int enc_tics);
int Motor_Init_Settings();


#endif