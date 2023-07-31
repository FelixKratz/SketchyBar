#include "bar_manager.h"
#include "workspace.h"
#include "mach.h"
#include "mouse.h"
#include "message.h"
#include "power.h"
#include "wifi.h"
#include "misc/help.h"
#include "media.h"
#include "hotload.h"
#include <libgen.h>

#define LCFILE_PATH_FMT  "/tmp/%s_%s.lock"

#define CLIENT_OPT_LONG  "--message"
#define CLIENT_OPT_SHRT  "-m"

#define VERSION_OPT_LONG "--version"
#define VERSION_OPT_SHRT "-v"

#define CONFIG_OPT_LONG  "--config"
#define CONFIG_OPT_SHRT  "-c"

#define HELP_OPT_LONG    "--help"
#define HELP_OPT_SHRT    "-h"

#define MAJOR 2
#define MINOR 15
#define PATCH 2

extern int SLSMainConnectionID(void);
extern CGError SLSRegisterNotifyProc(void* callback, uint32_t event, void* context);

extern CGError SLSGetEventPort(int cid, mach_port_t* port_out);
extern CGEventRef SLEventCreateNextEvent(int cid);
extern void _CFMachPortSetOptions(CFMachPortRef mach_port, int options);
extern CGError SLSSetBackgroundEventMask(int cid, int mask);
extern CGError SLSSetEventMask(int cid, int mask);

int g_connection;
CFTypeRef g_transaction;

struct bar_manager g_bar_manager;
struct mach_server g_mach_server;
void *g_workspace_context;

char g_name[256];
char g_config_file[4096];
char g_lock_file[MAXLEN];
bool g_volume_events;
bool g_brightness_events;
int64_t g_disable_capture = 0;

static int client_send_message(int argc, char **argv) {
  if (argc <= 1) {
    return EXIT_SUCCESS;
  }

  char *user = getenv("USER");
  if (!user) {
    error("sketchybar-msg: 'env USER' not set! abort..\n");
  }

  int message_length = argc;
  int argl[argc];

  for (int i = 1; i < argc; ++i) {
    argl[i] = strlen(argv[i]);
    message_length += argl[i] + 1;
  }

  char* message = malloc((sizeof(char) * (message_length + 1)));
  char* temp = message;

  for (int i = 1; i < argc; ++i) {
    memcpy(temp, argv[i], argl[i]);
    temp += argl[i];
    *temp++ = '\0';
  }
  *temp++ = '\0';

  char bs_name[256];
  snprintf(bs_name, 256, MACH_BS_NAME_FMT, g_name);

  char* rsp = mach_send_message(mach_get_bs_port(bs_name),
                                message,
                                message_length,
                                true                     );

  free(message);
  if (!rsp) return EXIT_SUCCESS;

  if (strlen(rsp) > 2 && rsp[1] == '!') {
    fprintf(stderr, "%s", rsp);
    return EXIT_FAILURE;
  } else {
    fprintf(stdout, "%s", rsp);
  }

  return EXIT_SUCCESS;
}

