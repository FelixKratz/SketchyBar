/* Unique argv0. Offscreen. Drives official production callers. Not a live bar. */
#include "bar.h"
#include "bar_item.h"
#include "bar_manager.h"
#include "group.h"
#include "mach.h"
#include "misc/helpers.h"
#include "popup.h"
#include "window.h"

#include <CoreFoundation/CoreFoundation.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern int SLSMainConnectionID(void);

int g_connection;
CFTypeRef g_transaction;
int g_space_management_mode;
struct bar_manager g_bar_manager;
struct mach_server g_mach_server;
void *g_workspace_context;
char g_name[256];
char g_config_file[4096];
char g_lock_file[MAXLEN];
bool g_volume_events;
bool g_brightness_events;
int64_t g_disable_capture;
pid_t g_pid;
CGError (*SBSLSTransactionAddPostDecodeAction)(CFTypeRef, void (^)()) = NULL;

/* Drawn items: two per arrangement, plus one each for the two
   display-associated items and the two open-popup children. */
#define EXPECTED_ITEM_WINDOWS 18

#define OFF_X (-9999.f)
#define OFF_Y (-9999.f)

static int g_fails;

static void die_timeout(int sig) {
  (void)sig;
  fprintf(stderr, "TEST_TIMEOUT\n");
  _exit(2);
}

static void fail(const char* msg) {
  fprintf(stderr, "FAIL %s\n", msg);
  g_fails++;
}

static void expect_eq(const char* msg, int got, int want) {
  if (got != want) {
    fprintf(stderr, "FAIL %s: got %d want %d\n", msg, got, want);
    g_fails++;
  }
}

static int item_window_count(struct bar_item* item) {
  int n = 0;
  if (!item || !item->windows) return 0;
  for (uint32_t i = 0; i < item->num_windows; i++) {
    if (item->windows[i] && item->windows[i]->id) n++;
  }
  return n;
}

static int slot_count(struct bar_item* item) {
  int n = 0;
  if (!item || !item->windows) return 0;
  for (uint32_t i = 0; i < item->num_windows; i++) {
    if (item->windows[i]) n++;
  }
  return n;
}

static struct bar* make_bar(uint32_t adid) {
  struct bar* bar = calloc(1, sizeof(struct bar));
  bar->shown = true;
  bar->hidden = false;
  bar->sid = 1;
  bar->adid = adid;
  bar->did = 1;
  window_init(&bar->window);
  window_open(&bar->window, (CGRect){{OFF_X, OFF_Y - (CGFloat)(adid * 40)}, {400, 25}});
  return bar;
}

static struct bar_item* add_item(const char* name) {
  struct bar_item* item = bar_item_create();
  bar_item_init(item, &g_bar_manager.default_item);
  bar_item_set_name(item, string_copy((char*)name));
  item->drawing = true;
  item->position = POSITION_LEFT;
  g_bar_manager.bar_items = realloc(g_bar_manager.bar_items,
                                    sizeof(struct bar_item*)
                                    * (g_bar_manager.bar_item_count + 1));
  g_bar_manager.bar_items[g_bar_manager.bar_item_count++] = item;
  return item;
}

static void refresh_all(void) {
  g_bar_manager.needs_ordering = true;
  for (int i = 0; i < (int)g_bar_manager.bar_count; i++) {
    bar_calculate_bounds(g_bar_manager.bars[i]);
    bar_draw(g_bar_manager.bars[i], false);
    if (g_bar_manager.needs_ordering)
      bar_order_item_windows(g_bar_manager.bars[i]);
  }
  g_bar_manager.needs_ordering = false;
  g_bar_manager.bar_needs_update = false;
}

static void clip_pass(void) {
  g_bar_manager.might_need_clipping = true;
  g_bar_manager.bar_needs_update = false;
  for (int i = 0; i < (int)g_bar_manager.bar_count; i++)
    bar_draw(g_bar_manager.bars[i], false);
}

static void close_item_windows(struct bar_item* item) {
  for (uint32_t adid = 1; adid <= item->num_windows; adid++)
    bar_item_remove_window(item, adid);
}

