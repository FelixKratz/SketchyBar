#include "bar_manager.h"
#include "event.h"
#include <ApplicationServices/ApplicationServices.h>
#include <libgen.h>

extern char g_config_file[4096];
extern char g_name[256];
bool g_hotload = false;

void hotload_set_state(int state) {
  g_hotload = state;
}

int hotload_get_state() {
  return g_hotload;
}

bool set_config_file_path(char* file) {
  char* path = realpath(file, NULL);
  if (path) {
    snprintf(g_config_file, sizeof(g_config_file), "%s", path);
    free(path);
    return true;
  }
  return false;
}

static bool get_config_file(char *restrict filename, char *restrict buffer, int buffer_size) {
  char *xdg_home = getenv("XDG_CONFIG_HOME");
  if (xdg_home && *xdg_home) {
    snprintf(buffer, buffer_size, "%s/%s/%s", xdg_home, g_name, filename);
    if (file_exists(buffer)) return true;
  }

  char *home = getenv("HOME");
  if (!home) return false;

  snprintf(buffer, buffer_size, "%s/.config/%s/%s", home, g_name, filename);
  if (file_exists(buffer)) return true;

  snprintf(buffer, buffer_size, "%s/.%s", home, filename);
  return file_exists(buffer);
}

void exec_config_file() {
  if (!*g_config_file
    && !get_config_file("sketchybarrc", g_config_file, sizeof(g_config_file))) {
    printf("could not locate config file..\n");
    return;
  }

  if (!file_exists(g_config_file)) {
    printf("file '%s' does not exist..\n", g_config_file);
    return;
  }

  setenv("CONFIG_DIR", dirname(g_config_file), 1);
  chdir(dirname(g_config_file));

  if (!ensure_executable_permission(g_config_file)) {
    printf("could not set the executable permission bit for '%s'\n", g_config_file);
    return;
  }

  if (!fork_exec(g_config_file, NULL)) {
    printf("failed to execute file '%s'\n", g_config_file);
    return;
  }
}

static void handler(ConstFSEventStreamRef stream,
             void* context,
             size_t count,
             void* paths,
             const FSEventStreamEventFlags* flags,
             const FSEventStreamEventId* ids) {
  if (g_hotload && count > 0) {
    struct event event = { NULL, HOTLOAD };
    event_post(&event); 
  }
}

int begin_receiving_config_change_events() {
  char* file = dirname(g_config_file);
  CFStringRef file_ref = CFStringCreateWithCString(
                             kCFAllocatorDefault, file, kCFStringEncodingUTF8);

  CFArrayRef paths = CFArrayCreate(NULL,
                                   (const void**)&file_ref,
                                   1,
                                   &kCFTypeArrayCallBacks);

  FSEventStreamRef stream = FSEventStreamCreate(
                                         kCFAllocatorDefault,
                                         handler,
                                         NULL,
                                         paths,
                                         kFSEventStreamEventIdSinceNow,
                                         0.5,
                                         kFSEventStreamCreateFlagNoDefer
                                         | kFSEventStreamCreateFlagFileEvents);

  CFRelease(file_ref);
  CFRelease(paths);

  FSEventStreamScheduleWithRunLoop(stream, CFRunLoopGetCurrent(),
                                           kCFRunLoopDefaultMode);

  FSEventStreamStart(stream);
  return 0;
}
