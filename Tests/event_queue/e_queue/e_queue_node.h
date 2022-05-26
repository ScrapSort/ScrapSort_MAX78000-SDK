#ifndef SCRAPSORT_INTEGRATION_E_QUEUE_NODE_H
#define SCRAPSORT_INTEGRATION_E_QUEUE_NODE_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct e_queue_node {
  int item_type;
  struct e_queue_node *next;

  void (*e_queue_node)(bool);
} e_queue_node;

e_queue_node *e_queueNode(int item_type);

#endif // SCRAPSORT_INTEGRATION_e_queue_NODE_H
