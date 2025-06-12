#include "battery.h"
#include "event.h"

/* Put escaped quotation marks around something */
#define q(x) "\""x"\""

/* Safely store a CFStringRef in a char[] */
char *mkstring(CFStringRef s1) {
  if (! s1) { return NULL; }

  long length = CFStringGetLength(s1) + 1;
  if (length <= 1) { return(NULL); }

  char *s2 = malloc(length);
  if (! s2) { return(NULL); }

  CFStringGetCString(s1, s2, length, kCFStringEncodingUTF8);
  return(s2);
}

/* Store a CFNumberRef in a long */
long mknumber(CFNumberRef n1) {
  long n2;

  CFNumberGetValue(n1, kCFNumberIntType, &n2);

  return(n2);
}

void battery_handler(void *context) {
  /* Power source (UPS Power, Battery Power, AC Power) */
  char *source;

  /* Battery health (Poor, Fair, Good) */
  char *health;

  /* Battery capacities */
  int current, max = 1;
  double percentage = -1;

  /* Time to full/empty (minutes) */
  int empty, full;

  /* Transport (Serial, USB, Ethernet, Internal) */
  char *transport;

  /*
    Final JSON payload to be posted.  Largest possible payload is 135
    bytes plus the lengths of the string values.

    {"source":"","health":"","percentage":100,"current":100,"max":100,
    "charging":false,"empty":9223372036854775807,"full":9223372036854775807,"transport":""}
  */
  int length = 256;
  char *json;

  /* Get the list of power sources */
  CFTypeRef info = IOPSCopyPowerSourcesInfo();
  CFArrayRef sources = IOPSCopyPowerSourcesList(info);

  /* Loop through each power source */
  /* TODO: What if there is more than one power source? */
  for (CFIndex i = 0; i < CFArrayGetCount(sources); ++i) {
    /* Dictionary containing health, capacity, charging, and transport data */
    CFDictionaryRef cfDesc = IOPSGetPowerSourceDescription(info, CFArrayGetValueAtIndex(sources, i));
    if (! cfDesc) continue;

    /* Source */
    source = mkstring(IOPSGetProvidingPowerSourceType(info));
    length += strlen(source);

    /* Health */
    health = mkstring(CFDictionaryGetValue(cfDesc, CFSTR(kIOPSBatteryHealthKey)));
    length += strlen(health);

    /* Capacity */
    current = mknumber(CFDictionaryGetValue(cfDesc, CFSTR(kIOPSCurrentCapacityKey)));
    max = mknumber(CFDictionaryGetValue(cfDesc, CFSTR(kIOPSMaxCapacityKey)));

    if ((current) && (max) && (max > 0)) {
      percentage = ((double)current / (double)max) * 100;
    }

    /* Time to full/empty */
    full = mknumber(CFDictionaryGetValue(cfDesc, CFSTR(kIOPSTimeToFullChargeKey)));
    empty = mknumber(CFDictionaryGetValue(cfDesc, CFSTR(kIOPSTimeToEmptyKey)));

    /* Transport */
    transport = mkstring(CFDictionaryGetValue(cfDesc, CFSTR(kIOPSTransportTypeKey)));
    length += strlen(transport);

    /* Charging */
    CFBooleanRef charging = CFDictionaryGetValue(cfDesc, CFSTR(kIOPSIsChargingKey));

    /* Post event in JSON format */
    json = malloc(length);
    snprintf(json, length,
	     /* source      health      percentage current max     charge  empty   full    transport */
             "{%s:\"%s\",%s:\"%s\",%s:%0.0f,%s:%d,%s:%d,%s:%s,%s:%d,%s:%d,%s:\"%s\"}",
             q("source"), source,
             q("health"), health,
	     q("percentage"), percentage,
	     q("current"), current,
             q("max"), max,
	     q("charging"), ((charging == kCFBooleanTrue) ? "true" : "false"),
	     q("empty"), empty,
	     q("full"), full,
	     q("transport"), transport);
    struct event event = {(void *)json, BATTERY_CHANGED};
    event_post(&event);

    /* Free mallocated memory */
    free(source);
    free(health);
    free(transport);
    free(json);
  }

  CFRelease(info);
  CFRelease(sources);
}

void forced_battery_event() {
  battery_handler(NULL);
}

void begin_receiving_battery_events() {
  CFRunLoopSourceRef source = IOPSNotificationCreateRunLoopSource(battery_handler, NULL);
  CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopDefaultMode);
}
