#include "mouse.h"

bool mouse_handle_event(CGEventType type, CGEventRef cg_event) {
  if (type == kCGEventOtherMouseUp
      || type == kCGEventLeftMouseUp
      || type == kCGEventRightMouseUp) {
    struct event event = { (void *) cg_event, MOUSE_UP };
    event_post(&event);
  } else if (type == kCGEventLeftMouseDragged) {
    struct event event = { (void *) cg_event, MOUSE_DRAGGED };
    event_post(&event);
  } else if (type == 0x8) {
    struct event event = { (void *) cg_event, MOUSE_ENTERED };
    event_post(&event); 
  } else if (type == 0x9) {
    struct event event = { (void *) cg_event, MOUSE_EXITED };
    event_post(&event); 
  } else if (type == kCGEventScrollWheel) {
    struct event event = { (void *) cg_event, MOUSE_SCROLLED };
    event_post(&event);
  } else return false;
  return true;
}
