#include "battery.h"
#include "event.h"

/* Put escaped quotation marks around something */
#define q(x) "\""x"\""

void battery_handler(void *context) {
  /* Power source (UPS Power, Battery Power, AC Power) */
  char source[16];

  /* Battery health (Poor, Fair, Good) */
  char health[8];

  /* Battery capacities */
  int current, max = 1;
  double percentage = -1;

  /* Time to full/empty (minutes) */
  int empty, full;

  /* Transport (Serial, USB, Ethernet, Internal) */
  char transport[16];

  /* Final JSON payload to be posted */
  char json[1024];

  /* Get the list of power sources */
  CFTypeRef info = IOPSCopyPowerSourcesInfo();
  CFArrayRef sources = IOPSCopyPowerSourcesList(info);

  /* Loop through each power source */
  /* TODO: What if there is more than one power source? */
  for (CFIndex i = 0; i < CFArrayGetCount(sources); ++i) {
    /* Dictionary containing health, capacity, charging, and transport data */
    CFDictionaryRef cfDescription = IOPSGetPowerSourceDescription(info, CFArrayGetValueAtIndex(sources, i));
    if (!cfDescription) continue;

    /* Source */
    CFStringRef cfSource = IOPSGetProvidingPowerSourceType(info);
    CFStringGetCString(cfSource, source, sizeof(source), kCFStringEncodingUTF8);

    /* Health */
    CFStringRef cfHealth = CFDictionaryGetValue(cfDescription, CFSTR(kIOPSBatteryHealthKey));
    CFStringGetCString(cfHealth, health, sizeof(health), kCFStringEncodingUTF8);

    /* Capacity */
    CFNumberRef currentCap = CFDictionaryGetValue(cfDescription, CFSTR(kIOPSCurrentCapacityKey));
    CFNumberRef maxCap = CFDictionaryGetValue(cfDescription, CFSTR(kIOPSMaxCapacityKey));

    if (currentCap && maxCap) {
      CFNumberGetValue(currentCap, kCFNumberIntType, &current);
      CFNumberGetValue(maxCap, kCFNumberIntType, &max);

      percentage = ((double) current / (double) max) * 100;
    }

    /* Time to full/empty */
    CFNumberRef cfFull = CFDictionaryGetValue(cfDescription, CFSTR(kIOPSTimeToFullChargeKey));
    CFNumberGetValue(cfFull, kCFNumberIntType, &full);

    CFNumberRef cfEmpty = CFDictionaryGetValue(cfDescription, CFSTR(kIOPSTimeToEmptyKey));
    CFNumberGetValue(cfEmpty, kCFNumberIntType, &empty);

    /* Transport */
    CFStringRef cfTransport = CFDictionaryGetValue(cfDescription, CFSTR(kIOPSTransportTypeKey));
    CFStringGetCString(cfTransport, transport, sizeof(transport), kCFStringEncodingUTF8);

    /* Charging */
    CFBooleanRef charging = CFDictionaryGetValue(cfDescription, CFSTR(kIOPSIsChargingKey));

    /* Post event in JSON format */
    snprintf(json, sizeof(json),
	     /* source      health      percentage current max     charge  empty   full    transport */
             "{ %s: \"%s\", %s: \"%s\", %s: %0.0f, %s: %d, %s: %d, %s: %s, %s: %d, %s: %d, %s: \"%s\" }",
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
