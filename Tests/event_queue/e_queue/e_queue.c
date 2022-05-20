#include "e_queue.h"

void e_queue__push(e_queue *q, int item_type) {
  if (q->num_items == q->max_items) {
    return;
  }

  q->num_items++;
  q->items[q->produce] = item_type;
  q->produce = (q->produce + 1) % q->max_items;
}

int e_queue__pop(e_queue *q) {
  if (!q->num_items) {
    return -1;
  }

  q->num_items--;
  int val = q->items[q->consume];
  q->consume = (q->consume + 1) % q->max_items;

  return val;
}

int e_queue__size(e_queue *q) { return q->num_items; }

bool e_queue__empty(e_queue *q) { return !q->num_items; }

void e_queue__print(e_queue *q) {
  printf("[");

  for (int i = 0; i < q->max_items; q++) {
    printf(" %i", q->items[i]);
  }

  printf(" ]\n");
}

int e_queue__peak(e_queue *q)
{
  if(!q->num_items){
    return -1;
  }
  return q->items[q->consume];
}

e_queue e_queue(int max_items) {
  e_queue q;

  // Define variables
  q.produce = 0;
  q.consume = 0;
  q.num_items = 0;
  q.max_items = max_items;

  q.items = malloc(sizeof(int) * (unsigned int)(q.max_items));

  return q;
}