#include <Carbon/Carbon.h>
#include "mouse.h"

extern struct event_loop g_event_loop;

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
      struct event *event = event_create(&g_event_loop,
                                         MOUSE_UP,
                                         (void *) CFRetain(CopyEventCGEvent(e)));

      event_loop_post(&g_event_loop, event);
      break;
    }
    case kEventMouseDragged: {
      struct event *event = event_create(&g_event_loop,
                                         MOUSE_DRAGGED,
                                         (void *) CFRetain(CopyEventCGEvent(e)));

      event_loop_post(&g_event_loop, event);
      break;
    }
    case kEventMouseEntered: {
      struct event *event = event_create(&g_event_loop,
                                         MOUSE_ENTERED,
                                         (void *) CFRetain(CopyEventCGEvent(e)));

      event_loop_post(&g_event_loop, event); 
      break;
    }
    case kEventMouseExited: {
      struct event *event = event_create(&g_event_loop,
                                         MOUSE_EXITED,
                                         (void *) CFRetain(CopyEventCGEvent(e)));
      event_loop_post(&g_event_loop, event); 
      break;
    }
    case kEventMouseWheelMoved: {
      struct event *event = event_create(&g_event_loop,
                                         MOUSE_SCROLLED,
                                         (void *) CFRetain(CopyEventCGEvent(e)));
      event_loop_post(&g_event_loop, event);
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
