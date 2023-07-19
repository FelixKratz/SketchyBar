#pragma once

#include <regex.h>
#include "alias.h"
#include "background.h"
#include "bar_item.h"
#include "bar_manager.h"
#include "display.h"
#include "group.h"
#include "slider.h"
#include "mach.h"
#include "event.h"
#include "misc/helpers.h"
#include "misc/defines.h"


MACH_HANDLER(mach_message_handler);
void handle_message_mach(struct mach_buffer* buffer);
