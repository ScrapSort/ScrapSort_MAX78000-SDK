#include "e_queue_node.h"
#include "stdio.h"

e_queue_node *e_queueNode(int item_type) {
  //printf("malloc\n");
  e_queue_node *qn = malloc(sizeof(e_queue_node));
  //printf("mallocd: %p\n",(void*)qn);

  // Define variables
  qn->item_type = item_type;

  return qn;
}
