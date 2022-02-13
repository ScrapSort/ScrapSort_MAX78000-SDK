#ifndef SCRAPSORT_INTEGRATION_STATS_H
#define SCRAPSORT_INTEGRATION_STATS_H

#include <stdlib.h>

typedef struct stats {
  int *num_items_active; // Holds number of items that are currently on the conveyor belt
  int *num_items_binned; // Holds number of items that have been sorted
  int num_types;
} stats;

void stats__added_item(stats *s, int item_type);
void stats__binned_item(stats *s, int item_type);

stats Stats(int num_types);

#endif // SCRAPSORT_INTEGRATION_STATS_H
