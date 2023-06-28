#pragma once
#include <Cocoa/Cocoa.h>
#include "event.h"
#include "event_loop.h"

@interface workspace_context : NSObject {
}
- (id)init;
- (void)addCustomObserver:(NSString *)name;
@end

void workspace_create_custom_observer (void **context, char* notification);
void workspace_event_handler_init(void **context);
void workspace_event_handler_begin(void **context);
void workspace_event_handler_end(void *context);
int workspace_display_notch_height(uint32_t did);

CGImageRef workspace_icon_for_app(char* app);
