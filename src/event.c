#include "event.h"

extern struct event_loop g_event_loop;
extern struct process_manager g_process_manager;
extern struct display_manager g_display_manager;
extern struct bar_manager g_bar_manager;
extern struct application_manager g_application_manager;
extern bool g_mission_control_active;
extern int g_connection;

enum event_type event_type_from_string(const char *str)
{
    for (int i = EVENT_TYPE_UNKNOWN + 1; i < EVENT_TYPE_COUNT; ++i) {
        if (string_equals(str, event_type_str[i])) return i;
    }

    return EVENT_TYPE_UNKNOWN;
}

struct event *event_create(struct event_loop *event_loop, enum event_type type, void *context)
{
    struct event *event = memory_pool_push(&event_loop->pool, struct event);
    event->type = type;
    event->context = context;
    event->param1 = 0;
    event->info = 0;
#ifdef DEBUG
    uint64_t count = __sync_add_and_fetch(&event_loop->count, 1);
    assert(count > 0 && count < EVENT_MAX_COUNT);
#endif
    return event;
}

struct event *event_create_p1(struct event_loop *event_loop, enum event_type type, void *context, int param1)
{
    struct event *event = memory_pool_push(&event_loop->pool, struct event);
    event->type = type;
    event->context = context;
    event->param1 = param1;
    event->info = 0;
#ifdef DEBUG
    uint64_t count = __sync_add_and_fetch(&event_loop->count, 1);
    assert(count > 0 && count < EVENT_MAX_COUNT);
#endif
    return event;
}

void event_destroy(struct event_loop *event_loop, struct event *event)
{
    switch (event->type) {
    default: break;
        case APPLICATION_TERMINATED: {
            process_destroy(event->context);
        } break;
    }

#ifdef DEBUG
    uint64_t count = __sync_sub_and_fetch(&event_loop->count, 1);
    assert(count >= 0 && count < EVENT_MAX_COUNT);
#endif
}


static EVENT_CALLBACK(EVENT_HANDLER_APPLICATION_LAUNCHED)
{
    struct process *process = context;
    debug("%s: %s\n", __FUNCTION__, process->name);

    if ((process->terminated) || (kill(process->pid, 0) == -1)) {
        debug("%s: %s terminated during launch\n", __FUNCTION__, process->name);
        return EVENT_FAILURE;
    }

    struct application *application = application_create(process);
    if (application_observe(application)) {
        application_manager_add_application(&g_application_manager, application);

        return EVENT_SUCCESS;
    } else {
        bool retry_ax = application->retry;
        application_unobserve(application);
        application_destroy(application);
        debug("%s: could not observe %s (%d)\n", __FUNCTION__, process->name, retry_ax);

        if (retry_ax) {
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.01f * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
                struct event *event = event_create(&g_event_loop, APPLICATION_LAUNCHED, process);
                event_loop_post(&g_event_loop, event);
            });
        }

        return EVENT_FAILURE;
    }
}

static EVENT_CALLBACK(EVENT_HANDLER_APPLICATION_TERMINATED)
{
    struct process *process = context;
    struct application *application = application_manager_find_application(&g_application_manager, process->pid);

    if (!application) {
        debug("%s: %s (not observed)\n", __FUNCTION__, process->name);
        return EVENT_FAILURE;
    }

    debug("%s: %s\n", __FUNCTION__, process->name);
    application_manager_remove_application(&g_application_manager, application->pid);

    application_unobserve(application);
    application_destroy(application);

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_APPLICATION_FRONT_SWITCHED)
{
    debug("%s\n", __FUNCTION__);
    bar_manager_refresh(&g_bar_manager);

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_FOCUSED)
{
    debug("%s\n", __FUNCTION__);
    bar_manager_refresh(&g_bar_manager);

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_WINDOW_TITLE_CHANGED)
{
    debug("%s\n", __FUNCTION__);

    // TODO: we can optimize by checking if it the focused window
    bar_manager_refresh(&g_bar_manager);

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_SPACE_CHANGED)
{
    debug("%s\n", __FUNCTION__);

    bar_manager_refresh(&g_bar_manager);

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_CHANGED)
{
    g_display_manager.last_display_id = g_display_manager.current_display_id;
    g_display_manager.current_display_id = display_manager_active_display_id();

    debug("%s: %d\n", __FUNCTION__, g_display_manager.current_display_id);

    bar_manager_refresh(&g_bar_manager);

    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_ADDED)
{
    uint32_t did = (uint32_t)(intptr_t) context;
    debug("%s: %d\n", __FUNCTION__, did);
    bar_manager_display_changed(&g_bar_manager);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_REMOVED)
{
    uint32_t did = (uint32_t)(intptr_t) context;
    debug("%s: %d\n", __FUNCTION__, did);
    bar_manager_display_changed(&g_bar_manager);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_MOVED)
{
    uint32_t did = (uint32_t)(intptr_t) context;
    debug("%s: %d\n", __FUNCTION__, did);
    bar_manager_display_changed(&g_bar_manager);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_DISPLAY_RESIZED)
{
    uint32_t did = (uint32_t)(intptr_t) context;
    debug("%s: %d\n", __FUNCTION__, did);
    bar_manager_display_changed(&g_bar_manager);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_MENU_BAR_HIDDEN_CHANGED)
{
    debug("%s:\n", __FUNCTION__);
    bar_manager_resize(&g_bar_manager);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_SYSTEM_WOKE)
{
    debug("%s:\n", __FUNCTION__);
    bar_manager_refresh(&g_bar_manager);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_BAR_REFRESH)
{
    bar_manager_refresh(&g_bar_manager);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_SHELL_REFRESH)
{
    bar_manager_script_update(&g_bar_manager);
    return EVENT_SUCCESS;
}

static EVENT_CALLBACK(EVENT_HANDLER_DAEMON_MESSAGE)
{
    FILE *rsp = fdopen(param1, "w");
    if (!rsp) goto out;

    if (g_verbose) {
        fprintf(stdout, "%s:", __FUNCTION__);
        for (char *message = context; *message;) {
            message += fprintf(stdout, " %s", message);
        }
        putc('\n', stdout);
        fflush(stdout);
    }

    handle_message(rsp, context);
    fflush(rsp);
    fclose(rsp);

out:
    socket_close(param1);
    free(context);

    return EVENT_SUCCESS;
}
