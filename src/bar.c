#include "bar.h"

extern struct event_loop g_event_loop;
//extern struct space_manager g_space_manager;
extern struct bar_manager g_bar_manager;

static POWER_CALLBACK(power_handler)
{
    struct event *event = event_create(&g_event_loop, BAR_REFRESH, NULL);
    event_loop_post(&g_event_loop, event);
}

static TIMER_CALLBACK(timer_handler)
{
    struct event *event = event_create(&g_event_loop, BAR_REFRESH, NULL);
    event_loop_post(&g_event_loop, event);
}

static int bar_find_battery_life(bool *has_battery, bool *charging)
{
    CFTypeRef ps_info = IOPSCopyPowerSourcesInfo();
    CFTypeRef ps_list = IOPSCopyPowerSourcesList(ps_info);

    int ps_count = CFArrayGetCount(ps_list);
    if (!ps_count) return 0;

    int cur_capacity = 0;
    int max_capacity = 0;
    int percent = 0;

    for (int i = 0; i < ps_count; ++i) {
        CFDictionaryRef ps = IOPSGetPowerSourceDescription(ps_info, CFArrayGetValueAtIndex(ps_list, i));
        if (!ps) continue;

        CFTypeRef ps_type = CFDictionaryGetValue(ps, CFSTR(kIOPSTypeKey));
        if (!ps_type || !CFEqual(ps_type, CFSTR(kIOPSInternalBatteryType))) continue;

        CFTypeRef ps_cur = CFDictionaryGetValue(ps, CFSTR(kIOPSCurrentCapacityKey));
        if (!ps_cur) continue;

        CFTypeRef ps_max = CFDictionaryGetValue(ps, CFSTR(kIOPSMaxCapacityKey));
        if (!ps_max) continue;

        CFTypeRef ps_charging = CFDictionaryGetValue(ps, CFSTR(kIOPSPowerSourceStateKey));
        if (!ps_charging) continue;

        CFNumberGetValue((CFNumberRef) ps_cur, kCFNumberSInt32Type, &cur_capacity);
        CFNumberGetValue((CFNumberRef) ps_max, kCFNumberSInt32Type, &max_capacity);
        *charging = !CFEqual(ps_charging, CFSTR(kIOPSBatteryPowerValue));
        *has_battery = true;
        percent = (int)((double) cur_capacity / (double) max_capacity * 100);
        break;
    }

    CFRelease(ps_list);
    CFRelease(ps_info);
    return percent;
}

