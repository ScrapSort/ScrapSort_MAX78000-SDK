#ifndef SCRAPSORT_INTEGRATION_QUEUE_NODE_H
#define SCRAPSORT_INTEGRATION_QUEUE_NODE_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct queue_node {
  int item_type;
  struct queue_node *next;

  void (*queue_node)(bool);
} queue_node;

queue_node *QueueNode(int item_type);

#endif // SCRAPSORT_INTEGRATION_QUEUE_NODE_H
