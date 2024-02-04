#include "app_windows.h"
#include "workspace.h"
#include "misc/helpers.h"
#include "event.h"

extern pid_t g_pid;
struct app_windows g_windows = { 0 };
struct app_windows g_hidden_windows = { 0 };
bool g_space_window_events = false;

static bool iterator_window_suitable(CFTypeRef iterator) {
  uint64_t tags = SLSWindowIteratorGetTags(iterator);
  uint64_t attributes = SLSWindowIteratorGetAttributes(iterator);
  uint32_t parent_wid = SLSWindowIteratorGetParentID(iterator);
  if (((parent_wid == 0)
        && ((attributes & 0x2)
          || (tags & 0x400000000000000))
        && (((tags & 0x1))
          || ((tags & 0x2)
            && (tags & 0x80000000))))) {
    return true;
  }
  return false;
}

void app_window_clear(struct app_window* window) {
  memset(window, 0, sizeof(struct app_window));
}

void app_windows_add(struct app_windows* windows, struct app_window* window) {
  for (int i = 0; i < windows->num_windows; i++) {
    if (!windows->windows[i].wid) {
      windows->windows[i] = *window;
      return;
    }
  }

  windows->windows = realloc(windows->windows,
                             sizeof(struct app_window)
                             * ++windows->num_windows);

  windows->windows[windows->num_windows - 1] = *window;
}

void app_windows_clear_space(struct app_windows* windows, uint64_t sid) {
  for (int i = 0; i < windows->num_windows; i++) {
    if (windows->windows[i].sid == sid) app_window_clear(windows->windows + i);
  }
}

void app_windows_register_notifications() {
  uint32_t window_count = 0;
  uint32_t wid_list[g_windows.num_windows + g_hidden_windows.num_windows];
  for (int i = 0; i < g_windows.num_windows; i++) {
    if (g_windows.windows[i].wid)
      wid_list[window_count++] = g_windows.windows[i].wid;
  }

  for (int i = 0; i < g_hidden_windows.num_windows; i++) {
    if (g_hidden_windows.windows[i].wid)
      wid_list[window_count++] = g_hidden_windows.windows[i].wid;
  }
  SLSRequestNotificationsForWindows(g_connection, wid_list, window_count);
}

bool app_windows_find(struct app_windows* windows, struct app_window* window) {
  for (int i = 0; i < windows->num_windows; i++) {
    if (windows->windows[i].wid == window->wid
        && windows->windows[i].sid == window->sid) {
      return true;
    }
  }
  return false;
}

struct app_window* app_windows_find_by_wid(struct app_windows* windows, uint32_t wid) {
  for (int i = 0; i < windows->num_windows; i++) {
    if (windows->windows[i].wid == wid) return &windows->windows[i];
  }
  return NULL;
}

static bool app_window_suitable(struct app_window* window) {
  CFArrayRef target_ref = cfarray_of_cfnumbers(&window->wid,
                                               sizeof(uint32_t),
                                               1,
                                               kCFNumberSInt32Type);

  if (!target_ref) return false;

  bool suitable = false;
  CFTypeRef query = SLSWindowQueryWindows(g_connection, target_ref, 0x0);
  if (query) {
    CFTypeRef iterator = SLSWindowQueryResultCopyWindows(query);
    if (iterator && SLSWindowIteratorGetCount(iterator) > 0) {
      if (SLSWindowIteratorAdvance(iterator)) {
        if (iterator_window_suitable(iterator)) suitable = true;
      }
    }
    if (iterator) CFRelease(iterator);
    CFRelease(query);
  }
  CFRelease(target_ref);
  return suitable;
}

static void app_windows_post_event_for_space(struct app_windows* windows, uint64_t sid) {
  int index = mission_control_index(sid);
  pid_t pid_list[windows->num_windows];
  uint32_t pid_count[windows->num_windows];
  char* pid_name[windows->num_windows];

  memset(&pid_list, 0, sizeof(pid_t)*windows->num_windows);
  memset(&pid_count, 0, sizeof(uint32_t)*windows->num_windows);
  memset(&pid_name, 0, sizeof(char*)*windows->num_windows);

  uint32_t length = 64;
  for (int i = 0; i < windows->num_windows; i++) {
    for (int j = 0; j < windows->num_windows; j++) {
      if ((!pid_list[j] || pid_list[j] == windows->windows[i].pid)
          && windows->windows[i].sid == sid) {
        pid_list[j] = windows->windows[i].pid;
        pid_count[j]++;
        if (!pid_name[j]) {
          pid_name[j] = workspace_copy_app_name_for_pid(pid_list[j]);
          length += pid_name[j] ? (strlen(pid_name[j]) + 16) : 0;
        }
        break;
      }
    }
  }

  for (int i = 0; i < windows->num_windows; i++) {
    for (int j = i + 1; j < windows->num_windows; j++) {
      if (pid_name[i]
          && pid_name[j]
          && strcmp(pid_name[i], pid_name[j]) == 0) {
        free(pid_name[j]);
        pid_name[j] = NULL;

        pid_count[i] += pid_count[j];
      }
    }
  }

  char payload[length];
  memset(payload, 0, length);
  snprintf(payload, length, "{\n"
                            "\t\"space\": %d,\n"
                            "\t\"apps\": {\n",
                            index               );

  char* cursor = payload + strlen(payload);
  bool first = true;
  for (int i = 0; i < windows->num_windows; i++) {
    if (!pid_list[i]) break; 
    if (!pid_name[i]) continue;
    if (!first) {
      snprintf(cursor, length - (cursor - payload), ",\n");
      cursor = payload + strlen(payload);
    } else first = false;
    snprintf(cursor,
             length - (cursor - payload),
             "\t\t\"%s\": %d",
             pid_name[i],
             pid_count[i]                );

    free(pid_name[i]);
    cursor = payload + strlen(payload);
  }

  snprintf(cursor, length - (cursor - payload), "\n\t}\n}\n");
  struct event event = { payload, SPACE_WINDOWS_CHANGED };
  event_post(&event);
}

