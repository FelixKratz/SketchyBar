#include "power.h"
#include "event.h"

uint32_t g_power_source = 0;

void power_handler(void* context) {
  CFTypeRef info = IOPSCopyPowerSourcesInfo();
  CFStringRef type = IOPSGetProvidingPowerSourceType(info);

  if (CFStringCompare(type, POWER_AC_KEY, 0) == 0) {
    if (g_power_source != POWER_AC) {
      g_power_source = POWER_AC;
      char* source = malloc(sizeof(char)*8);
      snprintf(source, 8, "AC");
      struct event *event = event_create(&g_event_loop,
                                         POWER_SOURCE_CHANGED,
                                         (void *) source);

      event_loop_post(&g_event_loop, event);
    }
  } else if (CFStringCompare(type, POWER_BATTERY_KEY, 0) == 0) {
    if (g_power_source != POWER_BATTERY) {
      g_power_source = POWER_BATTERY;
      char* source = malloc(sizeof(char)*8);
      snprintf(source, 8, "BATTERY");
      struct event *event = event_create(&g_event_loop,
                                         POWER_SOURCE_CHANGED,
                                         (void *) source);

      event_loop_post(&g_event_loop, event);
    }
  }
  CFRelease(info);
}

void begin_receiving_power_events() {
  CFRunLoopSourceRef source = IOPSNotificationCreateRunLoopSource(power_handler, NULL);
  CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopDefaultMode);
}
