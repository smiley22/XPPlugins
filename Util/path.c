/**
 * Utility library for X-Plane 11 Plugins.
 *
 * Static library containing common functionality for stuff like logging and
 * dealing with configuration files. Linked against by most plugins in the
 * solution.
 *
 * Copyright 2019 Torben K�nke.
 */
#include "util.h"

int get_plugin_dir(char *buf, int size) {
    XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);
    XPLMGetPluginInfo(XPLMGetMyID(), NULL, buf, NULL, NULL);
    /* skip plugin filename */
    char *p = strrchr(buf, '/');
    if (!p)
        return 0;
    *p = 0;
    /* skip /64 directory */
    p = strrchr(buf, '/');
    if (!p)
        return 0;
    *(p + 1) = 0;
    return 1;
}

int get_plugin_name(char *buf, int size) {
    /* Unfortunately this returns a garbage name in the form of
       plugin/64/win.xpl */
    XPLMGetPluginInfo(XPLMGetMyID(), buf, NULL, NULL, NULL);
    /* skip plugin filename */
    char *p = strrchr(buf, '/');
    if (!p)
        return 0;
    *p = 0;
    /* skip /64 directory */
    p = strrchr(buf, '/');
    if (!p)
        return 0;
    *p = 0;
    return 1;
}

int get_acft_dir(char *buf, int size) {

}

int get_data_path(const char *file, char *buf, int size) {
    if (!get_plugin_dir(buf, size))
        return 0;
    strncat(buf, "data/", size);
    strncat(buf, file, size);
    return 1;
}