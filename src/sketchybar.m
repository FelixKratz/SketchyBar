#include "bar_manager.h"
#include "workspace.h"
#include "event_loop.h"
#include "mach.h"
#include "mouse.h"
#include "message.h"
#include "power.h"
#include "wifi.h"

#define LCFILE_PATH_FMT         "/tmp/sketchybar_%s.lock"

#define CLIENT_OPT_LONG         "--message"
#define CLIENT_OPT_SHRT         "-m"

#define VERSION_OPT_LONG        "--version"
#define VERSION_OPT_SHRT        "-v"

#define CONFIG_OPT_LONG         "--config"
#define CONFIG_OPT_SHRT         "-c"

#define HELP_OPT_LONG           "--help"
#define HELP_OPT_SHRT           "-h"

#define MAJOR 2
#define MINOR 14
#define PATCH 2

extern int SLSMainConnectionID(void);
extern int RunApplicationEventLoop(void);

int g_connection;
CFTypeRef g_transaction;

struct bar_manager g_bar_manager;
struct event_loop g_event_loop;
struct mach_server g_mach_server;
void *g_workspace_context;

char g_config_file[4096];
char g_lock_file[MAXLEN];
bool g_volume_events;
bool g_brightness_events;

static void help(char *cmd) {
  printf(
    "Usage: %s [options]\n\n"
    "Startup: \n"
    "  -c, --config CONFIGFILE\tRead CONFIGFILE as the configuration file\n"
    "                         \tDefault CONFIGFILE is ~/.config/sketchybar/sketchybarrc\n\n"
    "Set global bar properties, see https://felixkratz.github.io/SketchyBar/config/bar\n"
    "      --bar <setting>=<value> ... <setting>=<value>\n\n"
    "Items and their properties, see https://felixkratz.github.io/SketchyBar/config/items\n"
    "      --add item <name> <position>\tAdd item to bar\n"
    "      --set <name> <property>=<value> ... <property>=<value>\n"
    "                                  \tChange item properties\n"
    "      --default <property>=<value> ... <property>=<value>\n"
    "                                  \tChange default properties for new items\n"
    "      --set <name> popup.<popup_property>=<value>\n"
    "                                  \tConfigure item popup menu\n"
    "                                  \tSee https://felixkratz.github.io/SketchyBar/config/popups\n"
    "      --reorder <name> ... <name> \tReorder items\n"
    "      --move <name> before <reference name>\n"
    "      --move <name> after <reference name>\n"
    "                                  \tMove item relative to reference item\n"
    "      --clone <parent name> <name> [optional: before/after]\n"
    "                                  \tClone parent to create new item\n"
    "      --rename <old name> <new name>\tRename item\n"
    "      --remove <name>             \tRemove item\n\n"
    "Special components, see https://felixkratz.github.io/SketchyBar/config/components\n"
    "      --add graph <name> <position> <width in points>\n"
    "                                  \tAdd graph component\n"
    "      --push <name> <data point> ... <data point>\n"
    "                                  \tPush data points to a graph\n"
    "      --add space <name> <position>\tAdd space component\n"
    "      --add bracket <name> <member name> ... <member name>\n"
    "                                  \tAdd bracket component\n"
    "      --add alias <application_name> <position>\n"
    "                                  \tAdd alias component\n"
    "      --add slider <name> <position> <width>\n"
    "                                  \tAdd slider component\n\n"
    "Events and Scripting, see https://felixkratz.github.io/SketchyBar/config/events\n"
    "      --subscribe <name> <event> ... <event>\n"
    "                                  \tSubscribe to events\n"
    "      --add event <name> [optional: <NSDistributedNotificationName>]\n"
    "                                  \tCreate custom event\n"
    "      --trigger <event> [Optional: <envvar>=<value> ... <envvar>=<value>]\n"
    "                                  \tTrigger custom event\n\n"
    "Querying information, see https://felixkratz.github.io/SketchyBar/config/querying\n"
    "      --query bar               \tQuery bar properties\n"
    "      --query <name>            \tQuery item properties\n"
    "      --query defaults          \tQuery default properties\n"
    "      --query events            \tQuery events\n"
    "      --query default_menu_items\tQuery names of available items for aliases\n\n"
    "Animations, see https://felixkratz.github.io/SketchyBar/config/animations\n"
    "      --animate <linear|quadratic|tanh|sin|exp|circ> <duration> \\\n"
    "                --bar <property=value> ... <property=value>\\\n"
    "                --set <name> <property=value> ... <property=value>\n"
    "                         \tAnimate from given source to target property values\n\n",
    cmd);
}

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

  char* rsp = mach_send_message(mach_get_bs_port(MACH_BS_NAME),
                                message,
                                message_length,
                                true                           );

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
    error("sketchybar: could not create lock-file! abort..\n");
  }

  struct flock lockfd = {
    .l_start  = 0,
    .l_len    = 0,
    .l_pid    = getpid(),
    .l_type   = F_WRLCK,
    .l_whence = SEEK_SET
  };

  if (fcntl(handle, F_SETLK, &lockfd) == -1) {
    error("sketchybar: could not acquire lock-file... already running?\n");
  }
}

