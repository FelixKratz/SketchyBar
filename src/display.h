#pragma once
#include "event.h"
#include "misc/helpers.h"

#define DISPLAY_EVENT_HANDLER(name) void name(uint32_t did, CGDisplayChangeSummaryFlags flags, void *context)
typedef DISPLAY_EVENT_HANDLER(display_callback);

uint32_t display_main_display_id(void);
uint32_t display_active_display_id(void);
uint32_t display_active_display_adid(void);
uint32_t display_arrangement_display_id(int arrangement);
bool display_menu_bar_visible(void);
CGRect display_menu_bar_rect(uint32_t did);
uint32_t display_active_display_count(void);
uint32_t* display_active_display_list(uint32_t* count);
bool display_begin(void);
bool display_end(void);

CFStringRef display_uuid(uint32_t did);
CGRect display_bounds(uint32_t did);
uint64_t display_space_id(uint32_t did);
uint64_t* display_space_list(uint32_t did, int* count);
int display_arrangement(uint32_t did);

void forced_brightness_event();
void begin_receiving_brightness_events();

void display_serialize(FILE* rsp);
