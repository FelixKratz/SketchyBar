#pragma once
#include "event.h"

static const CGEventMask g_mouse_events
                                     = CGEventMaskBit(kCGEventLeftMouseUp)
                                     | CGEventMaskBit(kCGEventRightMouseUp)
                                     | CGEventMaskBit(kCGEventOtherMouseUp)
                                     | CGEventMaskBit(kCGEventLeftMouseDragged)
                                     | CGEventMaskBit(kCGEventScrollWheel);
      
bool mouse_handle_event(CGEventType type, CGEventRef cg_event);
