#ifndef SCRAPSORT_INTEGRATION_QUEUE_H
#define SCRAPSORT_INTEGRATION_QUEUE_H

#include "queue_node.h"

typedef struct queue {
  queue_node *front;
  queue_node *back;
  int num_nodes;
} queue;

void queue__push(queue *q, int item_type);
int queue__pop(queue *q);
int queue__size(queue *q);
bool queue__empty(queue *q);

queue Queue();

#endif // SCRAPSORT_INTEGRATION_QUEUE_H
