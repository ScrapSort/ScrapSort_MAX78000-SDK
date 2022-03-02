#include "queue.h"

void queue__push(queue *q, int item_type) {
  if (q->num_items == q->max_items) {
    return;
  }

  q->num_items++;
  q->items[q->produce] = item_type;
  q->produce = (q->produce + 1) % q->max_items;
}

int queue__pop(queue *q) {
  if (!q->num_items) {
    return -1;
  }

  q->num_items--;
  int val = q->items[q->consume];
  q->consume = (q->consume + 1) % q->max_items;

  return val;
}

int queue__size(queue *q) { return q->num_items; }

bool queue__empty(queue *q) { return !q->num_items; }

void queue__print(queue *q) {
  printf("[");

  for (int i = 0; i < q->max_items; q++) {
    printf(" %i", q->items[i]);
  }

  printf(" ]\n");
}

queue Queue(int max_items) {
  queue q;

  // Define variables
  q.produce = 0;
  q.consume = 0;
  q.num_items = 0;
  q.max_items = max_items;

  q.items = malloc(sizeof(int) * (unsigned int)(q.max_items));

  return q;
}