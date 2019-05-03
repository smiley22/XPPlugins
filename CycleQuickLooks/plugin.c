/*
 * CycleQuickLooks - X-Plane 11 Plugin
 *
 * Adds two new commands for cycling through a plane's configured quick
 * looks.
 *
 * Copyright 2019 Torben Könke.
 */
#include "../Util/util.h"
#include "../XP/XPLMPlanes.h"
#include <stdlib.h>

#define PLUGIN_NAME         "Cycle Quick Looks"
#define PLUGIN_SIG          "S22.CycleQuickLooks"
#define PLUGIN_DESCRIPTION  "Adds two new commands for cycling through a plane's " \
                            "configured quick looks."

#define MAX_QUICK_LOOKS     20
static XPLMCommandRef cycle_forward;
static XPLMCommandRef cycle_backward;
static int quick_looks[MAX_QUICK_LOOKS];
static int num_quick_looks;
static int current;

int cycle_quick_look_cb(XPLMCommandRef cmd, XPLMCommandPhase phase, void *ref);
int get_quick_looks(int *buf, int buf_size);

/*
* X-Plane 11 Plugin Entry Point.
*
* Called when a plugin is initially loaded into X-Plane 11. If 0 is returned,
* the plugin will be unloaded immediately with no further calls to any of
* its callbacks.
*/
PLUGIN_API int XPluginStart(char *name, char *sig, char *desc) {
    strcpy(name, PLUGIN_NAME);
    strcpy(sig, PLUGIN_SIG);
    strcpy(desc, PLUGIN_DESCRIPTION);

    return 1;
}

/**
* X-Plane 11 Plugin Callback
*
* Called when the plugin is about to be unloaded from X-Plane 11.
*/
PLUGIN_API void XPluginStop(void) {
    /* nothing to do here */
}

/**
* X-Plane 11 Plugin Callback
*
* Called when the plugin is about to be enabled. Return 1 if the plugin
* started successfully, otherwise 0.
*/
PLUGIN_API int XPluginEnable(void) {
    cycle_forward = cmd_create("CycleQuickLooks/Forward",
        "Cycle forward to next quick look", cycle_quick_look_cb, 0);
    cycle_backward = cmd_create("CycleQuickLooks/Backward",
        "Cycle backward to previous quick look", cycle_quick_look_cb, (void*)1);
    return 1;
}

/**
* X-Plane 11 Plugin Callback
*
* Called when the plugin is about to be disabled.
*/
PLUGIN_API void XPluginDisable(void) {
    cmd_free(cycle_forward);
    cmd_free(cycle_backward);
}

/**
* X-Plane 11 Plugin Callback
*
* Called when a message is sent to the plugin by X-Plane 11 or another plugin.
*/
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID from, int msg, void *param) {
    if (from != XPLM_PLUGIN_XPLANE)
        return;
    if (msg == XPLM_MSG_PLANE_LOADED) {
        int index = (int) param;
        /* user's plane */
        if (index == 0) {
            /* we cannot call this from XPluginEnable because at that point
               XPLMGetNthAircraftModel won't return any paths yet...*/
            num_quick_looks = get_quick_looks(quick_looks, MAX_QUICK_LOOKS);
            _log("loaded %i quick looks", num_quick_looks);
        }
    }
}

/*
* Gets the list of quick-looks configured for the current plane.
*/
int get_quick_looks(int *buf, int buf_size) {
    char name[256], path[512];
    XPLMGetNthAircraftModel(0, name, path);
    /* overwrite .acf extension to get path for _prefs file */
    char *p = strrchr(path, '.');
    if (!p) {
        _log("unexpected aircraft path: %s", path);
        return 0;
    }
    strcpy(p, "_prefs.txt");
    FILE *fp = fopen(path, "r");
    if (!fp) {
        _log("couldn't open file '%s' for reading", path);
        return 0;
    }
    char line[256];
    const char *s = "_iql_view_type_";
    size_t len = strlen(s);
    int num = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, s, len))
            continue;
        *(buf + num++) = atoi(line + len);
        if (num >= buf_size)
            break;
    }
    fclose(fp);
    return num;
}

int cycle_quick_look_cb(XPLMCommandRef cmd, XPLMCommandPhase phase, void *ref) {
    if (phase != xplm_CommandBegin)
        return 1;
    if (ref) {
        /* cycle backward */
        if (--current < 0)
            current = num_quick_looks - 1;
    } else {
        /* cycle forward */
        if (++current == num_quick_looks)
            current = 0;
    }
    char buf[128];
    snprintf(buf, sizeof(buf), "sim/view/quick_look_%i",
        quick_looks[current]);
    _debug("exec '%s'", buf);
    XPLMCommandRef cmd_ref = XPLMCreateCommand(buf, "");
    if (cmd_ref)
        XPLMCommandOnce(cmd_ref);
    return 1;
}
