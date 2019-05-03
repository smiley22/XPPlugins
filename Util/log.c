#include "util.h"

static int debug_enabled = -1;
static char plugin_name[MAX_NAME];

static const char *get_plugin_name() {
    if (plugin_name[0])
        return plugin_name;
    XPLMGetPluginInfo(XPLMGetMyID(), plugin_name, NULL, NULL, NULL);
    return plugin_name;
}

void _log(const char *fmt, ...) {
    char buf[2048], out[2048];
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    sprintf(out, "[%s]: %s\r\n", get_plugin_name(), buf);
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
    sprintf(out, "[%s] (debug): %s\r\n", get_plugin_name(), buf);
    XPLMDebugString(out);
    va_end(args);
}