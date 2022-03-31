#include "flags.h"

// stores the flag bits
volatile uint32_t flags = 0;

void set_flag(Flag f) { flags |= (uint32_t)1 << f; }

void unset_flag(Flag f) { flags &= ~(uint32_t)1 << f; }

bool is_flag_set(Flag f) { return flags & ((uint32_t)1 << f); }