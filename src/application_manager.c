#include "application_manager.h"

extern struct process_manager g_process_manager;
extern struct mouse_state g_mouse_state;
extern char g_sa_socket_file[MAXLEN];

static TABLE_HASH_FUNC(hash_application)
{
    unsigned long result = *(uint32_t *) key;
    result = (result + 0x7ed55d16) + (result << 12);
    result = (result ^ 0xc761c23c) ^ (result >> 19);
    result = (result + 0x165667b1) + (result << 5);
    result = (result + 0xd3a2646c) ^ (result << 9);
    result = (result + 0xfd7046c5) + (result << 3);
    result = (result ^ 0xb55a4f09) ^ (result >> 16);
    return result;
}

static TABLE_COMPARE_FUNC(compare_application)
{
    return *(uint32_t *) key_a == *(uint32_t *) key_b;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
struct application *application_manager_focused_application(struct application_manager *application_manager)
{
    ProcessSerialNumber psn = {};
    _SLPSGetFrontProcess(&psn);

    pid_t pid;
    GetProcessPID(&psn, &pid);

    return application_manager_find_application(application_manager, pid);
}
#pragma clang diagnostic pop

struct application *application_manager_find_application(struct application_manager *application_manager, pid_t pid)
{
    return table_find(&application_manager->application, &pid);
}

void application_manager_remove_application(struct application_manager *application_manager, pid_t pid)
{
    table_remove(&application_manager->application, &pid);
}

void application_manager_add_application(struct application_manager *application_manager, struct application *application)
{
    table_add(&application_manager->application, &application->pid, application);
}

void application_manager_init(struct application_manager *application_manager)
{
    application_manager->system_element = AXUIElementCreateSystemWide();
    AXUIElementSetMessagingTimeout(application_manager->system_element, 1.0);

    table_init(&application_manager->application, 150, hash_application, compare_application);
}

void application_manager_begin(struct application_manager *application_manager)
{
    for (int process_index = 0; process_index < g_process_manager.process.capacity; ++process_index) {
        struct bucket *bucket = g_process_manager.process.buckets[process_index];
        while (bucket) {
            if (bucket->value) {
                struct process *process = bucket->value;
                struct application *application = application_create(process);

                if (application_observe(application)) {
                    application_manager_add_application(application_manager, application);
                } else {
                    application_unobserve(application);
                    application_destroy(application);
                }
            }

            bucket = bucket->next;
        }
    }
}
