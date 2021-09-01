extern struct event_loop g_event_loop;

static pascal OSStatus mouse_handler(EventHandlerCallRef next, EventRef e, void *data) {
  switch (GetEventKind(e)) {
    case kEventMouseUp: {
      struct event *event = event_create(&g_event_loop, MOUSE_UP, (void *) CFRetain(CopyEventCGEvent(e)));
      event_loop_post(&g_event_loop, event);
      break;
    }
    default:
      break;
  }

  return CallNextEventHandler(next, e);
}

void mouse_begin(void) {
  EventTargetRef target = GetEventDispatcherTarget();
  InstallEventHandler(target, NewEventHandlerUPP(mouse_handler), sizeof mouse_events / sizeof *mouse_events, mouse_events, 0, 0);
}
