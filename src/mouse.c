extern struct event_loop g_event_loop;

static pascal OSStatus mouse_handler(EventHandlerCallRef next, EventRef e, void *data) {
  switch (GetEventKind(e)) {
    case kEventMouseUp: {
      struct event *event = event_create(&g_event_loop, MOUSE_UP, (void *) CFRetain(CopyEventCGEvent(e)));
      event_loop_post(&g_event_loop, event);
      break;
    }
    case kEventMouseEntered: {
      struct event *event = event_create(&g_event_loop, MOUSE_ENTERED, (void *) CFRetain(CopyEventCGEvent(e)));
      event_loop_post(&g_event_loop, event); 
      break;
    }
    case kEventMouseExited: {
      struct event *event = event_create(&g_event_loop, MOUSE_EXITED, (void *) CFRetain(CopyEventCGEvent(e)));
      event_loop_post(&g_event_loop, event); 
      break;
    }
    default:
      printf("event: %d \n", GetEventKind(e));
      break;
  }

  return CallNextEventHandler(next, e);
}

void mouse_begin(void) {
  EventTargetRef target = GetEventDispatcherTarget();
  InstallEventHandler(target, NewEventHandlerUPP(mouse_handler), GetEventTypeCount(mouse_events), mouse_events, 0, 0);
}
