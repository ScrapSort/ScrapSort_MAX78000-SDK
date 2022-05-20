#ifndef SCRAPSORT_INTEGRATION_E_QUEUE_H
#define SCRAPSORT_INTEGRATION_E_QUEUE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct e_queue {
  int *items;    // Pointer to entire ring buffer of elements (array)
  int produce;   // Index of next `push` location
  int consume;   // Index of next `pop` location
  int num_items; // Number of items in e_queue
  int max_items; // Maximum number of items (size of ring buffer)
} e_queue;

void e_queue__push(e_queue *q, int item_type);
int e_queue__pop(e_queue *q);
int e_queue__size(e_queue *q);
bool e_queue__empty(e_queue *q);
void e_queue__print(e_queue *q);
int e_queue__peak(e_queue *q);

e_queue e_queue(int max_items);

#endif // SCRAPSORT_INTEGRATION_e_queue_H