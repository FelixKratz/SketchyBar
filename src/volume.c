#include "volume.h"
#include "event.h"

extern bool g_volume_events;

#if __MAC_OS_X_VERSION_MAX_ALLOWED < 120000
#define kAudioObjectPropertyElementMain kAudioObjectPropertyElementMaster
#endif

static AudioObjectPropertyAddress kHardwareDevicePropertyAddress = {
                                   kAudioHardwarePropertyDefaultOutputDevice,
                                   kAudioObjectPropertyScopeGlobal,
                                   kAudioObjectPropertyElementMain           };

static AudioObjectPropertyAddress kVolumeMainPropertyAddress = {
                                            kAudioDevicePropertyVolumeScalar,
                                            kAudioObjectPropertyScopeOutput,
                                            kAudioObjectPropertyElementMain  };

static AudioObjectPropertyAddress kVolumeLeftPropertyAddress = {
                                            kAudioDevicePropertyVolumeScalar,
                                            kAudioObjectPropertyScopeOutput,
                                            1                                };

static AudioObjectPropertyAddress kMuteMainPropertyAddress = {
                                            kAudioDevicePropertyMute,
                                            kAudioObjectPropertyScopeOutput,
                                            kAudioObjectPropertyElementMain  };

static AudioObjectPropertyAddress kMuteLeftPropertyAddress = {
                                            kAudioDevicePropertyMute,
                                            kAudioObjectPropertyScopeOutput,
                                            1                                };

static float g_last_volume = -1.f;
static int g_last_raw_volume = -1;
static int g_last_muted = -1;
static OSStatus handler(AudioObjectID id, uint32_t address_count, const AudioObjectPropertyAddress* addresses, void* context) {
  uint32_t muted_main = 0;
  uint32_t size = sizeof(uint32_t);

  OSStatus mute_main_status = AudioObjectGetPropertyData(id,
                                                          &kMuteMainPropertyAddress,
                                                          0,
                                                          NULL,
                                                          &size,
                                                          &muted_main               );

  uint32_t muted_left = 0;
  size = sizeof(uint32_t);
  AudioObjectGetPropertyData(id,
                             &kMuteLeftPropertyAddress,
                             0,
                             NULL,
                             &size,
                             &muted_left               );


  size = sizeof(float);
  float volume_main = 0.f;
  OSStatus volume_main_status = AudioObjectGetPropertyData(id,
                                                            &kVolumeMainPropertyAddress,
                                                            0,
                                                            NULL,
                                                            &size,
                                                            &volume_main                );

  size = sizeof(float);
  float volume_left = 0.f;
  OSStatus volume_left_status = AudioObjectGetPropertyData(id,
                                                            &kVolumeLeftPropertyAddress,
                                                            0,
                                                            NULL,
                                                            &size,
                                                            &volume_left                );

  struct volume_info volume = { 0, -1, -1 };
  // Preserve INFO's channel selection and mute handling.
  if (volume_left > 0.f) {
    volume.effective_volume = (muted_left || muted_main) ? 0.f : volume_left;
  } else {
    volume.effective_volume = muted_main ? 0.f : volume_main;
  }

  if (mute_main_status == noErr) volume.muted = muted_main != 0;

  if (volume_main_status == noErr || volume_left_status == noErr) {
    float scalar = volume_main_status == noErr ? volume_main : 0.f;
    if (volume_left_status == noErr
        && (volume_left > 0.f || volume_main_status != noErr)) {
      scalar = volume_left;
    }
    volume.volume = (int)(scalar*100. + 0.5);
  }

  // Metadata-only events must not change INFO's threshold baseline.
  bool volume_changed = volume.effective_volume > g_last_volume + 1e-2
                        || volume.effective_volume < g_last_volume - 1e-2;
  if (volume_changed) g_last_volume = volume.effective_volume;

  if (volume_changed
      || volume.volume != g_last_raw_volume
      || volume.muted != g_last_muted) {
    g_last_raw_volume = volume.volume;
    g_last_muted = volume.muted;
    struct event event = { (void*) &volume, VOLUME_CHANGED };
    event_post(&event);
  } 
  return KERN_SUCCESS;
}

static AudioObjectID g_audio_id = 0;
OSStatus device_changed(AudioObjectID id, uint32_t address_count, const AudioObjectPropertyAddress* addresses, void* context) {
  AudioObjectID new_id = 0;
  uint32_t size = sizeof(AudioObjectID);
  AudioObjectGetPropertyData(kAudioObjectSystemObject,
                             &kHardwareDevicePropertyAddress,
                             0,
                             NULL,
                             &size,
                             &new_id                         );

  if (g_audio_id) {
    AudioObjectRemovePropertyListener(g_audio_id,
                                      &kMuteMainPropertyAddress,
                                      &handler,
                                      NULL                      );

    AudioObjectRemovePropertyListener(g_audio_id,
                                      &kMuteLeftPropertyAddress,
                                      &handler,
                                      NULL                      );

    AudioObjectRemovePropertyListener(g_audio_id,
                                      &kVolumeMainPropertyAddress,
                                      &handler,
                                      NULL                        );

    AudioObjectRemovePropertyListener(g_audio_id,
                                      &kVolumeLeftPropertyAddress,
                                      &handler,
                                      NULL                        );
  }

  AudioObjectAddPropertyListener(new_id,
                                 &kMuteMainPropertyAddress,
                                 &handler,
                                 NULL                      );

  AudioObjectAddPropertyListener(new_id,
                                 &kMuteLeftPropertyAddress,
                                 &handler,
                                 NULL                      );

  AudioObjectAddPropertyListener(new_id,
                                 &kVolumeMainPropertyAddress,
                                 &handler,
                                 NULL                        );

  AudioObjectAddPropertyListener(new_id,
                                 &kVolumeLeftPropertyAddress,
                                 &handler,
                                 NULL                        );
  g_last_volume = -1.f;
  g_audio_id = new_id;
  handler(g_audio_id, address_count, addresses, context);
  return KERN_SUCCESS;
}

void forced_volume_event() {
  g_last_volume = -1.f;
  handler(g_audio_id, 0, 0, 0);
}

void begin_receiving_volume_events() {
  if (g_volume_events) return;
  g_volume_events = true;

  AudioObjectID id = 0;
  uint32_t size = sizeof(AudioObjectID);
  AudioObjectGetPropertyData(kAudioObjectSystemObject,
                             &kHardwareDevicePropertyAddress,
                             0,
                             NULL,
                             &size,
                             &id                             );

  g_audio_id = id;
  AudioObjectAddPropertyListener(id,
                                 &kMuteLeftPropertyAddress,
                                 &handler,
                                 NULL                      );

  AudioObjectAddPropertyListener(id,
                                 &kMuteMainPropertyAddress,
                                 &handler,
                                 NULL                      );

  AudioObjectAddPropertyListener(id,
                                 &kVolumeLeftPropertyAddress,
                                 &handler,
                                 NULL                        );

  AudioObjectAddPropertyListener(id,
                                 &kVolumeMainPropertyAddress,
                                 &handler,
                                 NULL                        );

  AudioObjectAddPropertyListener(kAudioObjectSystemObject,
                                 &kHardwareDevicePropertyAddress,
                                 &device_changed,
                                 NULL                            );
}
