/*
Title: dexarm.h

This file contains functions to abstract the g-code commands sent to 
a Rotrics Dexarm.

Author: Vincent Benenati 
Date: 1/11/21
*/
#ifndef DEXARM
#define DEXARM

void init();
void send_cmd(char* cmd, int wait_ok);
void go_home();
void set_work_origin();
void set_acceleration(int acceleration, int travel_acceleration, int retract_acceleration);
void set_module_type();
void get_module_type();
void move_to(int x, int y, int z, int e, int feedrate, char* mode, int wait);
void get_current_position();
void delay_ms();
void delay_s();
void soft_gripper_pick();
void soft_gripper_place();
void soft_gripper_stop();
void soft_gripper_neutral();
void air_picker_pick();
void air_picker_place();
void air_picker_neutral();
void air_picker_stop();
void close();

#endif