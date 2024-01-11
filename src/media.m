#include "event.h"
#include <Foundation/Foundation.h>

extern void MRMediaRemoteRegisterForNowPlayingNotifications(dispatch_queue_t queue);
extern void MRMediaRemoteGetNowPlayingInfo(dispatch_queue_t queue, void (^block)(NSDictionary* dict));
extern void MRMediaRemoteGetNowPlayingApplicationIsPlaying(dispatch_queue_t queue, void (^block)(BOOL playing));
extern void MRMediaRemoteGetNowPlayingApplicationDisplayName(int null, dispatch_queue_t queue, void (^block)(CFStringRef name));

extern NSString* kMRMediaRemoteNowPlayingApplicationIsPlayingDidChangeNotification;
extern NSString* kMRMediaRemoteNowPlayingInfoDidChangeNotification;
extern NSString* kMRMediaRemoteNowPlayingApplicationDidChangeNotification;

extern NSString* kMRMediaRemoteNowPlayingInfoAlbum;
extern NSString* kMRMediaRemoteNowPlayingInfoArtist;
extern NSString* kMRMediaRemoteNowPlayingInfoTitle;
extern NSString* kMRMediaRemoteNowPlayingInfoArtworkMIMEType;
extern NSString* kMRMediaRemoteNowPlayingInfoArtworkData;
extern NSString* kMRMediaRemoteNowPlayingApplicationDisplayNameUserInfoKey;

@interface media_context : NSObject {}
  @property const char* app;
  @property const char* artist;
  @property const char* title;
  @property const char* album;
  @property CGImageRef artwork;
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
      self.app = NULL;
      self.artist = NULL;
      self.title = NULL;
      self.album = NULL;
    }

    return self;
}

char* g_media_info = NULL;
bool g_media_events = false;
- (void) update {
  @autoreleasepool {
    if (self.app && self.artist && self.title && self.album) {
      char* escaped_artist = escape_string((char*)self.artist);
      char* escaped_title = escape_string((char*)self.title);
      char* escaped_album = escape_string((char*)self.album);

      uint32_t info_len = strlen(self.app)
                          + strlen(escaped_artist)
                          + strlen(escaped_title)
                          + strlen(escaped_album) + 256;

      char info[info_len];
      snprintf(info, info_len, "{\n"
                               "\t\"state\": \"%s\",\n"
                               "\t\"title\": \"%s\",\n"
                               "\t\"album\": \"%s\",\n"
                               "\t\"artist\": \"%s\",\n"
                               "\t\"app\": \"%s\"\n}",
                               self.playing ? "playing" : "paused",
                               escaped_title,
                               escaped_album,
                               escaped_artist,
                               self.app                            );

      free(escaped_artist);
      free(escaped_title);
      free(escaped_album);

      if (self.artwork) {
        struct event cover_event = { self.artwork, COVER_CHANGED };
        event_post(&cover_event);
      }

      if (!g_media_info || strcmp(info, g_media_info) != 0) {
        g_media_info = realloc(g_media_info, info_len);
        memcpy(g_media_info, info, info_len);

        char payload_info[info_len];
        memcpy(payload_info, info, info_len);

        struct event event = { payload_info, MEDIA_CHANGED };
        event_post(&event);
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
  MRMediaRemoteGetNowPlayingApplicationDisplayName(0, dispatch_get_main_queue(), ^(CFStringRef name) {
    @autoreleasepool {
      MRMediaRemoteGetNowPlayingInfo(dispatch_get_main_queue(), ^(NSDictionary* dict) {
        @autoreleasepool {
          if (dict && name) {
            NSString* app = (NSString*)name;
            NSString* artist = [dict objectForKey:kMRMediaRemoteNowPlayingInfoArtist];
            NSString* title = [dict objectForKey:kMRMediaRemoteNowPlayingInfoTitle];
            NSString* album = [dict objectForKey:kMRMediaRemoteNowPlayingInfoAlbum];
            if (artist && title && album && name) {
              self.app = (char*)[app UTF8String];
              self.artist = (char*)[artist UTF8String];
              self.title = (char*)[title UTF8String];
              self.album = (char*)[album UTF8String];

              NSString* mime_type = [dict objectForKey:kMRMediaRemoteNowPlayingInfoArtworkMIMEType];
              NSData* ns_data = [dict objectForKey:kMRMediaRemoteNowPlayingInfoArtworkData];
              CGImageRef image = NULL;
              if (mime_type && ns_data) {
                CFDataRef data = CFDataCreate(NULL,
                                              [ns_data bytes],
                                              [ns_data length]);

                CGDataProviderRef provider
                                        = CGDataProviderCreateWithCFData(data);

                if (provider) {
                  CGImageSourceRef source
                         = CGImageSourceCreateWithDataProvider(provider, NULL);
                  if (source) {
                    image = CGImageSourceCreateImageAtIndex(source, 0, NULL);
                    CFRelease(source);
                  }
                  CFRelease(provider);
                }
                CFRelease(data);
              }
              self.artwork = image;
              [self update];
            }
          }
        }
      });
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

void forced_media_change_event() {
  if (g_media_info) free(g_media_info);
  g_media_info = NULL;
  [g_media_context playing_change:NULL];
}
