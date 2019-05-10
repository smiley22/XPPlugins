/**
 * ToggleMouseLook - X-Plane 11 Plugin
 *
 * Adds two new commands that mimic the mouse look behaviour of Prepar3D.
 *
 * Copyright 2019 Torben Könke.
 */
#include "plugin.h"

#define PLUGIN_NAME         "ToggleMouseLook"
#define PLUGIN_SIG          "S22.ToggleMouseLook"
#define PLUGIN_DESCRIPTION  "Adds new commands for better control of mouse " \
                            "look inside the cockpit."
#define PLUGIN_VERSION      "1.2"

static XPLMCommandRef toggle_mouse_look;
static XPLMCommandRef hold_mouse_look;
static int mouse_look;
static int screen_height;
static float magenta[] = { 1.0f, 0, 1.0f };

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
    toggle_mouse_look = cmd_create("MouseLook/Toggle",
        "Toggle mouse-look on or off", toggle_cb, NULL);
    hold_mouse_look = cmd_create("MouseLook/Hold", "Hold key to look around",
        hold_cb, NULL);
    XPLMRegisterDrawCallback(draw_cb, xplm_Phase_Window, 0, NULL);
#ifdef IBM
    if (!hook_wnd_proc()) {
        _log("could not hook wnd proc");
    }
#elif APL
    if (!tap_events()) {
        _log("could not tap events");
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
    cmd_free(toggle_mouse_look, toggle_cb, NULL);
    cmd_free(hold_mouse_look, hold_cb, NULL);
    XPLMUnregisterDrawCallback(draw_cb, xplm_Phase_Window, 0, NULL);
#ifdef IBM
    unhook_wnd_proc();
#elif APL
    untap_events();
#endif
}

/**
* X-Plane 11 Plugin Callback
*
* Called when a message is sent to the plugin by X-Plane 11 or another plugin.
*/
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID from, int msg, void *param) {
}

void right_click() {
#ifdef IBM
    mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
#elif APL
    mouse_event(kCGEventRightMouseDown);
    mouse_event(kCGEventRightMouseUp);
#endif
}

int toggle_cb(XPLMCommandRef cmd, XPLMCommandPhase phase, void *data) {
    if (phase == xplm_CommandBegin) {
        right_click();
    }
    return 0;
}

int hold_cb(XPLMCommandRef cmd, XPLMCommandPhase phase, void *data) {
    switch (phase) {
    case xplm_CommandBegin:
    case xplm_CommandEnd:
        right_click();
        break;
    }
    return 0;
}

int draw_cb(XPLMDrawingPhase phase, int before, void *ref) {
    if (mouse_look) {
        XPLMDrawString(magenta, 20, screen_height - 20, "MOUSELOOK", NULL,
            xplmFont_Proportional);
    }
    return 1;
}

#ifdef IBM
static HWND xp_hwnd;
static WNDPROC old_wnd_proc;

LRESULT CALLBACK xp_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam,
    LPARAM lParam) {
    switch (msg) {
    case WM_RBUTTONDOWN:
        mouse_look = !mouse_look;
        if (mouse_look)
            XPLMGetScreenSize(NULL, &screen_height);
        else
            return 0;
        break;
    case WM_RBUTTONUP:
        if (mouse_look)
            return 0;
        break;
    }
    return CallWindowProcA(old_wnd_proc, hwnd, msg, wParam, lParam);
}

int hook_wnd_proc() {
    xp_hwnd = FindWindowA("X-System", "X-System");
    if (!xp_hwnd) {
        _log("could not find X-Plane 11 window");
        return 0;
    }
    old_wnd_proc = (WNDPROC) SetWindowLongPtrA(xp_hwnd, GWLP_WNDPROC,
        (LONG_PTR) &xp_wnd_proc);
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
#elif APL
static CFMachPortRef event_tap;
static CFRunLoopSourceRef loop_src;

CGEventRef cg_event_cb(CGEventTapProxy proxy, CGEventType type,
    CGEventRef ev, void *data) {
    switch (type) {
    case kCGEventRightMouseDown:
        mouse_look = !mouse_look;
        if (mouse_look)
            XPLMGetScreenSize(NULL, &screen_height);
        else
            return NULL;
        break;
    case kCGEventRightMouseUp:
        if (mouse_look)
            return NULL;
        break;
    }
    return ev;
}

int tap_events() {
    ProcessSerialNumber psn;
    OSErr err = GetCurrentProcess(&psn);
    if (err != noErr) {
      _log("could not get psn (%i)", err);
      return 0;
    }
    /* CGEventTapCreateForPid has only been added with 10.11 */
    event_tap = CGEventTapCreateForPSN(&psn, kCGHeadInsertEventTap,
        kCGEventTapOptionDefault, CGEventMaskBit(kCGEventRightMouseDown) |
        CGEventMaskBit(kCGEventRightMouseUp), cg_event_cb, NULL);
    if (!event_tap) {
        _log("could not create event tap");
        return 0;
    }
    loop_src =  CFMachPortCreateRunLoopSource(NULL, event_tap, 0);
    if (!loop_src) {
        _log("could not create run-loop source");
        CFRelease(event_tap);
        return 0;
    }
    CFRunLoopAddSource(CFRunLoopGetCurrent(), loop_src,
        kCFRunLoopCommonModes);
    CGEventTapEnable(event_tap, true);
    return 1;
}

int untap_events() {
    if (!event_tap)
        return 1;
    CFRunLoopRemoveSource(CFRunLoopGetCurrent(), loop_src,
        kCFRunLoopCommonModes);
    CGEventTapEnable(event_tap, false);
    /* Releasing the mach port also releases the tap. */
    CFRelease(event_tap);
    event_tap = NULL;
    return 1;
}

void mouse_event(CGEventType type) {
    CGEventRef ev = CGEventCreate(NULL);
    /* This gives us the current mouse position. */
    CGPoint pos = CGEventGetLocation(ev);
    CFRelease(ev);
    ev = CGEventCreateMouseEvent(NULL, type, pos, kCGMouseButtonRight);
    CGEventPost(kCGHIDEventTap, ev);
    CFRelease(ev);
}
#endif
