#pragma once
#include <Carbon/Carbon.h>
#include "event_loop.h"
#include "event.h"
static const EventTypeSpec mouse_events [] = {
    { kEventClassMouse, kEventMouseDown },
    { kEventClassMouse, kEventMouseUp },
    { kEventClassMouse, kEventMouseDragged },
    { kEventClassMouse, kEventMouseEntered },
    { kEventClassMouse, kEventMouseExited }
};

void mouse_begin(void);
