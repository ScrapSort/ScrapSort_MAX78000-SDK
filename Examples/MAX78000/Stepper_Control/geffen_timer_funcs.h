// This file defines helper functions for
// interacting with timers

#ifndef GEFFEN_TIMER_FUNCS_H
#define GEFFEN_TIMER_FUNCS_H

#include "flags.h"

/*
    Description: This function sets up a continuous timer using the Maxim
                 integrated drivers. When the timer expires (every 1 sec) an
                 interrupt will be generated. Once the timer expires, 
                 the start function needs to be called again to reenable it.
    Parameters: The expiration period in seconds (ex: 5 means an interrupt
                is generated after 5 seconds).
    Return: -1 if fails, 0 if successful
*/
int init_timer(int expiration_period, flag_callback cb_func);


/*
    Description: This function is used to change the expiration period
                 for the timer. This can be used to set different
                 periods for different states.
    Parameters: The expiration period in seconds (ex: 5 means an interrupt
                is generated after 5 seconds).
    Return: none
*/
void set_expiration_period(int expiration_period);


/*
    Description: This function returns the current timer expiration period in seconds
    Parameters: none
    Return: the current timer expiration period in second
*/
int get_expiration_period();


/*
    Description: This function can be used to get the amount of time
                 left in seconds before the timer expires.
    Parameters: none
    Return: time in seconds until the state expires
*/
int get_time_left();


/*
    Description: This function resets the count to 0 and disables th timer
    Parameters: none
    Return: none
*/
void reset_state_timer();


/*
    Description: This function starts the timer and should be called
                 after init or after reset.
    Parameters: none
    Return: none
*/
void start_state_timer();


/*
    Description: This function disables (pauses) the timer but does not reset the count.
    Parameters: none
    Return: none
*/
void stop_state_timer();

#endif