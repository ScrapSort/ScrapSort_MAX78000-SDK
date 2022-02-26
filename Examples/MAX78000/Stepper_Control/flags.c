#include "flags.h"

// stores the flag bits
volatile int flags = 0;

void set_flag(Flag f) { flags |= 1 << f; }

void unset_flag(Flag f) { flags &= ~1 << f; }

bool is_flag_set(Flag f) { return flags & (1 << f); }