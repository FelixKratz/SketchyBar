#pragma once
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ApplicationServices/ApplicationServices.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include "env_vars.h"
#include "defines.h"

#define array_count(a) (sizeof((a)) / sizeof(*(a)))
#define max(a, b) (a > b ? a : b)
#define min(a, b) (a < b ? a : b)
#define clamp(x, l, u) (min(max(x, l), u))

#define MAXLEN 512
#define FORK_TIMEOUT 60

extern CFArrayRef SLSCopyManagedDisplaySpaces(int cid);
extern uint32_t SLSGetActiveSpace(int cid);
extern CFStringRef SLSCopyManagedDisplayForSpace(int cid, uint64_t sid);
extern CFArrayRef SLSHWCaptureSpace(int64_t cid, int64_t sid, int64_t flags);
extern int g_connection;

struct signal_args {
  struct env_vars env_vars;
  void *entity;
  void *param1;
};

static CGPoint g_nirvana = {-9999, -9999};

static double deg_to_rad = 2.* M_PI / 360.;

struct token {
  char *text;
  unsigned int length;
};

struct notification {
  char* name;
  char* info;
};

static inline struct notification* notification_create() {
  struct notification* notification = malloc(sizeof(struct notification));
  memset(notification, 0, sizeof(struct notification));
  return notification;
}

static inline void notification_destroy(struct notification* notification) {
  if (notification->name) free(notification->name);
  if (notification->info) free(notification->info);
  free(notification);
}

static inline double function_linear(double x) {
  return x;
}

static inline double function_square(double x) {
  return x*x;
}

static inline double function_tanh(double x) {
  double a = 0.52;
  return a * tanh(2. * atanh(1. / (2. * a)) * (x  - 0.5)) + 0.5;
}

static inline double function_sin(double x) {
  return sin(M_PI / 2. * x);
}

static inline double function_exp(double x) {
  return x*exp(x - 1.);
}

static inline double function_circ(double x) {
    return sqrt(1.f - powf(x - 1.f, 2.f));
}

static inline char* format_bool(bool b) {
  return b ? "on" : "off";
}

static inline char* escape_string(char* string) {
  if (!string) return NULL;
  int len = strlen(string);
  char* buffer = malloc(2*len + 1);
  int cursor = 0;
  for (int i = 0; i < len; i++) {
    if (string[i] == '"') {
      buffer[cursor++] = '\\';
      buffer[cursor++] = '"';
    }
    else if (string[i] == '\n') {
      buffer[cursor++] = '\\';
      buffer[cursor++] = 'n';
    }
    else {
      buffer[cursor++] = string[i];
    }
  }
  buffer[cursor] = '\0';
  return buffer;
}

static inline void respond(FILE* rsp, char* response, ...) {
  time_t t = time(NULL);
  struct tm ltime = *localtime(&t);
  printf("[%d-%02d-%02d %02d:%02d:%02d] ",
         ltime.tm_year + 1900,
         ltime.tm_mon + 1,
         ltime.tm_mday,
         ltime.tm_hour,
         ltime.tm_min,
         ltime.tm_sec                     );

  va_list args_rsp;
  va_list args_stdout;
  va_start(args_rsp, response);
  va_copy(args_stdout, args_rsp);
  vfprintf(rsp, response, args_rsp);
  vfprintf(stdout, response, args_stdout);
  va_end(args_rsp);
  va_end(args_stdout);
}

static inline struct key_value_pair get_key_value_pair(char *token, char split) {
  struct key_value_pair key_value_pair;
  key_value_pair.key = token;

  while (*token) {
    if (token[0] == split) break;
    ++token;
  }

  if (*token != split) {
    key_value_pair.key = NULL;
    key_value_pair.value = NULL;
  } else if (token[1]) {
    *token = '\0';
    key_value_pair.value = token + 1;
  } else {
    *token = '\0';
    key_value_pair.value = NULL;
  }

  return key_value_pair;
}

static inline void pack_key_value_pair(char* cursor, struct key_value_pair* key_value_pair) {
  uint32_t key_len = strlen(key_value_pair->key);
  uint32_t val_len = key_value_pair->value ? strlen(key_value_pair->value) : 0;
  memcpy(cursor, key_value_pair->key, key_len);
  cursor += key_len;
  *cursor++ = '\0';
  memcpy(cursor, key_value_pair->value, val_len);
  cursor += val_len;
  *cursor++ = '\0';
  *cursor++ = '\0';
}

