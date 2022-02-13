#include "queue.h"
#include "queue_node.h"

void queue__push(queue *q, int item_type) {
  q->num_nodes++;

  if (!q->front) {
    q->front = q->back = QueueNode(item_type);
  } else {
    q->back->next = QueueNode(item_type);
    q->back = q->back->next;
  }
}

int queue__pop(queue *q) {
  if (!q->num_nodes) {
    return -1;
  }

  int val = q->front->item_type;
  q->num_nodes--;

  if (!q->front->next) {
    free(q->front);
    q->front = q->back = NULL;
  } else {
    struct queue_node *tmp = q->front;
    q->front = q->front->next;
    free(tmp);
  }

  return val;
}

int queue__size(queue *q) { return q->num_nodes; }

bool queue__empty(queue *q) { return !q->num_nodes; }

queue Queue() {
  queue q;

  // Define variables
  q.front = NULL;
  q.back = NULL;
  q.num_nodes = 0;

  return q;
}
