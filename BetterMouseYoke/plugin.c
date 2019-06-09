/**
 * BetterMouseYoke - X-Plane 11 Plugin
 *
 * Does away with X-Plane's idiotic centered little box for mouse steering and
 * replaces it with a more sane system for those who, for whatever reason,
 * want to or have to use the mouse for flying.
 *
 * Copyright 2019 Torben Könke.
 */
#include "plugin.h"

#define PLUGIN_NAME         "BetterMouseYoke"
#define PLUGIN_SIG          "S22.BetterMouseYoke"
#define PLUGIN_DESCRIPTION  "Does away with X-Plane's idiotic centered little box " \
                            "for mouse steering that has caused much grieve and "   \
                            "countless loss of virtual lives."
#define PLUGIN_VERSION      "1.4"

static XPLMCommandRef toggle_yoke_control;
static XPLMDataRef yoke_pitch_ratio;
static XPLMDataRef yoke_roll_ratio;
static XPLMDataRef eq_pfc_yoke;
static XPLMFlightLoopID loop_id;
static int screen_width;
static int screen_height;
static int yoke_control_enabled;
static int rudder_control;
static float magenta[] = { 1.0f, 0, 1.0f };
static float green[] = { 0, 1.0f, 0 };
static int set_pos;
static int change_cursor;
static int cursor_pos[2];
#ifdef IBM
static HWND xp_hwnd;
static HCURSOR yoke_cursor;
static HCURSOR rudder_cursor;
static HCURSOR arrow_cursor;
static HCURSOR(WINAPI *true_set_cursor) (HCURSOR cursor) = SetCursor;
#endif

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
    toggle_yoke_control = XPLMCreateCommand("BetterMouseYoke/ToggleYokeControl",
        "Toggle mouse yoke control");
    yoke_pitch_ratio = XPLMFindDataRef("sim/cockpit2/controls/yoke_pitch_ratio");
    if (yoke_pitch_ratio == NULL) {
        _log("init fail: could not find yoke_pitch_ratio dataref");
        return 0;
    }
    yoke_roll_ratio = XPLMFindDataRef("sim/cockpit2/controls/yoke_roll_ratio");
    if (yoke_roll_ratio == NULL) {
        _log("init fail: could not find yoke_roll_ratio dataref");
        return 0;
    }
    eq_pfc_yoke = XPLMFindDataRef("sim/joystick/eq_pfc_yoke");
    if (eq_pfc_yoke == NULL) {
        _log("init fail: could not find eq_pfc_yoke dataref");
        return 0;
    }
    XPLMDataRef has_joystick = XPLMFindDataRef("sim/joystick/has_joystick");
    if (XPLMGetDatai(has_joystick)) {
        _log("init: joystick detected, unloading plugin");
        return 0;
    }
    set_pos = ini_geti("set_pos", 1);
    change_cursor = ini_geti("change_cursor", 1);
#ifdef IBM
    xp_hwnd = FindWindowA("X-System", "X-System");
    if (!xp_hwnd) {
        _log("could not find X-Plane 11 window");
        return 0;
    }
    if (!hook_set_cursor(1)) {
        _log("could not hook SetCursor function");
        return 0;
    }
    yoke_cursor = LoadCursor(NULL, IDC_SIZEALL);
    if (!yoke_cursor) {
        _log("could not load yoke_cursor");
        return 0;
    }
    rudder_cursor = LoadCursor(NULL, IDC_SIZEWE);
    if (!rudder_cursor) {
        _log("could not load rudder_cursor");
        return 0;
    }
    arrow_cursor = LoadCursor(NULL, IDC_ARROW);
    if (!arrow_cursor) {
        _log("could not load arrow_cursor");
        return 0;
    }
#endif
    return 1;
}

/**
 * X-Plane 11 Plugin Callback
 *
 * Called when the plugin is about to be unloaded from X-Plane 11.
 */
PLUGIN_API void XPluginStop(void) {
#ifdef IBM
    if (!hook_set_cursor(0)) {
        _log("could not unhook SetCursor function");
    }
#endif
}

/**
 * X-Plane 11 Plugin Callback
 *
 * Called when the plugin is about to be enabled. Return 1 if the plugin
 * started successfully, otherwise 0.
 */
PLUGIN_API int XPluginEnable(void) {
    XPLMRegisterCommandHandler(toggle_yoke_control, toggle_yoke_control_cb,
        0, NULL);
    XPLMRegisterDrawCallback(draw_cb, xplm_Phase_Window, 0, NULL);
    XPLMCreateFlightLoop_t params = {
        .structSize = sizeof(XPLMCreateFlightLoop_t),
        .phase = xplm_FlightLoop_Phase_BeforeFlightModel,
        .refcon = NULL,
        .callbackFunc = loop_cb
    };
    loop_id = XPLMCreateFlightLoop(&params);
    return 1;
}

/**
 * X-Plane 11 Plugin Callback
 *
 * Called when the plugin is about to be disabled.
 */
PLUGIN_API void XPluginDisable(void) {
    XPLMUnregisterCommandHandler(toggle_yoke_control, toggle_yoke_control_cb,
        0, NULL);
    XPLMSetDatai(eq_pfc_yoke, 0);
    XPLMUnregisterDrawCallback(draw_cb, xplm_Phase_Window, 0, NULL);
    if (loop_id)
        XPLMDestroyFlightLoop(loop_id);
    loop_id = NULL;
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
            /* This will hide the clickable yoke control box. */
            XPLMSetDatai(eq_pfc_yoke, 1);
        }
    }
}