static inline bool is_root(void) {
  return getuid() == 0 || geteuid() == 0;
}

static inline bool string_equals(const char *a, const char *b) {
  return a && b && strcmp(a, b) == 0;
}

static inline char* get_type_description(uint32_t type) {
  switch (type) {
    case kCGEventLeftMouseUp:
      return "left";
    case kCGEventRightMouseUp:
      return "right";
    default:
      return "other";
  }
}

static inline char* get_modifier_description(uint32_t modifier) {
  if (modifier & kCGEventFlagMaskShift)
    return "shift";
  else if (modifier & kCGEventFlagMaskControl)
    return "ctrl";
  else if (modifier & kCGEventFlagMaskAlternate)
    return "alt";
  else if (modifier & kCGEventFlagMaskCommand)
    return "cmd";
  else 
    return "none";
}

static inline char** token_split(struct token token, char split, uint32_t* count) {
  if (!token.text || token.length == 0) return NULL;
  char** list = NULL;
  *count = 0;

  int prev = -1;
  for (int i = 0; i < token.length + 1; i++) {
    if (token.text[i] == split || token.text[i] == '\0') {
      list = realloc(list, sizeof(char*) * ++*count);
      token.text[i] = '\0';
      list[*count - 1] = &token.text[prev + 1];
      prev = i;
    }
  }
  return list;
}

static inline bool token_equals(struct token token, char *match) {
  char *at = match;
  for (int i = 0; i < token.length; ++i, ++at) {
    if ((*at == 0) || (token.text[i] != *at)) {
      return false;
    }
  }
  return *at == 0;
}

static inline char *token_to_string(struct token token) {
  char *result = malloc(token.length + 1);
  if (!result) return NULL;

  memcpy(result, token.text, token.length);
  result[token.length] = '\0';
  return result;
}

static inline uint32_t token_to_uint32t(struct token token) {
  char buffer[token.length + 1];
  memcpy(buffer, token.text, token.length);
  buffer[token.length] = '\0';
  return strtoul(buffer, NULL, 0);
}

static inline int token_to_int(struct token token) {
  char buffer[token.length + 1];
  memcpy(buffer, token.text, token.length);
  buffer[token.length] = '\0';
  return (int) strtol(buffer, NULL, 0);
}

static inline float token_to_float(struct token token) {
  char buffer[token.length + 1];
  memcpy(buffer, token.text, token.length);
  buffer[token.length] = '\0';
  return strtof(buffer, NULL);
}

static inline struct token get_token(char **message) {
  struct token token;

  token.text = *message;
  while (**message) {
    ++(*message);
  }
  token.length = *message - token.text;

  if ((*message)[0] == '\0' && (*message)[1] != '\0') {
    ++(*message);
  } else {
    // NOTE(koekeishiya): don't go past the null-terminator
  }

  return token;
}

static inline bool evaluate_boolean_state(struct token state, bool previous_state) {
  if (token_equals(state, ARGUMENT_COMMON_VAL_ON)
      || token_equals(state, ARGUMENT_COMMON_VAL_YES)
      || token_equals(state, ARGUMENT_COMMON_VAL_TRUE)
      || token_equals(state, ARGUMENT_COMMON_VAL_ONE)
      || token_equals(state, ARGUMENT_COMMON_VAL_NOT_OFF)
      || token_equals(state, ARGUMENT_COMMON_VAL_NOT_NO)
      || token_equals(state, ARGUMENT_COMMON_VAL_NOT_FALSE)
      || token_equals(state, ARGUMENT_COMMON_VAL_NOT_ZERO) )
    return true;
  else if (token_equals(state, ARGUMENT_COMMON_VAL_TOGGLE))
    return !previous_state;
  else return false;
}

static inline uint32_t get_set_bit_position(uint32_t mask) {
  if (mask == 0) return UINT32_MAX;
  uint32_t pos = 0;
  while (!(mask & 1)) {
    mask >>= 1;
    pos++;
  }
  return pos;
}

