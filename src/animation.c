#include "animation.h"
#include "event.h"
 
double function_linear(double x) {
  return x;
}

double function_tanh(double x) {
  double result = tanh(2.647 * x);
  return result >= 0.99 ? 1. : result;
}

// double function_bounce(double x) {
//   double alpha = 2.;
//   double beta = 0.8;
//   if (x < 1. / alpha) {
//     return alpha*alpha * x * x;
//   }
//   else {
//     return beta * beta * (x - 1./2. + 1./alpha/2.) + 1. - beta*beta*(1./2. + 1./alpha/2.);
//   }
// }

static ANIMATOR_CALLBACK(animator_handler) {
  struct event *event = event_create(&g_event_loop, ANIMATOR_REFRESH, NULL);
  event_loop_post(&g_event_loop, event);
}

struct animation* animation_create() {
  struct animation* animation = malloc(sizeof(struct animation));
  memset(animation, 0, sizeof(struct animation));

  return animation;
}

void animation_destroy(struct animation* animation) {
  if (animation) free(animation);
}

void animation_setup(struct animation* animation, void* target, animator_function* update_function, int initial_value, int final_value, uint32_t duration, char interp_function) {
  animation->counter = 1;
  animation->duration = duration;
  animation->initial_value = initial_value;
  animation->final_value = final_value;
  animation->update_function = update_function;
  animation->interp_function = interp_function;
  animation->target = target;
}

bool animation_update(struct animation* animation) {
  if (!animation->target
      || !animation->update_function
      || animation->counter > animation->duration) {
    return false;
  }

  double slider = 1.;
  if (animation->interp_function == INTERP_FUNCTION_LINEAR) {
    slider = function_linear((double)animation->counter
                             / (double)animation->duration);
  }
  else if (animation->interp_function == INTERP_FUNCTION_TANH) {
    slider = function_tanh((double)animation->counter
                           / (double)animation->duration);
  }

  int value = (1. - slider) * animation->initial_value
              + slider * animation->final_value;

  animation->counter++;
  return animation->update_function(animation->target, value);
}

void animator_init(struct animator* animator) {
  animator->animations = NULL;
  animator->animation_count = 0;
  animator->interp_function = 0;
  animator->duration = 0;
}

void animator_add(struct animator* animator, struct animation* animation) {
  animator->animations = realloc(animator->animations,
                                 sizeof(struct animaton*)
                                        * ++animator->animation_count);
  animator->animations[animator->animation_count - 1] = animation;

  if (animator->animation_count == 1) {

    animator->clock = CFRunLoopTimerCreate(NULL,
                                           CFAbsoluteTimeGetCurrent()+1./60.,
                                           1./60.,
                                           0,
                                           0,
                                           animator_handler,
                                           NULL                              );

    CFRunLoopAddTimer(CFRunLoopGetMain(),
                      animator->clock,
                      kCFRunLoopCommonModes);
  }
}

void animator_remove(struct animator* animator, struct animation* animation) {
  if (animator->animation_count == 1) {
    free(animator->animations);
    animator->animations = NULL;
    animator->animation_count = 0;
  } else {
    struct animation* tmp[animator->animation_count - 1];
    int count = 0;
    for (int i = 0; i < animator->animation_count; i++) {
      if (animator->animations[i] == animation) continue;
      tmp[count++] = animator->animations[i];
    }
    animator->animation_count--;
    animator->animations = realloc(animator->animations,
                                   sizeof(struct animation*)
                                          *animator->animation_count);

    memcpy(animator->animations,
           tmp,
           sizeof(struct animation*)*animator->animation_count);
  }

  animation_destroy(animation);

  if (animator->animation_count == 0) {
    CFRunLoopRemoveTimer(CFRunLoopGetMain(),
                         animator->clock,
                         kCFRunLoopCommonModes);
    CFRelease(animator->clock);
  }
}

bool animator_update(struct animator* animator) {
  bool removed = false;
  bool needs_refresh = false;
  for (uint32_t i = 0; i < animator->animation_count; i++) {
    if (removed) i--;
    removed = false;
    needs_refresh |= animation_update(animator->animations[i]);
    if (animator->animations[i]->counter > animator->animations[i]->duration) {
      animator_remove(animator, animator->animations[i]);
      removed = true;
    }
  }
  return needs_refresh;
}
