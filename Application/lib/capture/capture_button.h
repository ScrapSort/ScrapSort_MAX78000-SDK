// This file defines the initialization function for the capture button

#ifndef CAPTURE_BUTTON_H
#define CAPTURE_BUTTON_H

/*
    Description: This function sets up the GPIO for the push button
    Parameters: none
    Return: none
*/
void init_capture_button();

void init_class_button();

void capture();
void switch_class();

int get_capture_state();

int get_switch_state();

void trigger_cam();
void trigger();

void init_triggers();
#endif