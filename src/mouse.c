#include <Carbon/Carbon.h>
#include "mouse.h"

static const EventTypeSpec mouse_events [] = {
    { kEventClassMouse, kEventMouseDown },
    { kEventClassMouse, kEventMouseUp },
    { kEventClassMouse, kEventMouseDragged },
    { kEventClassMouse, kEventMouseEntered },
    { kEventClassMouse, kEventMouseExited },
    { kEventClassMouse, kEventMouseWheelMoved },
    { kEventClassMouse, kEventMouseScroll }
};


static pascal OSStatus mouse_handler(EventHandlerCallRef next, EventRef e, void *data) {
  switch (GetEventKind(e)) {
    case kEventMouseUp: {
      CGEventRef cg_event = CopyEventCGEvent(e);
      struct event event = { (void *) cg_event, MOUSE_UP };

      event_post(&event);
      CFRelease(cg_event);
      break;
    }
    case kEventMouseDragged: {
      CGEventRef cg_event = CopyEventCGEvent(e);
      struct event event = { (void *) cg_event, MOUSE_DRAGGED };

      event_post(&event);
      CFRelease(cg_event);
      break;
    }
    case kEventMouseEntered: {
      CGEventRef cg_event = CopyEventCGEvent(e);
      struct event event = { (void *) cg_event, MOUSE_ENTERED };

      event_post(&event); 
      CFRelease(cg_event);
      break;
    }
    case kEventMouseExited: {
      CGEventRef cg_event = CopyEventCGEvent(e);
      struct event event = { (void *) cg_event, MOUSE_EXITED };

      event_post(&event); 
      CFRelease(cg_event);
      break;
    }
    case kEventMouseScroll:
    case kEventMouseWheelMoved: {
      CGEventRef cg_event = CopyEventCGEvent(e);
      struct event event = { (void *) cg_event, MOUSE_SCROLLED };

      event_post(&event);
      CFRelease(cg_event);
      break;
    }
    default:
      break;
  }

  return CallNextEventHandler(next, e);
}

void mouse_begin(void) {
  InstallEventHandler(GetEventDispatcherTarget(),
                      NewEventHandlerUPP(mouse_handler),
                      GetEventTypeCount(mouse_events),
                      mouse_events, 0, 0);
}