static void app_windows_update_space(struct app_windows* windows, uint64_t sid, bool silent) {
  app_windows_clear_space(windows, sid);
  CFArrayRef space_list_ref = cfarray_of_cfnumbers(&sid,
                                                   sizeof(uint64_t),
                                                   1,
                                                   kCFNumberSInt64Type);

  uint64_t set_tags = 1;
  uint64_t clear_tags = 0;
  CFArrayRef window_list = SLSCopyWindowsWithOptionsAndTags(g_connection,
                                                            0,
                                                            space_list_ref,
                                                            0x2,
                                                            &set_tags,
                                                            &clear_tags    );

  if (window_list) {
    uint32_t window_count = CFArrayGetCount(window_list);
    if (window_count > 0) {
      CFTypeRef query = SLSWindowQueryWindows(g_connection, window_list, 0x0);
      if (query) {
        CFTypeRef iterator = SLSWindowQueryResultCopyWindows(query);
        if (iterator) {
          while(SLSWindowIteratorAdvance(iterator)) {
            if (iterator_window_suitable(iterator)) {
              uint32_t wid = SLSWindowIteratorGetWindowID(iterator);
              int wid_cid = 0;
              SLSGetWindowOwner(g_connection, wid, &wid_cid);

              pid_t pid = 0;
              SLSConnectionGetPID(wid_cid, &pid);
              struct app_window window = {.wid = wid, .sid = sid, .pid = pid};
              app_windows_add(windows, &window);
            }
          }
          CFRelease(iterator);
        }
        CFRelease(query);
      }
    }
    CFRelease(window_list);
  }

  CFRelease(space_list_ref);
  if (!silent) app_windows_post_event_for_space(windows, sid);
  app_windows_register_notifications();
}

struct window_spawn_data {
  uint64_t sid;
  uint32_t wid;
};

static void window_spawn_handler(uint32_t event, struct window_spawn_data* data, size_t _, int cid) {
  uint32_t wid = data->wid;
  uint64_t sid = data->sid;

  if (!wid || !sid) return;

  struct app_window window = { .wid = wid, .sid = sid, .pid = 0 };

  if (event == 1325 && app_window_suitable(&window)) {
    app_windows_update_space(&g_windows, sid, false);
  } else if (event == 1326 && app_windows_find(&g_windows, &window)) {
    app_windows_update_space(&g_windows, sid, false);
    struct app_window* window = app_windows_find_by_wid(&g_hidden_windows,
                                                        wid               );
    if (window) app_window_clear(window);
  }
}

static void window_hide_handler(uint32_t event, uint32_t* window_id, size_t _, int cid) {
  uint32_t wid = *window_id;

  if (event == 816) {
    struct app_window* window = app_windows_find_by_wid(&g_windows, wid);
    if (window) {
      if (!app_windows_find(&g_hidden_windows, window)) {
        app_windows_add(&g_hidden_windows, window);
      }
      app_windows_update_space(&g_windows, window->sid, false);
    }
  } else if (event == 815) {
    struct app_window* window = app_windows_find_by_wid(&g_hidden_windows, wid);
    if (window) {
      app_windows_update_space(&g_windows, window->sid, false);
      app_window_clear(window);
      app_windows_register_notifications();
    }
  }
}

static void update_all_spaces(struct app_windows* windows, bool silent) {
  uint32_t display_count = 0;
  uint32_t* displays = display_active_display_list(&display_count);
  for (int i = 0; i < display_count; i++) {
    int space_count = 0;
    uint64_t* spaces = display_space_list(displays[i], &space_count);
    for (int j = 0; j < space_count; j++) {
      app_windows_update_space(windows, spaces[j], silent);
    }
    if (spaces) free(spaces);
  }
  if (displays) free(displays);
}

static void space_handler(uint32_t event, void* data, size_t data_length, void* context) {
  update_all_spaces(&g_windows, event == 1401);
}

void forced_space_windows_event() {
  if (g_space_window_events) update_all_spaces(&g_windows, false);
}

void begin_receiving_space_window_events() {
  if (!g_space_window_events) {
    SLSRegisterNotifyProc(window_spawn_handler,
                          1325,
                          (void*)(intptr_t)g_connection);

    SLSRegisterNotifyProc(window_spawn_handler,
                          1326,
                          (void*)(intptr_t)g_connection);

    SLSRegisterNotifyProc(window_hide_handler,
                          815,
                          (void*)(intptr_t)g_connection);

    SLSRegisterNotifyProc(window_hide_handler,
                          816,
                          (void*)(intptr_t)g_connection);

    SLSRegisterNotifyProc((void*)space_handler, 1401, NULL);
    if (__builtin_available(macOS 13.0, *)) {
      SLSRegisterNotifyProc((void*)space_handler, 1327, NULL);
      SLSRegisterNotifyProc((void*)space_handler, 1328, NULL);
    }

    g_space_window_events = true;
    update_all_spaces(&g_windows, true);
  }
}
