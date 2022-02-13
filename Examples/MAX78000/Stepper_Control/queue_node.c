#include "queue_node.h"

queue_node *QueueNode(int item_type) {
  queue_node *qn = malloc(sizeof(queue_node));

  // Define variables
  qn->item_type = item_type;

  return qn;
}
