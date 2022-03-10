#ifndef SCRAPSORT_INTEGRATION_FLAGS_H
#define SCRAPSORT_INTEGRATION_FLAGS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// interrupt flag identifiers (type)
typedef enum Flag 
{
  Flipper0,
  Flipper1,
  Flipper2,
  Flipper3,
  Flipper4,
  Flipper5,
  Camera,
  Close,
  num_flags
} Flag;

// used to set the bits for a given flag (interrupt happened)
void set_flag(Flag f);

// used to reset the bits for a given flag (interrupt handled)
void unset_flag(Flag f);

// check if the bits for a given flag are set (check if interrupt happened)
bool is_flag_set(Flag f);

// the callback function signature for the interrupts (call function of this type in main)
typedef void (*flag_callback)(void);

#endif // SCRAPSORT_INTEGRATION_FLAGS_H