int toggle_yoke_control_cb(XPLMCommandRef cmd, XPLMCommandPhase phase, void *ref) {
    if (phase != xplm_CommandBegin)
        return 1;
    if (yoke_control_enabled) {
        if (change_cursor)
            set_cursor_bmp(CURSOR_ARROW);
        yoke_control_enabled = 0;
        rudder_control = 0;
    } else {
        /* Fetch screen dimensions here because doing it from XPluginEnable
           give unrealiable results. Also the screen size may be changed by
           the user at any time. */
        XPLMGetScreenSize(&screen_width, &screen_height);
        /* Set cursor position to align with current deflection of yoke. */
        if (set_pos)
            set_cursor_pos();
        if (change_cursor)
            set_cursor_bmp(CURSOR_YOKE);
        yoke_control_enabled = 1;
        XPLMScheduleFlightLoop(loop_id, -1.0f, 0);
    }
    return 1;
}

int draw_cb(XPLMDrawingPhase phase, int before, void *ref) {
    /* Show a little text indication in top left corner of screen. */
    if (yoke_control_enabled) {
        XPLMDrawString(magenta, 20, screen_height - 40, rudder_control ?
            "MOUSE RUDDER CONTROL" : "MOUSE YOKE CONTROL",
            NULL, xplmFont_Proportional);
        if (rudder_control) {
            /* Draw little bars to indicate maximum rudder deflection. */
            for (int i = 1; i < 3; i++) {
                XPLMDrawString(green, cursor_pos[0] - 100,
                    cursor_pos[1] + 4 - 7 * i, "|", NULL, xplmFont_Basic);
                XPLMDrawString(green, cursor_pos[0] + 100,
                    cursor_pos[1] + 4 - 7 * i, "|", NULL, xplmFont_Basic);
            }
        }
    }
    return 1;
}

float loop_cb(float last_call, float last_loop, int count, void *ref) {
    /* If user has disabled mouse yoke control, suspend loop. */
    if (yoke_control_enabled == 0) {
        /* if rudder is still deflected, interpolate it back to neutral */
        return 0;
    }
    int m_x, m_y;
    XPLMGetMouseLocationGlobal(&m_x, &m_y);
    if (controlling_rudder()) {
        /* TODO */
        /* (m_x - cursor_pos[0]) */
    } else {
        float yoke_roll = 2 * (m_x / (float)screen_width) - 1;
        float yoke_pitch = 1 - 2 * (m_y / (float)screen_height);
        XPLMSetDataf(yoke_roll_ratio, yoke_roll);
        XPLMSetDataf(yoke_pitch_ratio, yoke_pitch);
    }
    /* Call us again next frame. */
    return -1.0f;
}

int left_mouse_down() {
#ifdef IBM
    /* Most significant bit is set if button is being held. */
    return GetAsyncKeyState(VK_LBUTTON) >> 15;
#elif APL
    /* TODO */
#endif
}

int controlling_rudder() {
    if (left_mouse_down()) {
        /* Transitioning into rudder control */
        if (!rudder_control) {
            if (change_cursor)
                set_cursor_bmp(CURSOR_RUDDER);
            /* Remember current cursor position */
            XPLMGetMouseLocationGlobal(cursor_pos, cursor_pos + 1);
            rudder_control = 1;
        }
    } else {
        /* Transitioning out of rudder control. */
        if (rudder_control) {
            if (change_cursor)
                set_cursor_bmp(CURSOR_YOKE);
            /* Restore cursor position */

            rudder_control = 0;
        }
    }
    return rudder_control;
}

void set_cursor_pos() {
    int x = 0.5 * screen_width * (XPLMGetDataf(yoke_roll_ratio) + 1);
    int y = 0.5 * screen_height * (1 - XPLMGetDataf(yoke_pitch_ratio));
#ifdef IBM
    POINT pt;
    GetCursorPos(&pt);
    /* Convert to coordinates relative to X-Plane window. */
    ScreenToClient(xp_hwnd, &pt);
    pt.x = x;
    /* On windows (0,0) is the upper-left corner. */
    pt.y = screen_height - y;
    ClientToScreen(xp_hwnd, &pt);
    SetCursorPos(pt.x, pt.y);
#elif APL
    /* TODO */
#endif
}

void set_cursor_bmp(cursor_t cursor) {
#ifdef IBM
    HCURSOR c = arrow_cursor;
    switch (cursor) {
    case CURSOR_YOKE:
        c = yoke_cursor;
        break;
    case CURSOR_RUDDER:
        c = rudder_cursor;
        break;
    }
    true_set_cursor(c);
#elif APL
    /* TODO */
#endif
}

#ifdef IBM
HCURSOR WINAPI set_cursor(HCURSOR cursor) {
    if (!yoke_control_enabled)
        return true_set_cursor(cursor);
    return cursor;
}

int hook_set_cursor(int attach) {
    long err;
    if ((err = DetourTransactionBegin())) {
        _log("DetourTransactionBegin error (%i)", err);
        return 0;
    }
    if ((err = DetourUpdateThread(GetCurrentThread()))) {
        _log("DetourUpdateThread error (%i)", err);
        return 0;
    }
    if (attach) {
        if ((err = DetourAttach((void**)&true_set_cursor, set_cursor))) {
            _log("DetourAttach error (%i)", err);
            return 0;
        }
    } else {
        if ((err = DetourDetach((void**)&true_set_cursor, set_cursor))) {
            _log("DetourDetach error (%i)", err);
            return 0;
        }
    }
    if ((err = DetourTransactionCommit())) {
        _log("DetourTransactionCommit error (%i)", err);
        return 0;
    }
    return 1;
}
#endif
