#include "sorter.h"
#include "queue.h"

void sorter__add_item(sorter *s, int item_type) {
  for (int i = 0; i < item_type; i++) {
    queue__push(&s->queues[i], item_type);
  }

  queue__push(&s->queues[item_type], item_type);
}

bool sorter__detected_item(sorter *s, int flipper) {
  return queue__pop(&s->queues[flipper]) == flipper;
}

void sorter__print(sorter *s) {
  for (int i = 0; i < s->num_types; i++) {
    printf("%i:\t", i);
    queue__print(&s->queues[i]);
  }
}

sorter Sorter(int num_types) {
  sorter s;

  s.num_types = num_types;
  s.queues = malloc(sizeof(queue) * (unsigned int)(num_types));

  for (int i = 0; i < num_types; i++) {
    s.queues[i] = Queue();
  }

  return s;
}
