#pragma once
#include <CoreVideo/CVDisplayLink.h>
#include "misc/helpers.h"

extern struct bar_manager g_bar_manager;

#define ANIMATE(f, o, p, t) \
{\
  if (g_bar_manager.animator.duration > 0) { \
    animator_cancel_locked(&g_bar_manager.animator, (void*)o, (bool (*)(void*, int))&f); \
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
    needs_refresh = animator_cancel(&g_bar_manager.animator, (void*)o, (bool (*)(void*, int))&f); \
    needs_refresh |= f(o, t); \
  } \
}

#define ANIMATE_FLOAT(f, o, p, t) \
{\
  if (g_bar_manager.animator.duration > 0) { \
    animator_cancel_locked(&g_bar_manager.animator, (void*)o, (bool (*)(void*, int))&f); \
    struct animation* animation = animation_create(); \
    float initial_value = p; \
    float final_value = t; \
    animation_setup(animation, \
                    (void*)o, \
                    (bool (*)(void*, int))&f, \
                    *(int*)&initial_value, \
                    *(int*)&final_value, \
                    g_bar_manager.animator.duration, \
                    g_bar_manager.animator.interp_function ); \
    animation->as_float = true; \
    animator_add(&g_bar_manager.animator, animation); \
  } else { \
    needs_refresh = animator_cancel(&g_bar_manager.animator, (void*)o, (bool (*)(void*, int))&f); \
    needs_refresh |= f(o, t); \
  } \
}

#define ANIMATE_BYTES(f, o, p, t) \
{\
  if (g_bar_manager.animator.duration > 0) { \
    animator_cancel_locked(&g_bar_manager.animator, (void*)o, (bool (*)(void*, int))&f); \
    struct animation* animation = animation_create(); \
    animation_setup(animation, \
                    (void*)o, \
                    (bool (*)(void*, int))&f, \
                    p, \
                    t, \
                    g_bar_manager.animator.duration, \
                    g_bar_manager.animator.interp_function ); \
    animation->separate_bytes = true; \
    animator_add(&g_bar_manager.animator, animation); \
  } else { \
    needs_refresh = animator_cancel(&g_bar_manager.animator, (void*)o, (bool (*)(void*, int))&f); \
    needs_refresh |= f(o, t); \
  } \
}

#define ANIMATOR_FUNCTION(name) bool name(void* target, int value);
typedef ANIMATOR_FUNCTION(animator_function);

#define ANIMATION_FUNCTION(name) double name(double x);
typedef ANIMATION_FUNCTION(animation_function);

#define INTERP_FUNCTION_LINEAR    'l'
#define INTERP_FUNCTION_QUADRATIC 'q'
#define INTERP_FUNCTION_SIN       's'
#define INTERP_FUNCTION_TANH      't'
#define INTERP_FUNCTION_CIRC      'c'
#define INTERP_FUNCTION_BOUNCE    'b'
#define INTERP_FUNCTION_EXP       'e'
#define INTERP_FUNCTION_OVERSHOOT 'o'


struct animation {
  bool separate_bytes;
  bool as_float;
  bool locked;
  bool finished;

  double duration;
  double counter;
  double offset;

  int initial_value;
  int final_value;
  animation_function* interp_function;

  void* target;
  animator_function* update_function;
};

struct animation* animation_create();
void animation_setup(struct animation* animation, void* target, animator_function* update_function, int initial_value, int final_value, uint32_t duration, char interp_function);

struct animator {
  CVDisplayLinkRef display_link;

  double time_scale;
  uint32_t interp_function;
  uint32_t duration;
  struct animation** animations;
  uint32_t animation_count;
};

void animator_init(struct animator* animator);
void animator_add(struct animator* animator, struct animation* animation);

bool animator_cancel(struct animator* animator, void* target, animator_function* function);
void animator_cancel_locked(struct animator* animator, void* target, animator_function* function);

bool animator_update(struct animator* animator);
void animator_lock(struct animator* animator);
void animator_destroy(struct animator* animator);

void animator_renew_display_link(struct animator* animator);
void animator_destroy_display_link(struct animator* animator);