static void acquire_lockfile(void) {
  int handle = open(g_lock_file, O_CREAT | O_WRONLY, 0600);
  if (handle == -1) {
    error("%s: could not create lock-file! abort..\n", g_name);
  }

  struct flock lockfd = {
    .l_start  = 0,
    .l_len    = 0,
    .l_pid    = getpid(),
    .l_type   = F_WRLCK,
    .l_whence = SEEK_SET
  };

  if (fcntl(handle, F_SETLK, &lockfd) == -1) {
    error("%s: could not acquire lock-file... already running?\n", g_name);
  }
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
static inline void init_misc_settings(void) {
  char *user = getenv("USER");
  if (!user) {
    error("%s: 'env USER' not set! abort..\n", g_name);
  }

  snprintf(g_lock_file, sizeof(g_lock_file), LCFILE_PATH_FMT, g_name, user);

  signal(SIGCHLD, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  CGSetLocalEventsSuppressionInterval(0.0f);
  CGEnableEventStateCombining(false);
  g_connection = SLSMainConnectionID();
  g_volume_events = false;
  g_brightness_events = false;
}
#pragma clang diagnostic pop

static void parse_arguments(int argc, char **argv) {
  if ((string_equals(argv[1], VERSION_OPT_LONG))
      || (string_equals(argv[1], VERSION_OPT_SHRT))) {
    fprintf(stdout, "sketchybar-v%d.%d.%d\n", MAJOR, MINOR, PATCH);
    exit(EXIT_SUCCESS);
  } else if ((string_equals(argv[1], HELP_OPT_LONG))
      || (string_equals(argv[1], HELP_OPT_SHRT))) {
    printf(help_str, argv[0]);
    exit(EXIT_SUCCESS);
  } else if ((string_equals(argv[1], CLIENT_OPT_LONG))
             || (string_equals(argv[1], CLIENT_OPT_SHRT))) {
    exit(client_send_message(argc-1, argv+1));
  } else if ((string_equals(argv[1], CONFIG_OPT_LONG))
             || (string_equals(argv[1], CONFIG_OPT_SHRT))) {
    if (argc < 3) {
      printf("[!] Error: Too few arguments for argument 'config'.\n");
    } else {
      if (set_config_file_path(argv[2])) return;
      printf("[!] Error: Specified config file path invalid.\n");
    }
    exit(EXIT_FAILURE);
  }

  exit(client_send_message(argc, argv));
}

void system_events(uint32_t event, void* data, size_t data_length, void* context) {
  if (event == 1322) {
    g_disable_capture = clock_gettime_nsec_np(CLOCK_MONOTONIC_RAW_APPROX);
  } else if (event == 905) {
    g_disable_capture = -1;
  } else {
    g_disable_capture = 0;
  }
}

void mach_port_callback(CFMachPortRef port, void* message, CFIndex size, void* context) {
  int cid = g_connection;
  CGEventRef cg_event = SLEventCreateNextEvent(cid);
  if (!cg_event) return;
  do {
    CGEventType type = CGEventGetType(cg_event);
    if (type != 0xd) mouse_handle_event(type, cg_event);

    CFRelease(cg_event);
    cg_event = SLEventCreateNextEvent(cid);
  } while (cg_event != NULL);
}

int main(int argc, char **argv) {
  snprintf(g_name, sizeof(g_name), "%s", basename(argv[0]));
  if (argc > 1) parse_arguments(argc, argv);

  if (is_root())
    error("%s: running as root is not allowed! abort..\n", g_name);

  init_misc_settings();
  acquire_lockfile();

  SLSRegisterNotifyProc((void*)system_events, 904, NULL);
  SLSRegisterNotifyProc((void*)system_events, 905, NULL);
  SLSRegisterNotifyProc((void*)system_events, 1401, NULL);
  SLSRegisterNotifyProc((void*)system_events, 1508, NULL);
  SLSRegisterNotifyProc((void*)system_events, 1322, NULL);

  workspace_event_handler_init(&g_workspace_context);
  bar_manager_init(&g_bar_manager);

  display_begin();
  workspace_event_handler_begin(&g_workspace_context);

  windows_freeze();
  bar_manager_begin(&g_bar_manager);
  windows_unfreeze();

  if (!mach_server_begin(&g_mach_server, mach_message_handler))
    error("%s: could not initialize daemon! abort..\n", g_name);

  begin_receiving_power_events();
  begin_receiving_network_events();
  initialize_media_events();

  exec_config_file();
  begin_receiving_config_change_events();
  SLSSetBackgroundEventMask(g_connection, g_mouse_events);
  SLSSetEventMask(g_connection, g_mouse_events);
  
  mach_port_t port;
  CGError error = SLSGetEventPort(g_connection, &port);
  CFMachPortRef cf_mach_port = NULL;
  CFRunLoopSourceRef source = NULL;
  if (error == kCGErrorSuccess) {
    cf_mach_port = CFMachPortCreateWithPort(NULL,
                                            port,
                                            mach_port_callback,
                                            NULL,
                                            false              );

    _CFMachPortSetOptions(cf_mach_port, 0x40);
    source = CFMachPortCreateRunLoopSource(NULL, cf_mach_port, 0);

    CFRunLoopAddSource(CFRunLoopGetCurrent(),
                       source,
                       kCFRunLoopDefaultMode);
  }

  CFRunLoopRun();
  return 0;
}
