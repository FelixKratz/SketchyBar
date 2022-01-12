#ifndef WORKSPACE_H
#define WORKSPACE_H

struct notification {
  char* name;
  char* info;
};

struct notification* notification_create();
void notification_destroy(struct notification* notification);

@interface workspace_context : NSObject {
}
- (id)init;
- (void)addCustomObserver:(NSString *)name;
@end

void workspace_create_custom_observer (void **context, char* notification);
void workspace_event_handler_init(void **context);
void workspace_event_handler_begin(void **context);
void workspace_event_handler_end(void *context);

#endif