int main(void) {
  alarm(15);
  signal(SIGALRM, die_timeout);

  memset(&g_bar_manager, 0, sizeof(g_bar_manager));
  g_bar_manager.sticky = false;
  g_bar_manager.position = POSITION_TOP;
  g_bar_manager.window_level = kCGBackstopMenuLevel;
  g_bar_manager.active_adid = 1;
  g_bar_manager.background.bounds.size.height = 25;
  g_bar_manager.background.padding_left = 4;
  g_bar_manager.background.padding_right = 4;
  bar_item_init(&g_bar_manager.default_item, NULL);

  g_connection = SLSMainConnectionID();
  if (!g_connection) {
    fprintf(stderr, "FAIL no SLS connection\n");
    return 1;
  }

  g_bar_manager.bar_count = 2;
  g_bar_manager.bars = calloc(2, sizeof(struct bar*));
  g_bar_manager.bars[0] = make_bar(1);
  g_bar_manager.bars[1] = make_bar(2);

  struct bar_item* hidden[4];
  for (int i = 0; i < 4; i++) {
    char name[16];
    snprintf(name, sizeof(name), "hid%d", i);
    hidden[i] = add_item(name);
    hidden[i]->drawing = false;
    hidden[i]->associated_bar = 0x3;
  }

  struct bar_item* assoc1 = add_item("assoc1");
  assoc1->associated_display = 1u << 1;
  struct bar_item* assoc2 = add_item("assoc2");
  assoc2->associated_display = 1u << 2;

  struct bar_item* closed_host = add_item("closed_host");
  struct bar_item* closed_a = add_item("closed_a");
  struct bar_item* closed_b = add_item("closed_b");
  closed_a->position = POSITION_POPUP;
  closed_b->position = POSITION_POPUP;
  popup_add_item(&closed_host->popup, closed_a);
  popup_add_item(&closed_host->popup, closed_b);
  closed_host->popup.drawing = false;
  closed_host->popup.adid = 0;

  struct bar_item* open_host = add_item("open_host");
  struct bar_item* open_a = add_item("open_a");
  struct bar_item* open_b = add_item("open_b");
  open_a->position = POSITION_POPUP;
  open_b->position = POSITION_POPUP;
  popup_add_item(&open_host->popup, open_a);
  popup_add_item(&open_host->popup, open_b);
  open_host->popup.drawing = true;
  open_host->popup.adid = 1;
  open_host->popup.anchor = (CGPoint){OFF_X, OFF_Y};
  open_host->popup.needs_ordering = true;

  struct bar_item* drawn[2];
  drawn[0] = add_item("drawn0");
  drawn[1] = add_item("drawn1");

  struct bar_item* bracket = add_item("bracket");
  bracket->type = BAR_COMPONENT_GROUP;
  bracket->group = group_create();
  group_init(bracket->group);
  group_add_member(bracket->group, bracket);
  struct bar_item* gm1 = add_item("gm1");
  struct bar_item* gm2 = add_item("gm2");
  group_add_member(bracket->group, gm1);
  group_add_member(bracket->group, gm2);

  refresh_all();
  popup_calculate_bounds(&open_host->popup, g_bar_manager.bars[0]);
  popup_draw(&open_host->popup);
  clip_pass();

  struct window* hit_window = NULL;
  bar_manager_get_item_by_point(&g_bar_manager, (CGPoint){OFF_X, OFF_Y}, &hit_window);
  bar_manager_get_item_by_wid(&g_bar_manager, 0x7fffffffu, &hit_window);

  int hidden_windows = 0;
  for (int i = 0; i < 4; i++) {
    hidden_windows += item_window_count(hidden[i]);
    if (hidden[i]->associated_bar != 0)
      fail("hidden still associated");
  }
  expect_eq("drawing-off windows", hidden_windows, 0);

  expect_eq("assoc1 windows", item_window_count(assoc1), 1);
  expect_eq("assoc2 windows", item_window_count(assoc2), 1);
  if (assoc1->windows && assoc1->windows[0] && !assoc1->windows[0]->id)
    fail("assoc1 slot0 not created");
  if (assoc2->num_windows >= 2 && assoc2->windows[1] && assoc2->windows[1]->id) {
    /* adid 2 is slot 1 */
  } else if (item_window_count(assoc2) != 1) {
    fail("assoc2 missing adid2 window");
  }

  expect_eq("closed popup child a", item_window_count(closed_a), 0);
  expect_eq("closed popup child b", item_window_count(closed_b), 0);
  expect_eq("closed host windows", item_window_count(closed_host), 2);

  expect_eq("open popup child a", item_window_count(open_a), 1);
  expect_eq("open popup child b", item_window_count(open_b), 1);
  expect_eq("open host windows", item_window_count(open_host), 2);

  expect_eq("drawn0 windows", item_window_count(drawn[0]), 2);
  expect_eq("drawn1 windows", item_window_count(drawn[1]), 2);
  expect_eq("group member1", item_window_count(gm1), 2);
  expect_eq("group member2", item_window_count(gm2), 2);
  expect_eq("group bracket", item_window_count(bracket), 2);

  int after_first = 0;
  for (int i = 0; i < (int)g_bar_manager.bar_item_count; i++)
    after_first += item_window_count(g_bar_manager.bar_items[i]);

  /* Only items a bar actually draws own a window. Stock allocates for every
     item on every arrangement, including drawing-off, unassociated and
     closed-popup items. */
  expect_eq("total item windows", after_first, EXPECTED_ITEM_WINDOWS);

  refresh_all();
  clip_pass();
  popup_draw(&open_host->popup);
  int after_second = 0;
  for (int i = 0; i < (int)g_bar_manager.bar_item_count; i++)
    after_second += item_window_count(g_bar_manager.bar_items[i]);
  expect_eq("allocate once", after_second, after_first);

  hidden[0]->drawing = true;
  refresh_all();
  expect_eq("reveal hidden[0]", item_window_count(hidden[0]), 2);

  uint32_t drawn0_id = drawn[0]->windows[0]->id;
  CGPoint drawn0_origin = drawn[0]->windows[0]->origin;
  drawn[0]->drawing = false;
  drawn[0]->associated_bar = 0x3;
  refresh_all();
  expect_eq("hide keeps window", item_window_count(drawn[0]), 2);
  if (drawn[0]->associated_bar != 0) fail("hide did not clear association");
  if (drawn[0]->windows[0]->id != drawn0_id) fail("hide recreated window");
  (void)drawn0_origin;
  if (!CGPointEqualToPoint(drawn[0]->windows[0]->origin, g_nirvana))
    fail("hide did not park the window at nirvana");

  assoc1->associated_display = 1u << 2;
  int assoc1_before = item_window_count(assoc1);
  refresh_all();
  expect_eq("reassoc creates other adid", item_window_count(assoc1), assoc1_before + 1);

  uint32_t removed_id = drawn[1]->windows[0] ? drawn[1]->windows[0]->id : 0;
  bar_item_remove_window(drawn[1], 1);
  if (drawn[1]->windows[0]) fail("removed slot still set");
  expect_eq("removed keeps other adid", slot_count(drawn[1]), 1);
  int slots_before_hit = slot_count(drawn[1]);
  struct window* out = NULL;
  struct bar_item* hit = bar_manager_get_item_by_point(&g_bar_manager,
                                                       (CGPoint){0, 0},
                                                       &out);
  (void)hit;
  hit = bar_manager_get_item_by_wid(&g_bar_manager, removed_id, &out);
  (void)hit;
  g_bar_manager.needs_ordering = true;
  bar_order_item_windows(g_bar_manager.bars[0]);
  expect_eq("hit/order cannot recreate", slot_count(drawn[1]), slots_before_hit);
  if (drawn[1]->windows[0]) fail("hit/order recreated slot");

  drawn[1]->drawing = false;
  clip_pass();
  expect_eq("hidden clip cannot recreate", slot_count(drawn[1]), slots_before_hit);
  if (drawn[1]->windows[0]) fail("clip recreated slot");

  hidden[1]->drawing = false;
  hidden[1]->associated_bar = 0x3;
  refresh_all();
  if (hidden[1]->associated_bar != 0) fail("null peek still must clear association");
  expect_eq("still-hidden stays zero", item_window_count(hidden[1]), 0);

  printf("item_windows_first=%d item_windows_stable=%d\n", after_first, after_second);
  printf("hidden=%d assoc1=%d assoc2=%d closed_children=%d,%d open_children=%d,%d drawn=%d,%d group=%d,%d,%d\n",
         hidden_windows,
         item_window_count(assoc1),
         item_window_count(assoc2),
         item_window_count(closed_a),
         item_window_count(closed_b),
         item_window_count(open_a),
         item_window_count(open_b),
         item_window_count(drawn[0]),
         item_window_count(drawn[1]),
         item_window_count(bracket),
         item_window_count(gm1),
         item_window_count(gm2));

  for (int i = 0; i < (int)g_bar_manager.bar_item_count; i++)
    close_item_windows(g_bar_manager.bar_items[i]);
  if (open_host->popup.window.id) window_close(&open_host->popup.window);
  if (closed_host->popup.window.id) window_close(&closed_host->popup.window);
  window_close(&g_bar_manager.bars[0]->window);
  window_close(&g_bar_manager.bars[1]->window);

  if (g_fails) {
    fprintf(stderr, "%d failure(s)\n", g_fails);
    return 1;
  }
  printf("PASS lazy-item-windows\n");
  return 0;
}
