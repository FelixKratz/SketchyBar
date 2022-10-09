#include "volume.h"
#include "event.h"

extern bool g_volume_events;

static AudioObjectPropertyAddress kVolumePropertyAddress = {
                                            kAudioDevicePropertyVolumeScalar,
                                            kAudioObjectPropertyScopeOutput,
                                            kAudioObjectPropertyElementMain  };

static AudioObjectPropertyAddress kMutePropertyAddress = {
                                            kAudioDevicePropertyMute,
                                            kAudioObjectPropertyScopeOutput,
                                            kAudioObjectPropertyElementMain  };

float g_last_volume = 0.f;
static OSStatus handler(AudioObjectID id, uint32_t address_count, const AudioObjectPropertyAddress* addresses, void* context) {
  float* volume = malloc(sizeof(float));
  memset(volume, 0, sizeof(float));

  uint32_t muted = 0;
  uint32_t size = sizeof(uint32_t);

  AudioObjectGetPropertyData(id,
                             &kMutePropertyAddress,
                             0,
                             NULL,
                             &size,
                             &muted                );

  size = sizeof(float);
  AudioObjectGetPropertyData(id,
                             &kVolumePropertyAddress,
                             0,
                             NULL,
                             &size,
                             volume                 );

  if (muted) *volume = 0.f;
  if (*volume > g_last_volume + 1e-2 || *volume < g_last_volume - 1e-2) {
    g_last_volume = *volume;
    struct event *event = event_create(&g_event_loop,
                                       VOLUME_CHANGED,
                                       (void *) volume);

    event_loop_post(&g_event_loop, event);
  }
  return KERN_SUCCESS;
}

void begin_receiving_volume_events() {
  if (g_volume_events) return;
  AudioObjectPropertyAddress output_addr = {
                                   kAudioHardwarePropertyDefaultOutputDevice,
                                   kAudioObjectPropertyScopeGlobal,
                                   kAudioObjectPropertyElementMain           };

  AudioObjectID id = 0;
  uint32_t size = sizeof(AudioObjectID);
  AudioObjectGetPropertyData(kAudioObjectSystemObject,
                             &output_addr,
                             0,
                             NULL,
                             &size,
                             &id                      );

  AudioObjectAddPropertyListener(id, &kMutePropertyAddress, &handler, NULL);
  AudioObjectAddPropertyListener(id, &kVolumePropertyAddress, &handler, NULL);
}
