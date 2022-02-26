#include "queue_node.h"
#include "stdio.h"

queue_node *QueueNode(int item_type) {
  printf("malloc\n");
  queue_node *qn = malloc(sizeof(queue_node));
  printf("mallocd: %p\n",(void*)qn);

  // Define variables
  qn->item_type = item_type;

  return qn;
}
