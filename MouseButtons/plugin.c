/**
 * MouseButtons - X-Plane 11 Plugin
 *
 * Enables the use of extra mouse buttons and allows the right mouse button
 * and mouse wheel to be re-assigned to arbitrary commands.
 *
 * Copyright 2019 Torben Könke.
 */
#include "plugin.h"

#define PLUGIN_NAME         "MouseButtons"
#define PLUGIN_SIG          "S22.MouseButtons"
#define PLUGIN_DESCRIPTION  "Enables the use of extra mouse buttons and allows " \
                            "the right mouse button and mouse wheel to be re-"   \
                            "assigned to arbitrary commands."
#define PLUGIN_VERSION      "1.0"

 /**
 * X-Plane 11 Plugin Entry Point.
 *
 * Called when a plugin is initially loaded into X-Plane 11. If 0 is returned,
 * the plugin will be unloaded immediately with no further calls to any of
 * its callbacks.
 */
PLUGIN_API int XPluginStart(char *name, char *sig, char *desc) {
    /* SDK docs state buffers are at least 256 bytes. */
    sprintf(name, "%s (v%s)", PLUGIN_NAME, PLUGIN_VERSION);
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
#ifdef IBM
    if (!hook_wnd_proc()) {
        _log("could not hook wnd proc");
    }
#endif
    return 1;
}

/**
* X-Plane 11 Plugin Callback
*
* Called when the plugin is about to be disabled.
*/
PLUGIN_API void XPluginDisable(void) {
#ifdef IBM
    unhook_wnd_proc();
#endif
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
        int index = (int)param;
        /* user's plane */
        if (index == 0) {
            /* We cannot call this from XPluginEnable because at that point
            XPLMGetNthAircraftModel won't return any paths yet...*/
            int num_bindings = bindings_init();
            _log("loaded %i mouse bindings", num_bindings);
        }
    }
}

#ifdef IBM
static HWND xp_hwnd;
static WNDPROC old_wnd_proc;

static mbutton_t wm_to_mbutton(UINT msg, WPARAM wParam, LPARAM lParam) {
    return M_NONE;
}

LRESULT CALLBACK xp_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam,
    LPARAM lParam) {
    XPLMCommandRef cmd = bindings_get(
        wm_to_mbutton(msg, wParam, lParam),
        0
    );
    if (cmd) {
        switch (msg) {
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_XBUTTONDOWN:
            XPLMCommandBegin(cmd);
            break;
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_XBUTTONUP:
            XPLMCommandEnd(cmd);
            break;
        }
    }
    return CallWindowProcA(old_wnd_proc, hwnd, msg, wParam, lParam);
}

int hook_wnd_proc() {
    xp_hwnd = FindWindowA("X-System", "X-System");
    if (!xp_hwnd) {
        _log("could not find X-Plane 11 window");
        return 0;
    }
    old_wnd_proc = (WNDPROC)SetWindowLongPtrA(xp_hwnd, GWLP_WNDPROC,
        (LONG_PTR)&xp_wnd_proc);
    if (!old_wnd_proc) {
        _log("could not set window procedure (%i)", GetLastError());
        return 0;
    }
    return 1;
}

int unhook_wnd_proc() {
    if (!xp_hwnd || !old_wnd_proc)
        return 0;
    if (!SetWindowLongPtrA(xp_hwnd, GWLP_WNDPROC,
        (LONG_PTR)old_wnd_proc)) {
        _log("could not restore window procedure (%i)", GetLastError());
        return 0;
    }
    return 1;
}
#endif
