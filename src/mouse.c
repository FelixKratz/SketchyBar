#include <Carbon/Carbon.h>
#include "mouse.h"

static const EventTypeSpec mouse_events [] = {
    { kEventClassMouse, kEventMouseDown },
    { kEventClassMouse, kEventMouseUp },
    { kEventClassMouse, kEventMouseDragged },
    { kEventClassMouse, kEventMouseEntered },
    { kEventClassMouse, kEventMouseExited },
    { kEventClassMouse, kEventMouseWheelMoved }
};


static pascal OSStatus mouse_handler(EventHandlerCallRef next, EventRef e, void *data) {
  switch (GetEventKind(e)) {
    case kEventMouseUp: {
      struct event event = { (void *) CopyEventCGEvent(e),
                              0,
                              MOUSE_UP                    };

      event_post(&event);
      break;
    }
    case kEventMouseDragged: {
      CGEventRef cg_event = CopyEventCGEvent(e);
      struct event event = { (void *) cg_event,
                              0,
                              MOUSE_DRAGGED    };

      event_post(&event);
      break;
    }
    case kEventMouseEntered: {
      CGEventRef cg_event = CopyEventCGEvent(e);
      struct event event = { (void *) cg_event,
                              0,
                              MOUSE_ENTERED    };

      event_post(&event); 
      break;
    }
    case kEventMouseExited: {
      CGEventRef cg_event = CopyEventCGEvent(e);
      struct event event = { (void *) cg_event,
                              0,
                              MOUSE_EXITED     };

      event_post(&event); 
      break;
    }
    case kEventMouseWheelMoved: {
      CGEventRef cg_event = CopyEventCGEvent(e);
      struct event event = { (void *) cg_event,
                              0,
                              MOUSE_SCROLLED   };

      event_post(&event);
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
