#ifndef SCRAPSORT_INTEGRATION_SORTER_H
#define SCRAPSORT_INTEGRATION_SORTER_H

#include <stdio.h>

#include "queue.h"

typedef struct sorter {
  queue *queues; // Holds array of queues (one per item type / flipper)
  int num_types; // Length of queues array
} sorter;

// Add item type prediction from CNN when first IR sensor detects an object
void sorter__add_item(sorter *s, int item_type);

/*
 * Update queues when a flipper's IR sensor detects an object
 * Returns: true if flipper should activate
 */
bool sorter__detected_item(sorter *s, int flipper);

void sorter__print(sorter *s);

sorter Sorter(int num_types);

#endif // SCRAPSORT_INTEGRATION_SORTER_H
