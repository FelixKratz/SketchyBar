#ifndef EVENT_TAP_H
#define EVENT_TAP_H

#define EVENT_MASK_MOUSE (1 << kCGEventLeftMouseUp) | \
                         (1 << kCGEventRightMouseUp)

#define EVENT_MASK_ALT   kCGEventFlagMaskAlternate
#define EVENT_MASK_SHIFT kCGEventFlagMaskShift
#define EVENT_MASK_CMD   kCGEventFlagMaskCommand
#define EVENT_MASK_CTRL  kCGEventFlagMaskControl
#define EVENT_MASK_FN    kCGEventFlagMaskSecondaryFn

struct event_tap
{
    CFMachPortRef handle;
    CFRunLoopSourceRef runloop_source;
    CGEventMask mask;
};

#define EVENT_TAP_CALLBACK(name) \
    CGEventRef name(CGEventTapProxy proxy, \
                    CGEventType type, \
                    CGEventRef cgevent, \
                    void *reference)
typedef EVENT_TAP_CALLBACK(event_tap_callback);

static EVENT_TAP_CALLBACK(mouse_handler);
bool event_tap_enabled(struct event_tap *event_tap);
bool event_tap_begin(struct event_tap *event_tap, uint32_t mask, event_tap_callback *callback);
void event_tap_end(struct event_tap *event_tap);

#endif