static bool get_config_file(char *restrict filename, char *restrict buffer, int buffer_size) {
  char *xdg_home = getenv("XDG_CONFIG_HOME");
  if (xdg_home && *xdg_home) {
    snprintf(buffer, buffer_size, "%s/sketchybar/%s", xdg_home, filename);
    if (file_exists(buffer)) return true;
  }

  char *home = getenv("HOME");
  if (!home) return false;

  snprintf(buffer, buffer_size, "%s/.config/sketchybar/%s", home, filename);
  if (file_exists(buffer)) return true;

  snprintf(buffer, buffer_size, "%s/.%s", home, filename);
  return file_exists(buffer);
}

static void exec_config_file(void) {
  if (!*g_config_file
    && !get_config_file("sketchybarrc", g_config_file, sizeof(g_config_file))) {
    printf("could not locate config file..");
    return;
  }

  if (!file_exists(g_config_file)) {
    printf("file '%s' does not exist..", g_config_file);
    return;
  }

  if (!ensure_executable_permission(g_config_file)) {
    printf("could not set the executable permission bit for '%s'", g_config_file);
    return;
  }

  if (!fork_exec(g_config_file, NULL)) {
    printf("failed to execute file '%s'", g_config_file);
    return;
  }
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
static inline void init_misc_settings(void) {
  char *user = getenv("USER");
  if (!user) {
    error("sketchybar: 'env USER' not set! abort..\n");
  }

  snprintf(g_lock_file, sizeof(g_lock_file), LCFILE_PATH_FMT, user);

  if (__builtin_available(macOS 13.0, *)) {
  } else {
    NSApplicationLoad();
  }

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
    help(argv[0]);
    exit(EXIT_SUCCESS);
  } else if ((string_equals(argv[1], CLIENT_OPT_LONG))
             || (string_equals(argv[1], CLIENT_OPT_SHRT))) {
    exit(client_send_message(argc-1, argv+1));
  } else if ((string_equals(argv[1], CONFIG_OPT_LONG))
             || (string_equals(argv[1], CONFIG_OPT_SHRT))) {
    if (argc < 3) {
      printf("[!] Error: Too few arguments for argument 'config'.\n");
    } else {
      snprintf(g_config_file, sizeof(g_config_file), "%s", argv[2]);
      return;
    }
    exit(EXIT_FAILURE);
  }

  exit(client_send_message(argc, argv));
}

int main(int argc, char **argv) {
  if (argc > 1) parse_arguments(argc, argv);

  if (is_root()) error("sketchybar: running as root is not allowed! abort..\n");

  init_misc_settings();
  acquire_lockfile();

  if (!event_loop_init(&g_event_loop))
    error("sketchybar: could not initialize event_loop! abort..\n");

  workspace_event_handler_init(&g_workspace_context);
  bar_manager_init(&g_bar_manager);

  event_loop_begin(&g_event_loop);
  mouse_begin();
  display_begin();
  workspace_event_handler_begin(&g_workspace_context);

  windows_freeze();
  bar_manager_begin(&g_bar_manager);
  windows_unfreeze();

  if (!mach_server_begin(&g_mach_server, mach_message_handler))
    error("sketchybar: could not initialize daemon! abort..\n");

  begin_receiving_power_events();
  begin_receiving_network_events();

  exec_config_file();
  RunApplicationEventLoop();
  return 0;
}