static CTFontRef bar_create_font(char *cstring)
{
    float size = 10.0f;
    char font_properties[2][255] = { {}, {} };
    sscanf(cstring, "%254[^:]:%254[^:]:%f", font_properties[0], font_properties[1], &size);
    CFStringRef font_family_name = CFStringCreateWithCString(NULL, font_properties[0], kCFStringEncodingUTF8);
    CFStringRef font_style_name = CFStringCreateWithCString(NULL, font_properties[1], kCFStringEncodingUTF8);
    CFNumberRef font_size = CFNumberCreate(NULL, kCFNumberFloat32Type, &size);

    const void *keys[] = { kCTFontFamilyNameAttribute, kCTFontStyleNameAttribute, kCTFontSizeAttribute };
    const void *values[] = { font_family_name, font_style_name, font_size };
    CFDictionaryRef attributes = CFDictionaryCreate(NULL, keys, values, array_count(keys), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CTFontDescriptorRef descriptor = CTFontDescriptorCreateWithAttributes(attributes);
    CTFontRef font = CTFontCreateWithFontDescriptor(descriptor, 0.0, NULL);

    CFRelease(descriptor);
    CFRelease(attributes);
    CFRelease(font_size);
    CFRelease(font_style_name);
    CFRelease(font_family_name);

    return font;
}

static CGPoint bar_align_line(struct bar *bar, struct bar_line line, int align_x, int align_y)
{
    float x = 0, y = 0;

    if (align_x == ALIGN_NONE) {
        x = CGContextGetTextPosition(bar->context).x;
    } else if (align_x == ALIGN_LEFT) {
        x = 20;
    } else if (align_x == ALIGN_CENTER) {
        x = (bar->frame.size.width / 2) - (line.bounds.size.width  / 2);
    } else if (align_x == ALIGN_RIGHT) {
        x = bar->frame.size.width - line.bounds.size.width - 20;
    }

    if (align_y == ALIGN_NONE) {
        y = CGContextGetTextPosition(bar->context).y;
    } else if (align_y == ALIGN_TOP) {
        y = bar->frame.size.height;
    } else if (align_y == ALIGN_CENTER) {
        y = (bar->frame.size.height / 2) - ((line.ascent - line.descent) / 2);
    } else if (align_y == ALIGN_BOTTOM) {
        y = line.descent;
    }

    return (CGPoint) { x, y };
}

static void bar_draw_line(struct bar *bar, struct bar_line line, float x, float y)
{
    CGContextSetRGBFillColor(bar->context, line.color.r, line.color.g, line.color.b, line.color.a);
    CGContextSetTextPosition(bar->context, x, y);
    CTLineDraw(line.line, bar->context);
}

static void bar_destroy_line(struct bar_line line)
{
    CFRelease(line.line);
}

static struct bar_line bar_prepare_line(CTFontRef font, char *cstring, struct rgba_color color)
{
    const void *keys[] = { kCTFontAttributeName, kCTForegroundColorFromContextAttributeName };
    const void *values[] = { font, kCFBooleanTrue };
    CFDictionaryRef attributes = CFDictionaryCreate(NULL, keys, values, array_count(keys), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFStringRef string = CFStringCreateWithCString(NULL, cstring, kCFStringEncodingUTF8);
    CFAttributedStringRef attr_string = CFAttributedStringCreate(NULL, string, attributes);
    CTLineRef line = CTLineCreateWithAttributedString(attr_string);

    CGFloat ascent, descent;
    CTLineGetTypographicBounds(line, &ascent, &descent, NULL);
    CGRect bounds = CTLineGetBoundsWithOptions(line, kCTLineBoundsUseGlyphPathBounds);

    CFRelease(string);
    CFRelease(attributes);
    CFRelease(attr_string);

    return (struct bar_line) {
        .line = line,
        .ascent = ascent,
        .descent = descent,
        .bounds = bounds,
        .color = color
    };
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
static char * focused_window_title()
{
    ProcessSerialNumber psn = {};
    _SLPSGetFrontProcess(&psn);

    pid_t pid;
    GetProcessPID(&psn, &pid);

    AXUIElementRef application_ref = AXUIElementCreateApplication(pid);
    if (!application_ref)
        return NULL;

    CFTypeRef window_ref = NULL;
    AXUIElementCopyAttributeValue(application_ref, kAXFocusedWindowAttribute, &window_ref);
    if (!window_ref) {
        CFRelease(application_ref);
        return NULL;
    }

    char *title = NULL;
    CFTypeRef value = NULL;
    AXUIElementCopyAttributeValue(window_ref, kAXTitleAttribute, &value);
    if (value) {
        title = cfstring_copy(value);
        CFRelease(value);
    }

    CFRelease(window_ref);
    CFRelease(application_ref);

    return title;
}
#pragma clang diagnostic pop

static int mission_control_index(uint64_t sid)
{
    uint64_t result = 0;
    int desktop_cnt = 1;

    CFArrayRef display_spaces_ref = SLSCopyManagedDisplaySpaces(g_connection);
    int display_spaces_count = CFArrayGetCount(display_spaces_ref);

    for (int i = 0; i < display_spaces_count; ++i) {
        CFDictionaryRef display_ref = CFArrayGetValueAtIndex(display_spaces_ref, i);
        CFArrayRef spaces_ref = CFDictionaryGetValue(display_ref, CFSTR("Spaces"));
        int spaces_count = CFArrayGetCount(spaces_ref);

        for (int j = 0; j < spaces_count; ++j) {
            CFDictionaryRef space_ref = CFArrayGetValueAtIndex(spaces_ref, j);
            CFNumberRef sid_ref = CFDictionaryGetValue(space_ref, CFSTR("id64"));
            CFNumberGetValue(sid_ref, CFNumberGetType(sid_ref), &result);
            if (sid == result) goto out;

            ++desktop_cnt;
        }
    }

    desktop_cnt = 0;
    out:
    CFRelease(display_spaces_ref);
    return desktop_cnt;
}

void bar_refresh(struct bar *bar)
{
    SLSDisableUpdate(g_connection);
    SLSOrderWindow(g_connection, bar->id, -1, 0);
    CGContextClearRect(bar->context, bar->frame);
    CGContextSetRGBFillColor(bar->context, g_bar_manager.background_color.r, g_bar_manager.background_color.g, g_bar_manager.background_color.b, g_bar_manager.background_color.a);
    CGContextFillRect(bar->context, bar->frame);
    CGContextStrokePath(bar->context);

    //
    // BAR LEFT
    //

    int final_bar_left_x = 10;
    int space_count;
    uint64_t *space_list = display_space_list(bar->did, &space_count);
    if (space_list) {
        uint64_t sid = display_space_id(bar->did);

        for (int i = 0; i < space_count; ++i) {
            CGPoint pos = CGContextGetTextPosition(bar->context);

            int index = mission_control_index(space_list[i]) - 1;

            struct bar_line space_line = index >= buf_len(g_bar_manager.space_icon_strip)
                                       ? g_bar_manager.space_icon
                                       : g_bar_manager.space_icon_strip[index];
            if (i == 0) {
                pos = bar_align_line(bar, space_line, ALIGN_LEFT, ALIGN_CENTER);
            } else {
                pos.x += 25;
            }

            bar_draw_line(bar, space_line, pos.x, pos.y);

            if (sid == space_list[i]) {
                CGPoint new_pos = CGContextGetTextPosition(bar->context);
                struct bar_line mark_line = g_bar_manager.space_underline;
                CGPoint mark_pos = bar_align_line(bar, mark_line, 0, ALIGN_BOTTOM);
                mark_pos.x = mark_pos.x - mark_line.bounds.size.width / 2 - space_line.bounds.size.width / 2;
                bar_draw_line(bar, mark_line, mark_pos.x, mark_pos.y);
                CGContextSetTextPosition(bar->context, new_pos.x, new_pos.y);
            }

            final_bar_left_x = pos.x + space_line.bounds.size.width + 10;
        }

        free(space_list);
    }

    //
    // BAR RIGHT
    //

    int initial_bar_right_x = bar->frame.size.width - 10;
    time_t rawtime;
    time(&rawtime);
    float time_line_width = 0;
    struct tm *timeinfo = localtime(&rawtime);
    if (timeinfo) {
        char time[255];
        strftime(time, sizeof(time), g_bar_manager._clock_format, timeinfo);
        //snprintf(time, sizeof(time), "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
        struct bar_line time_line = bar_prepare_line(g_bar_manager.t_font, time, g_bar_manager.foreground_color);
        CGPoint t_pos = bar_align_line(bar, time_line, ALIGN_RIGHT, ALIGN_CENTER);
        bar_draw_line(bar, time_line, t_pos.x, t_pos.y);

        CGPoint ti_pos = bar_align_line(bar, g_bar_manager.clock_icon, 0, ALIGN_CENTER);
        ti_pos.x = t_pos.x - g_bar_manager.clock_icon.bounds.size.width - 5;

        CGPoint tu_pos = bar_align_line(bar, g_bar_manager.clock_underline, 0, ALIGN_BOTTOM);
        tu_pos.x = tu_pos.x - g_bar_manager.clock_underline.bounds.size.width / 2 - time_line.bounds.size.width / 2 - (g_bar_manager.clock_icon.bounds.size.width + 5) / 2;

        bar_draw_line(bar, g_bar_manager.clock_icon, ti_pos.x, ti_pos.y);
        bar_draw_line(bar, g_bar_manager.clock_underline, tu_pos.x, tu_pos.y);
        bar_destroy_line(time_line);

        initial_bar_right_x = tu_pos.x - 10;
    }

    bool has_batt = false;
    bool charging = false;
    int percent = bar_find_battery_life(&has_batt, &charging);
    if (has_batt) {
        char batt[255];
        snprintf(batt, sizeof(batt), "%' '3d%%", percent);

        struct bar_line batt_line = bar_prepare_line(g_bar_manager.t_font, batt, g_bar_manager.foreground_color);
        CGPoint p_pos = bar_align_line(bar, batt_line, ALIGN_RIGHT, ALIGN_CENTER);
        p_pos.x = p_pos.x - time_line_width - g_bar_manager.clock_underline.bounds.size.width - 20;
        bar_draw_line(bar, batt_line, p_pos.x, p_pos.y);

        struct bar_line batt_icon = charging ? g_bar_manager.power_icon : g_bar_manager.battr_icon;
        CGPoint pi_pos = bar_align_line(bar, batt_icon, 0, ALIGN_CENTER);
        pi_pos.x = p_pos.x - batt_icon.bounds.size.width - 5;

        CGPoint pu_pos = bar_align_line(bar, g_bar_manager.power_underline, 0, ALIGN_BOTTOM);
        pu_pos.x = pu_pos.x - g_bar_manager.power_underline.bounds.size.width / 2 - batt_line.bounds.size.width / 2 - (batt_icon.bounds.size.width + 5) / 2;

        bar_draw_line(bar, batt_icon, pi_pos.x, pi_pos.y);
        bar_draw_line(bar, g_bar_manager.power_underline, pu_pos.x, pu_pos.y);
        bar_destroy_line(batt_line);

        initial_bar_right_x = pu_pos.x - 10;
    }

    // BAR CENTER
    char *title = focused_window_title();
    if (title) {
        int overlap_left = 0;
        int overlap_right = 0;

        struct bar_line title_line = bar_prepare_line(g_bar_manager.t_font, title, g_bar_manager.foreground_color);
        CGPoint pos = bar_align_line(bar, title_line, ALIGN_CENTER, ALIGN_CENTER);

        if (final_bar_left_x >= pos.x) {
            overlap_left = final_bar_left_x - pos.x;
        }

        assert(overlap_left >= 0);

        if (overlap_left > 0) {
            pos.x = final_bar_left_x;
        }

        if (initial_bar_right_x <= pos.x + title_line.bounds.size.width) {
            overlap_right = pos.x + title_line.bounds.size.width - initial_bar_right_x;
        }

        assert(overlap_right >= 0);

        if (overlap_right > 0) {
            int truncated_width = (int)title_line.bounds.size.width - overlap_right;
            if (truncated_width > 0) {
                CTLineRef truncated_line = CTLineCreateTruncatedLine(title_line.line, truncated_width, kCTLineTruncationEnd, NULL);
                CFRelease(title_line.line);
                title_line.line = truncated_line;
            } else {
                goto free_title;
            }
        }

        bar_draw_line(bar, title_line, pos.x, pos.y);
free_title:
        bar_destroy_line(title_line);
        free(title);
    }

    CGContextFlush(bar->context);
    SLSOrderWindow(g_connection, bar->id, 1, bar->id);
    SLSReenableUpdate(g_connection);
}

static CGPoint bar_create_frame(struct bar *bar, CFTypeRef *frame_region)
{
    CGRect bounds = display_bounds(bar->did);
    CGPoint origin = bounds.origin;

    if (!display_manager_menu_bar_hidden()) {
        CGRect menu = display_manager_menu_bar_rect(bar->did);
        origin.y   += menu.size.height;
    }

    bar->frame = (CGRect) {{0, 0},{bounds.size.width, 26}};
    CGSNewRegionWithRect(&bar->frame, frame_region);

    return origin;
}

void bar_resize(struct bar *bar)
{
    CFTypeRef frame_region;
    CGPoint origin = bar_create_frame(bar, &frame_region);

    SLSDisableUpdate(g_connection);
    SLSOrderWindow(g_connection, bar->id, -1, 0);
    SLSSetWindowShape(g_connection, bar->id, origin.x, origin.y, frame_region);
    bar_refresh(bar);
    SLSOrderWindow(g_connection, bar->id, 1, 0);
    SLSReenableUpdate(g_connection);
    CFRelease(frame_region);
}

struct bar *bar_create(uint32_t did)
{
    struct bar *bar = malloc(sizeof(struct bar));
    memset(bar, 0, sizeof(struct bar));
    bar->did = did;

    uint32_t set_tags[2] = {
        kCGSStickyTagBit |
        kCGSModalWindowTagBit |
        kCGSDisableShadowTagBit |
        kCGSHighQualityResamplingTagBit |
        kCGSIgnoreForExposeTagBit
    };

    uint32_t clear_tags[2] = { 0, 0 };
    *((int8_t *)(clear_tags) + 0x5) = 0x20;

    CFTypeRef frame_region;
    CGPoint origin = bar_create_frame(bar, &frame_region);

    SLSNewWindow(g_connection, 2, origin.x, origin.y, frame_region, &bar->id);
    CFRelease(frame_region);

    SLSSetWindowResolution(g_connection, bar->id, 2.0f);
    SLSSetWindowTags(g_connection, bar->id, set_tags, 64);
    SLSClearWindowTags(g_connection, bar->id, clear_tags, 64);
    SLSSetWindowOpacity(g_connection, bar->id, 0);
    SLSSetMouseEventEnableFlags(g_connection, bar->id, false);
    SLSSetWindowLevel(g_connection, bar->id, CGWindowLevelForKey(4));
    bar->context = SLWindowContextCreate(g_connection, bar->id, 0);

    int refresh_frequency = 5;
    bar->power_source = IOPSNotificationCreateRunLoopSource(power_handler, NULL);
    bar->refresh_timer = CFRunLoopTimerCreate(NULL, CFAbsoluteTimeGetCurrent() + refresh_frequency, refresh_frequency, 0, 0, timer_handler, NULL);

    CFRunLoopAddSource(CFRunLoopGetMain(), bar->power_source, kCFRunLoopCommonModes);
    CFRunLoopAddTimer(CFRunLoopGetMain(), bar->refresh_timer, kCFRunLoopCommonModes);

    bar_refresh(bar);

    return bar;
}

void bar_destroy(struct bar *bar)
{
    CFRunLoopRemoveSource(CFRunLoopGetMain(), bar->power_source, kCFRunLoopCommonModes);
    CFRunLoopSourceInvalidate(bar->power_source);

    CFRunLoopRemoveTimer(CFRunLoopGetMain(), bar->refresh_timer, kCFRunLoopCommonModes);
    CFRunLoopTimerInvalidate(bar->refresh_timer);

    CGContextRelease(bar->context);
    SLSReleaseWindow(g_connection, bar->id);
    free(bar);
}
