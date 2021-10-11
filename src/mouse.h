static const EventTypeSpec mouse_events [] = {
    { kEventClassMouse, kEventMouseDown },
    { kEventClassMouse, kEventMouseUp },
    { kEventClassMouse, kEventMouseEntered },
    { kEventClassMouse, kEventMouseExited }
};

void mouse_begin(void);
