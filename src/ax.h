#ifndef AX_H_
#define AX_H_

struct ax {
  bool is_privileged;

  AXUIElementRef element;
};

struct ax g_ax;
void ax_init(struct ax* ax);
void ax_clear(struct ax* ax);
void ax_get_menu_item(struct ax* ax, pid_t pid, char* name);

#endif // !AX_H_

