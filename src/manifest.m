#include <Carbon/Carbon.h>
#include <Cocoa/Cocoa.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <pthread.h>

#include "misc/timing.h"
#include "misc/log.h"
#include "misc/env_vars.h"
#include "misc/helpers.h"
#include "misc/memory_pool.h"
#include "misc/socket.h"
#include "misc/socket.c"

#include "event_loop.h"
#include "mouse.h"
#include "event.h"
#include "workspace.h"
#include "message.h"
#include "display.h"
#include "background.h"
#include "bar.h"
#include "text.h"
#include "graph.h"
#include "alias.h"
#include "group.h"
#include "bar_item.h"
#include "custom_events.h"
#include "bar_manager.h"

#include "event_loop.c"
#include "mouse.c"
#include "event.c"
#include "workspace.m"
#include "message.c"
#include "display.c"
#include "background.c"
#include "bar.c"
#include "text.c"
#include "graph.c"
#include "alias.c"
#include "group.c"
#include "bar_item.c"
#include "custom_events.c"
#include "bar_manager.c"

#include "sketchybar.c"
