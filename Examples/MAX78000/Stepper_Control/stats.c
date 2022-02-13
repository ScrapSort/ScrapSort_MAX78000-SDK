#include "stats.h"

void stats__added_item(stats *s, int item_type) {
  if (s->num_types < item_type - 1)
    return;

  s->num_items_active[item_type]++;
}

void stats__binned_item(stats *s, int item_type) {
  if (s->num_types < item_type - 1 || !s->num_items_active[item_type])
    return;

  s->num_items_active[item_type]--;
  s->num_items_binned[item_type]++;
}

stats Stats(int num_types) {
  stats s;

  s.num_types = num_types;
  s.num_items_active = malloc(sizeof(int) * (unsigned int)(num_types - 1));
  s.num_items_binned = malloc(sizeof(int) * (unsigned int)(num_types - 1));

  for (int i = 0; i < num_types; i++) {
    s.num_items_active[i] = 0;
    s.num_items_binned[i] = 0;
  }

  return s;
}
