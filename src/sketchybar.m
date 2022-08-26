#include "bar_manager.h"
#include "workspace.h"
#include "event_loop.h"
#include "mach.h"
#include "mouse.h"
#include "message.h"

#define LCFILE_PATH_FMT         "/tmp/sketchybar_%s.lock"

#define CLIENT_OPT_LONG         "--message"
#define CLIENT_OPT_SHRT         "-m"

#define DEBUG_VERBOSE_OPT_LONG  "--verbose"
#define DEBUG_VERBOSE_OPT_SHRT  "-V"
#define VERSION_OPT_LONG        "--version"
#define VERSION_OPT_SHRT        "-v"

#define MAJOR 2
#define MINOR 8
#define PATCH 6

extern int SLSMainConnectionID(void);
extern int RunApplicationEventLoop(void);

int g_connection;

struct bar_manager g_bar_manager;
struct event_loop g_event_loop;
struct mach_server g_mach_server;
void *g_workspace_context;

char g_config_file[4096];
char g_lock_file[MAXLEN];
bool g_verbose;

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

    if (!mach_send_message(mach_get_bs_port(), message, message_length, true))
      return EXIT_FAILURE;

    return 0;
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
        error("sketchybar: could not acquire lock-file! abort..\n");
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
    if (!get_config_file("sketchybarrc", g_config_file, sizeof(g_config_file))) {
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

    NSApplicationLoad();
    signal(SIGCHLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    CGSetLocalEventsSuppressionInterval(0.0f);
    CGEnableEventStateCombining(false);
    g_connection = SLSMainConnectionID();
}
#pragma clang diagnostic pop

static void parse_arguments(int argc, char **argv) {
  if ((string_equals(argv[1], VERSION_OPT_LONG)) || (string_equals(argv[1], VERSION_OPT_SHRT))) {
    fprintf(stdout, "sketchybar-v%d.%d.%d\n", MAJOR, MINOR, PATCH);
    exit(EXIT_SUCCESS);
  }
  
  if ((string_equals(argv[1], DEBUG_VERBOSE_OPT_LONG) || string_equals(argv[1], DEBUG_VERBOSE_OPT_SHRT)))
    g_verbose = true;

  if ((string_equals(argv[1], CLIENT_OPT_LONG)) || (string_equals(argv[1], CLIENT_OPT_SHRT)))
    exit(client_send_message(argc-1, argv+1));

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

  exec_config_file();
  RunApplicationEventLoop();
  return 0;
}
