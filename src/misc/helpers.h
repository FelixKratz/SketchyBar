#ifndef HELPERS_H
#define HELPERS_H

#include <_types/_uint32_t.h>
#include <stdint.h>
#define array_count(a) (sizeof((a)) / sizeof(*(a)))
#define MAXLEN 512
#define FORK_TIMEOUT 60

#include <string.h>
extern CFArrayRef SLSCopyManagedDisplaySpaces(int cid);
extern uint32_t SLSGetActiveSpace(int cid);
extern int g_connection;

struct signal_args {
    struct env_vars env_vars;
    void *entity;
    void *param1;
};

struct rgba_color {
    bool is_valid;
    uint32_t p;
    float r;
    float g;
    float b;
    float a;
};

struct token {
    char *text;
    unsigned int length;
};

static uint32_t hex_from_rgba_color(struct rgba_color rgba_color) {
  uint32_t result = 0;
  result += ((uint32_t)(rgba_color.a * 255.0)) << 24;
  result += ((uint32_t)(rgba_color.r * 255.0)) << 16;
  result += ((uint32_t)(rgba_color.g * 255.0)) << 8;
  result += ((uint32_t)(rgba_color.b * 255.0)) << 0;
  return result;
}

static struct rgba_color rgba_color_from_hex(uint32_t color) {
    struct rgba_color result;
    result.is_valid = true;
    result.p = color;
    result.r = ((color >> 16) & 0xff) / 255.0;
    result.g = ((color >> 8) & 0xff) / 255.0;
    result.b = ((color >> 0) & 0xff) / 255.0;
    result.a = ((color >> 24) & 0xff) / 255.0;
    return result;
}

static struct key_value_pair get_key_value_pair(char *token, char split) {
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

static void pack_key_value_pair(char* cursor, struct key_value_pair* key_value_pair) {
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

static bool token_equals(struct token token, char *match) {
  char *at = match;
  for (int i = 0; i < token.length; ++i, ++at) {
    if ((*at == 0) || (token.text[i] != *at)) {
      return false;
    }
  }
  return *at == 0;
}

static char *token_to_string(struct token token) {
  char *result = malloc(token.length + 1);
  if (!result) return NULL;

  memcpy(result, token.text, token.length);
  result[token.length] = '\0';
  return result;
}

static uint32_t token_to_uint32t(struct token token) {
  char buffer[token.length + 1];
  memcpy(buffer, token.text, token.length);
  buffer[token.length] = '\0';
  return strtoul(buffer, NULL, 0);
}

static int token_to_int(struct token token) {
  char buffer[token.length + 1];
  memcpy(buffer, token.text, token.length);
  buffer[token.length] = '\0';
  return (int) strtol(buffer, NULL, 0);
}

static float token_to_float(struct token token) {
  char buffer[token.length + 1];
  memcpy(buffer, token.text, token.length);
  buffer[token.length] = '\0';
  return strtof(buffer, NULL);
}

static struct token get_token(char **message) {
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

static inline uint32_t get_set_bit_position(uint32_t mask) {
  if (mask == 0) return UINT32_MAX;
  uint32_t pos = 0;
  while (!(mask & 1)) {
    mask >>= 1;
    pos++;
  }
  return pos;
}

static inline void draw_rect(CGContextRef context, CGRect region, struct rgba_color* fill_color, uint32_t corner_radius, uint32_t line_width, struct rgba_color* stroke_color, bool clear) {
  CGContextSetLineWidth(context, line_width);
  if (stroke_color) CGContextSetRGBStrokeColor(context, stroke_color->r, stroke_color->g, stroke_color->b, stroke_color->a);
  CGContextSetRGBFillColor(context, fill_color->r, fill_color->g, fill_color->b, fill_color->a);
  
  if (clear) CGContextClearRect(context, region);
  CGMutablePathRef path = CGPathCreateMutable();
  CGRect inset_region = CGRectInset(region, (float)(line_width) / 2.f, (float)(line_width) / 2.f);
  if (corner_radius > inset_region.size.height / 2.f || corner_radius > inset_region.size.width / 2.f)
    corner_radius = inset_region.size.height > inset_region.size.width ? inset_region.size.width / 2.f : inset_region.size.height / 2.f; 
  CGPathAddRoundedRect(path, NULL, inset_region, corner_radius, corner_radius);
  CGContextAddPath(context, path);
  CGContextDrawPath(context, kCGPathFillStroke);
  CFRelease(path);
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

static bool sync_exec(char *command, struct env_vars *env_vars) {
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
static bool fork_exec(char *command, struct env_vars* env_vars) {
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

static inline uint32_t current_space(void) {
  return mission_control_index(SLSGetActiveSpace(g_connection));
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
static inline bool psn_equals(ProcessSerialNumber *a, ProcessSerialNumber *b) {
    Boolean result;
    SameProcess(a, b, &result);
    return result == 1;
}
#pragma clang diagnostic pop

static inline float clampf_range(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

#endif
