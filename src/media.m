#include <Carbon/Carbon.h>
#include "event.h"
#include "event_loop.h"

extern struct event_loop g_event_loop;

extern void MRMediaRemoteRegisterForNowPlayingNotifications(dispatch_queue_t queue);
extern void MRMediaRemoteGetNowPlayingInfo(dispatch_queue_t queue, void (^block)(NSDictionary* dict));
extern void MRMediaRemoteGetNowPlayingApplicationIsPlaying(dispatch_queue_t queue, void (^block)(BOOL playing));

extern NSString* kMRMediaRemoteNowPlayingApplicationIsPlayingDidChangeNotification;
extern NSString* kMRMediaRemoteNowPlayingInfoDidChangeNotification;
extern NSString* kMRMediaRemoteNowPlayingApplicationDidChangeNotification;

extern NSString* kMRMediaRemoteNowPlayingInfoAlbum;
extern NSString* kMRMediaRemoteNowPlayingInfoArtist;
extern NSString* kMRMediaRemoteNowPlayingInfoTitle;
extern NSString* kMRMediaRemoteNowPlayingApplicationDisplayNameUserInfoKey;

@interface media_context : NSObject {}
  @property (retain) NSString* app;
  @property (retain) NSString* artist;
  @property (retain) NSString* title;
  @property (retain) NSString* album;
  @property BOOL playing;
- (id)init;
@end

@implementation media_context
- (id)init {
    if ((self = [super init])) {
      [NSNotificationCenter.defaultCenter addObserver:self
                                          selector:@selector(media_change:)
                                          name:kMRMediaRemoteNowPlayingInfoDidChangeNotification
                                          object:nil];

      [NSNotificationCenter.defaultCenter addObserver:self
                                          selector:@selector(playing_change:)
                                          name:kMRMediaRemoteNowPlayingApplicationIsPlayingDidChangeNotification
                                          object:nil];

      [NSNotificationCenter.defaultCenter addObserver:self
                                          selector:@selector(media_change:)
                                          name:kMRMediaRemoteNowPlayingApplicationDidChangeNotification
                                          object:nil];
    }
    return self;
}

char* g_media_info = NULL;
bool g_media_events = false;
- (void) update {
  @autoreleasepool {
    if (self.app && self.artist && self.title && self.album) {
      const char* app_cstr = [self.app UTF8String];
      const char* artist_cstr = [self.artist UTF8String];
      const char* title_cstr = [self.title UTF8String];
      const char* album_cstr = [self.album UTF8String];
      uint32_t info_len = strlen(app_cstr)
                          + strlen(artist_cstr)
                          + strlen(title_cstr)
                          + strlen(album_cstr) + 256;

      char info[info_len];
      snprintf(info, info_len, "{\n"
                               "\t\"state\": \"%s\",\n"
                               "\t\"title\": \"%s\",\n"
                               "\t\"album\": \"%s\",\n"
                               "\t\"artist\": \"%s\",\n"
                               "\t\"app\": \"%s\"\n}",
                               self.playing ? "playing" : "paused",
                               title_cstr,
                               album_cstr,
                               artist_cstr,
                               app_cstr                            );

      if (!g_media_info || strcmp(info, g_media_info) != 0) {
        g_media_info = realloc(g_media_info, info_len);
        memcpy(g_media_info, info, info_len);

        char* payload_info = malloc(info_len);
        memcpy(payload_info, info, info_len);

        struct event *event = event_create(&g_event_loop,
                                           MEDIA_CHANGED,
                                           payload_info  );
        event_loop_post(&g_event_loop, event);
      }
    }
  }
}

- (void)playing_change:(NSNotification *)notification {
  if (!g_media_events) return;
  MRMediaRemoteGetNowPlayingApplicationIsPlaying(dispatch_get_main_queue(), ^(BOOL playing) {
    self.playing = playing;
    [self media_change:notification];
  });
}

- (void)media_change:(NSNotification *)notification {
  if (!g_media_events) return;
  MRMediaRemoteGetNowPlayingInfo(dispatch_get_main_queue(), ^(NSDictionary* dict) {
    @autoreleasepool {
      self.app = [notification.userInfo valueForKey:kMRMediaRemoteNowPlayingApplicationDisplayNameUserInfoKey];
      self.artist = [dict valueForKey:kMRMediaRemoteNowPlayingInfoArtist];
      self.title = [dict valueForKey:kMRMediaRemoteNowPlayingInfoTitle];
      self.album = [dict valueForKey:kMRMediaRemoteNowPlayingInfoAlbum];

      [self update];
    }
  });
}
@end

media_context* g_media_context = NULL;
void initialize_media_events() {
  MRMediaRemoteRegisterForNowPlayingNotifications(dispatch_get_main_queue());

  g_media_context = [media_context alloc];
  [g_media_context init];
}

void begin_receiving_media_events() {
  g_media_events = true;
}
