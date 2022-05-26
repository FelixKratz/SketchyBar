#pragma once
#include "misc/helpers.h"

#define ANIMATE(f, o, p, t) \
{\
  if (g_bar_manager.animator.duration > 0) { \
    struct animation* animation = animation_create(); \
    animation_setup(animation, \
                    (void*)o, \
                    (bool (*)(void*, int))&f, \
                    p, \
                    t, \
                    g_bar_manager.animator.duration, \
                    g_bar_manager.animator.interp_function ); \
    animator_add(&g_bar_manager.animator, animation); \
  } else { \
    needs_refresh = f(o, t); \
  } \
}

#define ANIMATE_BYTES(f, o, p, t) \
{\
  if (g_bar_manager.animator.duration > 0) { \
    struct animation* animation = animation_create(); \
    animation_setup(animation, \
                    (void*)o, \
                    (bool (*)(void*, int))&f, \
                    p, \
                    t, \
                    g_bar_manager.animator.duration, \
                    g_bar_manager.animator.interp_function ); \
    animation->seperate_bytes = true; \
    animator_add(&g_bar_manager.animator, animation); \
  } else { \
    needs_refresh = f(o, t); \
  } \
}

#define ANIMATOR_FUNCTION(name) bool name(void* target, int value);
typedef ANIMATOR_FUNCTION(animator_function);

#define ANIMATION_FUNCTION(name) double name(double x);
typedef ANIMATION_FUNCTION(animation_function);

#define INTERP_FUNCTION_LINEAR       'l'
#define INTERP_FUNCTION_SIN          's'
#define INTERP_FUNCTION_TANH         't'
#define INTERP_FUNCTION_BOUNCE       'b'
#define INTERP_FUNCTION_EXP          'e'
#define INTERP_FUNCTION_OVERSHOOT    'o'


struct animation {
  bool seperate_bytes;

  uint32_t duration;
  uint32_t counter;

  int initial_value;
  int final_value;
  animation_function* interp_function;

  void* target;
  animator_function* update_function;
};

struct animation* animation_create();
void animation_destroy(struct animation* animation);

void animation_setup(struct animation* animation, void* target, animator_function* update_function, int initial_value, int final_value, uint32_t duration, char interp_function);
bool animation_update(struct animation* animation);

extern struct event_loop g_event_loop;

#define ANIMATOR_CALLBACK(name) void name(CFRunLoopTimerRef timer, void *context)
typedef ANIMATOR_CALLBACK(animator_callback);

struct animator {
  CFRunLoopTimerRef clock;

  uint32_t interp_function;
  uint32_t duration;
  struct animation** animations;
  uint32_t animation_count;
};

void animator_init(struct animator* animator);
void animator_add(struct animator* animator, struct animation* animation);
void animator_remove(struct animator* animator, struct animation* animation);
bool animator_update(struct animator* animator);