static inline void clip_rect(CGContextRef context, CGRect region, float clip, uint32_t corner_radius) {
  CGMutablePathRef path = CGPathCreateMutable();
  if (corner_radius > region.size.height / 2.f || corner_radius > region.size.width / 2.f)
    corner_radius = region.size.height > region.size.width ? region.size.width / 2.f : region.size.height / 2.f;
  CGPathAddRoundedRect(path, NULL, region, corner_radius, corner_radius);
  CGContextSetBlendMode(context, kCGBlendModeDestinationOut);
  CGContextSetRGBFillColor(context, 0.f, 0.f, 0.f, clip);
  CGContextAddPath(context, path);
  CGContextDrawPath(context, kCGPathFillStroke);
  CGContextSetBlendMode(context, kCGBlendModeNormal);
  CFRelease(path);
}

static inline CGRect cgrect_mirror_y(CGRect rect, float y) {
  CGRect mirrored_rect = rect;
  mirrored_rect.origin.y = 2*y - rect.origin.y;
  return mirrored_rect;
}

static inline bool cgrect_contains_point(CGRect* r, CGPoint* p) {
  return p->x >= r->origin.x && p->x <= r->origin.x + r->size.width &&
         p->y >= r->origin.y && p->y <= r->origin.y + r->size.height;
}

static inline char *string_escape_quote(char *s) {
  if (!s) return NULL;

  char *cursor = s;
  int num_quotes = 0;

  while (*cursor) {
    if (*cursor == '"') ++num_quotes;
    ++cursor;
  }

  if (!num_quotes) return NULL;

  int size_in_bytes = (int)(cursor - s) + num_quotes;
  char *result = malloc(sizeof(char) * (size_in_bytes+1));
  result[size_in_bytes] = '\0';

  for (char *dst = result, *cursor = s; *cursor; ++cursor) {
    if (*cursor == '"') *dst++ = '\\';
    *dst++ = *cursor;
  }

  return result;
}

static inline CFArrayRef cfarray_of_cfnumbers(void *values, size_t size, int count, CFNumberType type) {
  CFNumberRef temp[count];

  for (int i = 0; i < count; ++i) {
    temp[i] = CFNumberCreate(NULL, type, ((char *)values) + (size * i));
  }

  CFArrayRef result = CFArrayCreate(NULL, (const void **)temp, count, &kCFTypeArrayCallBacks);

  for (int i = 0; i < count; ++i) {
    CFRelease(temp[i]);
  }

  return result;
}

static inline char *cfstring_copy(CFStringRef string) {
  CFIndex num_bytes = CFStringGetMaximumSizeForEncoding(CFStringGetLength(string), kCFStringEncodingUTF8);
  char *result = malloc(num_bytes + 1);
  if (!result) return NULL;

  if (!CFStringGetCString(string, result, num_bytes + 1, kCFStringEncodingUTF8)) {
    free(result);
    result = NULL;
  }

  return result;
}

static inline char *string_copy(char *s) {
  int length = strlen(s);
  char *result = malloc(length + 1);
  if (!result) return NULL;

  memcpy(result, s, length);
  result[length] = '\0';
  return result;
}

static inline char* read_file(char* path) {
  int fd = open(path, O_RDONLY);
  int len = lseek(fd, 0, SEEK_END);
  char* file = mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0);
  close(fd);
  free(path);
  return string_copy(file);
}

static inline char* resolve_path(char* path) {
  if (!path) return NULL;

  if (path[0] == '~') {
    char* home = getenv("HOME");
    char buf[512];
    snprintf(buf, sizeof(buf), "%s%s", home, &path[1]);
    free(path);
    return string_copy(buf);
  }
  return path;
}

static inline bool file_exists(char *filename) {
  struct stat buffer;

  if (stat(filename, &buffer) != 0) {
    return false;
  }

  if (buffer.st_mode & S_IFDIR) {
    return false;
  }

  return true;
}

static inline bool ensure_executable_permission(char *filename) {
  struct stat buffer;

  if (stat(filename, &buffer) != 0) {
    return false;
  }

  bool is_executable = buffer.st_mode & S_IXUSR;
  if (!is_executable && chmod(filename, S_IXUSR | buffer.st_mode) != 0) {
    return false;
  }
  return true;
}

