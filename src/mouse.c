#include <Carbon/Carbon.h>
#include "mouse.h"

static const EventTypeSpec mouse_events [] = {
    { kEventClassMouse, kEventMouseUp },
    { kEventClassMouse, kEventMouseDragged },
    { kEventClassMouse, kEventMouseEntered },
    { kEventClassMouse, kEventMouseExited },
    { kEventClassMouse, kEventMouseWheelMoved },
    { kEventClassMouse, kEventMouseScroll }
};

static int carbon_event_translation[] = {
  [kEventMouseUp] = MOUSE_UP,
  [kEventMouseDragged] = MOUSE_DRAGGED,
  [kEventMouseEntered] = MOUSE_ENTERED,
  [kEventMouseExited]  = MOUSE_EXITED,
  [kEventMouseWheelMoved] = MOUSE_SCROLLED,
  [kEventMouseScroll] = MOUSE_SCROLLED
};

static pascal OSStatus mouse_handler(EventHandlerCallRef next, EventRef e, void *data) {
  enum event_type event_type = carbon_event_translation[GetEventKind(e)];

  CGEventRef cg_event = CopyEventCGEvent(e);
  struct event event = { (void *) cg_event, event_type };
  event_post(&event);
  CFRelease(cg_event);

  return CallNextEventHandler(next, e);
}

void mouse_begin(void) {
  InstallEventHandler(GetEventDispatcherTarget(),
                      NewEventHandlerUPP(mouse_handler),
                      GetEventTypeCount(mouse_events),
                      mouse_events, 0, 0);
}
