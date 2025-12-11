#pragma once
#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdbool.h>

// In macOS 26 (Tahoe), menu bar item windows are owned by Control Center
// instead of their source applications. This module provides functions to
// find the actual source PID using the Accessibility API.

// Check if we're running on macOS 26 or later where this workaround is needed
bool source_pid_needs_workaround(void);

// Get the source PID for a menu bar item window by matching its bounds
// to accessibility elements in running applications' extras menu bars.
// Returns 0 if the source PID cannot be determined.
pid_t source_pid_for_window(CGRect window_bounds);

// Get the source application name for a menu bar item window.
// Returns NULL if not found. Caller must free the returned string.
char* source_name_for_window(CGRect window_bounds);

// Initialize the source PID cache (call once at startup)
void source_pid_cache_init(void);

// Refresh the cached running applications list
void source_pid_cache_refresh(void);
