#ifndef ENV_VARS_H
#define ENV_VARS_H

#include <_types/_uint32_t.h>
#include <string.h>

struct key_value_pair {
  char* key;
  char* value;
};

struct env_vars {
  uint32_t count;
  struct key_value_pair** vars;
};

void env_vars_init(struct env_vars* env_vars) {
  env_vars->vars = NULL;
  env_vars->count = 0;
}

void env_vars_unset(struct env_vars* env_vars, char* key) {
  struct key_value_pair* key_value_pair = NULL;
  for (int i = 0; i < env_vars->count; i++) {
    if (strcmp(env_vars->vars[i]->key, key) == 0) key_value_pair = env_vars->vars[i];
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
    env_vars->vars = realloc(env_vars->vars, sizeof(struct key_value_pair*)*env_vars->count);
    memcpy(env_vars->vars, tmp, sizeof(struct key_value_pair*)*env_vars->count);
  }

  if (key_value_pair->key) free(key_value_pair->key);
  if (key_value_pair->value) free(key_value_pair->value);
  free(key_value_pair);
}

void env_vars_set(struct env_vars* env_vars, char* key, char* value) {
  env_vars_unset(env_vars, key);

  env_vars->count++;
  env_vars->vars = realloc(env_vars->vars, env_vars->count * sizeof(struct key_value_pair*));
  env_vars->vars[env_vars->count - 1] = malloc(sizeof(struct key_value_pair));
  env_vars->vars[env_vars->count - 1]->key = key;
  env_vars->vars[env_vars->count - 1]->value = value;
}

char* env_vars_get_value_for_key(struct env_vars* env_vars, char* key) {
  for (int i = 0; i < env_vars->count; i++) {
    if (strcmp(env_vars->vars[i]->key, key) == 0) return env_vars->vars[i]->value;
  }
  return NULL;
}

void env_vars_destroy(struct env_vars* env_vars) {
  for (int i = 0; i < env_vars->count; i++) {
    if (env_vars->vars[i]->key) free(env_vars->vars[i]->key);
    if (env_vars->vars[i]->value) free(env_vars->vars[i]->value);
    free(env_vars->vars[i]);
  }
  free(env_vars->vars);
}

#endif
