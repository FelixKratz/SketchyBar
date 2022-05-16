#pragma once
#include "misc/helpers.h"

struct animation {
  uint32_t duration;
  uint32_t counter;

  uint32_t initial_value;
  uint32_t final_value;

  uint32_t* target;
};

struct animation* animation_create();
void animation_destroy(struct animation* animation);

void animation_setup(struct animation* animation, uint32_t* property, uint32_t final_value, uint32_t duration);
bool animation_update(struct animation* animation);

extern struct event_loop g_event_loop;

#define ANIMATOR_CALLBACK(name) void name(CFRunLoopTimerRef timer, void *context)
typedef ANIMATOR_CALLBACK(animator_callback);

struct animator {
  CFRunLoopTimerRef clock;

  struct animation** animations;
  uint32_t animation_count;
};

void animator_init(struct animator* animator);
void animator_add(struct animator* animator, struct animation* animation);
void animator_remove(struct animator* animator, struct animation* animation);
void animator_update(struct animator* animator);
