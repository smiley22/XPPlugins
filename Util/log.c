/**
 * Utility library for X-Plane 11 Plugins.
 *
 * Static library containing common functionality for stuff like logging and
 * dealing with configuration files. Linked against by most plugins in the
 * solution.
 *
 * Copyright 2019 Torben Könke.
 */
#include "util.h"

static int debug_enabled = -1;
static char plugin_name[MAX_NAME];

static const char *get_name() {
    if (plugin_name[0])
        return plugin_name;
    get_plugin_name(plugin_name, MAX_NAME);
    return plugin_name;
}

void _log(const char *fmt, ...) {
    char buf[2048], out[2048];
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    sprintf(out, "[%s]: %s\r\n", get_name(), buf);
    XPLMDebugString(out);
    va_end(args);
}

void _debug(const char *fmt, ...) {
    if (debug_enabled < 0)
        debug_enabled = ini_geti("debug", 0);
    if (!debug_enabled)
        return;
    char buf[2048], out[2048];
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    sprintf(out, "[%s] (debug): %s\r\n", get_name(), buf);
    XPLMDebugString(out);
    va_end(args);
}
