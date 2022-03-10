#ifndef SCRAPSORT_INTEGRATION_QUEUE_H
#define SCRAPSORT_INTEGRATION_QUEUE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct queue {
  int *items;    // Pointer to entire ring buffer of elements (array)
  int produce;   // Index of next `push` location
  int consume;   // Index of next `pop` location
  int num_items; // Number of items in queue
  int max_items; // Maximum number of items (size of ring buffer)
} queue;

void queue__push(queue *q, int item_type);
int queue__pop(queue *q);
int queue__size(queue *q);
bool queue__empty(queue *q);
void queue__print(queue *q);
int queue__peak(queue *q);

queue Queue(int max_items);

#endif // SCRAPSORT_INTEGRATION_QUEUE_H