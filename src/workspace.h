#pragma once
#include <Cocoa/Cocoa.h>
#include "event.h"
#include "event_loop.h"

@interface workspace_context : NSObject {
}
- (id)init;
- (void)addCustomObserver:(NSString *)name;
@end

void *g_workspace_context;
void workspace_create_custom_observer (void **context, char* notification);
void workspace_event_handler_init(void **context);
void workspace_event_handler_begin(void **context);
void workspace_event_handler_end(void *context);
