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
    if (from != XPLM_PLUGIN_XPLANE)
        return;
    if (msg == XPLM_MSG_PLANE_LOADED) {
        int index = (int)param;
        /* user's plane */
        if (index == XPLM_USER_AIRCRAFT) {
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

static mbutton_t wm_to_mbutton(UINT msg, WPARAM wParam, int *state) {
    switch (msg) {
    case WM_LBUTTONDOWN:
        *state = M_STATE_DOWN;
        return M_LEFT;
    case WM_LBUTTONUP:
        *state = M_STATE_UP;
        return M_LEFT;
    case WM_RBUTTONDOWN:
        *state = M_STATE_DOWN;
        return M_RIGHT;
    case WM_RBUTTONUP:
        *state = M_STATE_UP;
        return M_RIGHT;
    case WM_MBUTTONDOWN:
        *state = M_STATE_DOWN;
        return M_MIDDLE;
    case WM_MBUTTONUP:
        *state = M_STATE_UP;
        return M_MIDDLE;
    case WM_XBUTTONDOWN:
        *state = M_STATE_DOWN;
        return (HIWORD(wParam) & XBUTTON1) ? M_FORWARD : M_BACKWARD;
    case WM_XBUTTONUP:
        *state = M_STATE_UP;
        return (HIWORD(wParam) & XBUTTON1) ? M_FORWARD : M_BACKWARD;
    case WM_MOUSEWHEEL:
        *state = M_STATE_DOWN | M_STATE_UP;
        return GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? M_W_FORWARD :
            M_W_BACKWARD;
    case WM_MOUSEHWHEEL:
        *state = M_STATE_DOWN | M_STATE_UP;
        return GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? M_W_RIGHT : M_W_LEFT;
    default:
        return M_NONE;
    }
}

LRESULT CALLBACK xp_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam,
    LPARAM lParam) {
    int state;
    mbutton_t mbutton = wm_to_mbutton(msg, wParam, &state);
    if (mbutton != M_NONE) {
        int mod = 0;
        if (wParam & MK_CONTROL)
            mod |= M_MOD_CTRL;
        if (wParam & MK_SHIFT)
            mod |= M_MOD_SHIFT;
        if ((wParam & MK_LBUTTON) && (mbutton != M_LEFT))
            mod |= M_MOD_LMB;
        if ((wParam & MK_RBUTTON) && (mbutton != M_RIGHT))
            mod |= M_MOD_RMB;
        if ((wParam  & MK_MBUTTON) && (mbutton != M_MIDDLE))
            mod |= M_MOD_MMB;
        if ((wParam & MK_XBUTTON1) && (mbutton != M_FORWARD))
            mod |= M_MOD_FMB;
        if ((wParam & MK_XBUTTON2) && (mbutton != M_BACKWARD))
            mod |= M_MOD_BMB;
        if (GetKeyState(VK_MENU) < 0)
            mod |= M_MOD_ALT;
        XPLMCommandRef cmd = bindings_get(mbutton, mod);
        if (cmd) {
            if (state & M_STATE_DOWN)
                XPLMCommandBegin(cmd);
            if (state & M_STATE_UP)
                XPLMCommandEnd(cmd);
            return 0;
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
#elif APL
static CFMachPortRef event_tap;
static CFRunLoopSourceRef loop_src;

static mbutton_t ev_to_mbutton(CGEventType type, CGEventRef ev, int *state) {
    int n;
    switch (type) {
    case kCGEventLeftMouseDown:
        *state = M_STATE_DOWN;
        return M_LEFT;
    case kCGEventLeftMouseUp:
        *state = M_STATE_UP;
        return M_LEFT;
    case kCGEventRightMouseDown:
        *state = M_STATE_DOWN;
        return M_RIGHT;
    case kCGEventRightMouseUp:
        *state = M_STATE_UP;
        return M_RIGHT;
    case kCGEventOtherMouseDown:
    case kCGEventOtherMouseUp:
        *state = type == kCGEventOtherMouseDown ? M_STATE_DOWN :
            M_STATE_UP;
        n = CGEventGetIntegerValueField(ev, kCGMouseEventButtonNumber);
        switch (n) {
        case kCGMouseButtonCenter:
            return M_MIDDLE;
        case kCGMouseButtonForward:
            return M_FORWARD;
        case kCGMouseButtonBackward:
            return M_BACKWARD;
        default:
            _debug("unknown mouse button %i", n);
            return M_NONE;
        }
    case kCGEventScrollWheel:
        *state = M_STATE_DOWN | M_STATE_UP;
        n = CGEventGetIntegerValueField(ev,
            kCGScrollWheelEventDeltaAxis1);
        if (n != 0)
            return n > 0 ? M_W_FORWARD : M_W_BACKWARD;
        n = CGEventGetIntegerValueField(ev,
            kCGScrollWheelEventDeltaAxis2);
        if (n != 0)
            return n > 0 ? M_W_RIGHT : M_W_LEFT;
        return M_NONE;
    default:
        return M_NONE;
    }
}

CGEventRef cg_event_cb(CGEventTapProxy proxy, CGEventType type,
    CGEventRef ev, void *data) {
    int state;

    mbutton_t mbutton = ev_to_mbutton(type, ev, &state);
    if (mbutton != M_NONE) {
        int mod = 0;
        /* Figure out state of ALT, CONTROL and SHIFT keys. */
        CGEventFlags flags = CGEventSourceFlagsState(
            kCGEventSourceStateHIDSystemState);
        if (flags & kCGEventFlagMaskControl)
            mod |= M_MOD_CTRL;
        if (flags & kCGEventFlagMaskShift)
            mod |= M_MOD_SHIFT;
        if (flags & kCGEventFlagMaskAlternate)
            mod |= M_MOD_ALT;
        if (CGEventSourceButtonState(0, kCGMouseButtonLeft) &&
            mbutton != M_LEFT) {
            mod |= M_MOD_LMB;
        }
        if (CGEventSourceButtonState(0, kCGMouseButtonRight) &&
            mbutton != M_RIGHT) {
            mod |= M_MOD_RMB;
        }
        if (CGEventSourceButtonState(0, kCGMouseButtonCenter) &&
            mbutton != M_MIDDLE) {
            mod |= M_MOD_MMB;
        }
        if (CGEventSourceButtonState(0, kCGMouseButtonForward) &&
            mbutton != M_FORWARD) {
            mod |= M_MOD_FMB;
        }
        if (CGEventSourceButtonState(0, kCGMouseButtonBackward) &&
            mbutton != M_BACKWARD) {
            mod |= M_MOD_BMB;
        }
        XPLMCommandRef cmd = bindings_get(mbutton, mod);
        if (cmd) {
            if (state & M_STATE_DOWN) {
                XPLMCommandBegin(cmd);
            }
            if (state & M_STATE_UP) {
                XPLMCommandEnd(cmd);
            }
            return NULL;
        }
    }
    return ev;
}

int tap_events() {
    /* CGEventTapCreateForPid has only been added with 10.11 */
    event_tap = CGEventTapCreateForPid(getpid(), kCGTailAppendEventTap,
        kCGEventTapOptionListenOnly,
        CGEventMaskBit(kCGEventLeftMouseDown)  |
        CGEventMaskBit(kCGEventLeftMouseUp)    |
        CGEventMaskBit(kCGEventRightMouseDown) |
        CGEventMaskBit(kCGEventRightMouseUp)   |
        CGEventMaskBit(kCGEventOtherMouseDown) |
        CGEventMaskBit(kCGEventOtherMouseUp)   |
        CGEventMaskBit(kCGEventScrollWheel), cg_event_cb, NULL);
    if (!event_tap) {
        _log("could not create event tap");
        return 0;
    }
    loop_src = CFMachPortCreateRunLoopSource(NULL, event_tap, 0);
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
#endif
