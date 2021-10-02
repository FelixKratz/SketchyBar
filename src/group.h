#ifndef GROUP_H_
#define GROUP_H_

#include <_types/_uint32_t.h>

struct group {
  char* name;

  struct background background;
};

struct group* group_create();
void group_init(struct group* group);
void group_set_name(struct group* group, char* _name);
void group_destroy(struct group* group);

#endif
