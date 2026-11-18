#pragma once
#include "CoreAudio/CoreAudio.h"
#include <CoreAudio/AudioHardware.h>

struct volume_info {
  float effective_volume;
  int volume;
  int muted;
};

void forced_volume_event();
void begin_receiving_volume_events();
