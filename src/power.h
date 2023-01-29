#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/ps/IOPowerSources.h>

#define POWER_AC_KEY      CFSTR(kIOPMACPowerKey)
#define POWER_BATTERY_KEY CFSTR(kIOPMBatteryPowerKey)
#define POWER_UPS_KEY     CFSTR(kIOPMUPSPowerKey)

#define POWER_AC 1
#define POWER_BATTERY 2

void forced_power_event();
void begin_receiving_power_events();