static inline bool sync_exec(char *command, struct env_vars *env_vars) {
  if (env_vars) {
    for (int i = 0; i < env_vars->count; i++) {
      setenv(env_vars->vars[i]->key, env_vars->vars[i]->value, 1);
    }
  }

  char *exec[] = { "/usr/bin/env", "sh", "-c", command, NULL};
  return execvp(exec[0], exec);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
static inline bool fork_exec(char *command, struct env_vars* env_vars) {
  int pid = vfork();
  if (pid == -1) return false;
  if (pid !=  0) return true;

  alarm(FORK_TIMEOUT);
  exit(sync_exec(command, env_vars));
}
#pragma clang diagnostic pop

static inline int mission_control_index(uint64_t sid) {
  uint64_t result = 0;
  int desktop_cnt = 1;

  CFArrayRef display_spaces_ref = SLSCopyManagedDisplaySpaces(g_connection);
  int display_spaces_count = CFArrayGetCount(display_spaces_ref);

  for (int i = 0; i < display_spaces_count; ++i) {
    CFDictionaryRef display_ref = CFArrayGetValueAtIndex(display_spaces_ref, i);
    CFArrayRef spaces_ref = CFDictionaryGetValue(display_ref, CFSTR("Spaces"));
    int spaces_count = CFArrayGetCount(spaces_ref);

    for (int j = 0; j < spaces_count; ++j) {
      CFDictionaryRef space_ref = CFArrayGetValueAtIndex(spaces_ref, j);
      CFNumberRef sid_ref = CFDictionaryGetValue(space_ref, CFSTR("id64"));
      CFNumberGetValue(sid_ref, CFNumberGetType(sid_ref), &result);
      if (sid == result) goto out;

      ++desktop_cnt;
    }
  }

  desktop_cnt = 0;
out:
  CFRelease(display_spaces_ref);
  return desktop_cnt;
}

static inline uint64_t dsid_from_sid(uint32_t sid) {
  uint64_t result = 0;
  int desktop_cnt = 1;

  CFArrayRef display_spaces_ref = SLSCopyManagedDisplaySpaces(g_connection);
  int display_spaces_count = CFArrayGetCount(display_spaces_ref);

  for (int i = 0; i < display_spaces_count; ++i) {
    CFDictionaryRef display_ref = CFArrayGetValueAtIndex(display_spaces_ref, i);
    CFArrayRef spaces_ref = CFDictionaryGetValue(display_ref, CFSTR("Spaces"));
    int spaces_count = CFArrayGetCount(spaces_ref);

    for (int j = 0; j < spaces_count; ++j) {
      CFDictionaryRef space_ref = CFArrayGetValueAtIndex(spaces_ref, j);
      CFNumberRef sid_ref = CFDictionaryGetValue(space_ref, CFSTR("id64"));
      CFNumberGetValue(sid_ref, CFNumberGetType(sid_ref), &result);
      if (sid == desktop_cnt) goto out;

      ++desktop_cnt;
    }
  }

  result = 0;
out:
  CFRelease(display_spaces_ref);
  return result;
}

static inline CGImageRef space_capture(uint32_t sid) {
  uint64_t dsid = dsid_from_sid(sid);
  CGImageRef image = NULL;
  if (dsid) {
    CFArrayRef result = SLSHWCaptureSpace(g_connection, dsid, 0);
    uint32_t count = CFArrayGetCount(result);
    if (count > 0) {
      image = (CGImageRef)CFRetain(CFArrayGetValueAtIndex(result, 0));
    }
    CFRelease(result);
  }
  return image;
}

static inline uint32_t display_id_for_space(uint32_t sid) {
  uint64_t dsid = dsid_from_sid(sid);
  if (!dsid) return 0;
  CFStringRef uuid_string = SLSCopyManagedDisplayForSpace(g_connection, dsid);
  if (!uuid_string) return 0;

  CFUUIDRef uuid = CFUUIDCreateFromString(NULL, uuid_string);
  uint32_t id = CGDisplayGetDisplayIDFromUUID(uuid);

  CFRelease(uuid);
  CFRelease(uuid_string);

  return id;
}

static inline void error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

static inline int get_wid_from_cg_event(CGEventRef event) {
  return CGEventGetIntegerValueField(event, 0x33);
}
