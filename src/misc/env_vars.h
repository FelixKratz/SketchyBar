#pragma once
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

struct key_value_pair {
  char* key;
  char* value;
};

struct env_vars {
  uint32_t count;
  struct key_value_pair** vars;
};

static inline void env_vars_init(struct env_vars* env_vars) {
  env_vars->vars = NULL;
  env_vars->count = 0;
}

static inline void env_vars_unset(struct env_vars* env_vars, char* key) {
  struct key_value_pair* key_value_pair = NULL;
  for (int i = 0; i < env_vars->count; i++) {
    if (strcmp(env_vars->vars[i]->key, key) == 0)
      key_value_pair = env_vars->vars[i];
  }

  if (key_value_pair == NULL) return;

  if (env_vars->count == 1) {
    free(env_vars->vars);
    env_vars->vars = NULL;
    env_vars->count = 0;
  } else {
    struct key_value_pair* tmp[env_vars->count - 1];
    int count = 0;
    for (int i = 0; i < env_vars->count; i++) {
      if (env_vars->vars[i] == key_value_pair) continue;
      tmp[count++] = env_vars->vars[i];
    }
    env_vars->count--;
    env_vars->vars = realloc(env_vars->vars,
                             sizeof(struct key_value_pair*)*env_vars->count);

    memcpy(env_vars->vars,
           tmp,
           sizeof(struct key_value_pair*)*env_vars->count);
  }

  if (key_value_pair->key) free(key_value_pair->key);
  if (key_value_pair->value) free(key_value_pair->value);
  free(key_value_pair);
}

static inline void env_vars_set(struct env_vars* env_vars, char* key, char* value) {
  env_vars_unset(env_vars, key);

  env_vars->count++;
  env_vars->vars = realloc(env_vars->vars,
                           env_vars->count * sizeof(struct key_value_pair*));

  env_vars->vars[env_vars->count - 1] = malloc(sizeof(struct key_value_pair));
  env_vars->vars[env_vars->count - 1]->key = key;
  env_vars->vars[env_vars->count - 1]->value = value;
}

static inline char* env_vars_get_value_for_key(struct env_vars* env_vars, char* key) {
  for (int i = 0; i < env_vars->count; i++) {
    if (strcmp(env_vars->vars[i]->key, key) == 0)
      return env_vars->vars[i]->value;
  }
  return NULL;
}

static inline char* env_vars_copy_serialized_representation(struct env_vars* env_vars, uint32_t* len) {
  uint32_t length = 0;
  for (int i = 0; i < env_vars->count; i++) {
    length += env_vars->vars[i]->key
              ? strlen(env_vars->vars[i]->key) + 1
              : 1;

    length += env_vars->vars[i]->value
              ? strlen(env_vars->vars[i]->value) + 1
              : 1;
  }

  uint32_t caret = 0;
  char* seri = (char*)malloc(++length);
  for (int i = 0; i < env_vars->count; i++) {
    if (env_vars->vars[i]->key) {
      uint32_t len = strlen(env_vars->vars[i]->key) + 1;
      memcpy(seri + caret,
             env_vars->vars[i]->key,
             len                    );

      caret += len;
    } else {
      seri[caret++] = '\0';
    }

    if (env_vars->vars[i]->value) {
      uint32_t len = strlen(env_vars->vars[i]->value) + 1;
      memcpy(seri + caret,
             env_vars->vars[i]->value,
             len                      );

      caret += len;
    } else {
      seri[caret++] = '\0';
    }
  }
  seri[caret++] = '\0';
  assert(caret == length);
  *len = length;
  return seri;
}

static inline void env_vars_destroy(struct env_vars* env_vars) {
  for (int i = 0; i < env_vars->count; i++) {
    if (env_vars->vars[i]->key) free(env_vars->vars[i]->key);
    if (env_vars->vars[i]->value) free(env_vars->vars[i]->value);
    free(env_vars->vars[i]);
  }
  free(env_vars->vars);
}
