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
#define PLUGIN_VERSION      "1.5"

#define RUDDER_DEFL_DIST    200
#define RUDDER_RET_SPEED    2.0f

static XPLMCommandRef toggle_yoke_control;
static XPLMDataRef yoke_pitch_ratio;
static XPLMDataRef yoke_roll_ratio;
static XPLMDataRef yoke_heading_ratio;
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
static int set_rudder_pos;
static int rudder_return;
static int rudder_defl_dist;
static float yaw_ratio;
static float rudder_ret_spd;
#ifdef IBM
static HWND xp_hwnd;
static HCURSOR yoke_cursor;
static HCURSOR rudder_cursor;
static HCURSOR arrow_cursor;
static HCURSOR(WINAPI *true_set_cursor) (HCURSOR cursor) = SetCursor;
#elif LIN
    Display *dpy;
    XEvent ev;
    XIEvent *xi_event;
    XIRawEvent *xev;
    Cursor cross;
    Cursor arrows;
    Cursor def;
    Window window;
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
    yoke_heading_ratio = XPLMFindDataRef(
        "sim/cockpit2/controls/yoke_heading_ratio");
    if (yoke_heading_ratio == NULL) {
        _log("init fail: could not find yoke_heading_ratio dataref");
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
    if (!init_menu()) {
        _log("init: could not init menu");
        return 0;
    }
    rudder_defl_dist = ini_geti("rudder_deflection_distance", RUDDER_DEFL_DIST);
    rudder_ret_spd = ini_getf("rudder_return_speed", RUDDER_RET_SPEED);
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
#elif LIN
int xi_opcode, event, error;

    dpy = XOpenDisplay(NULL);
    if (!dpy) {
    _log("Failed to open display.\n");
    return 0;
    }

    window = XDefaultRootWindow(dpy);
    if (!XQueryExtension(dpy, "XInputExtension", &xi_opcode, &event, &error)) {
       _log("X Input extension not available.\n");
          return 0;
    }

    if (!has_xi2(dpy))
    return 0;

    def = XcursorLibraryLoadCursor(dpy, "left_ptr");
    cross = XcursorLibraryLoadCursor(dpy, "cross");
    arrows = XcursorLibraryLoadCursor(dpy, "sb_h_double_arrow");

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
#elif LIN
    XDefineCursor(dpy, window, def);
    XFreeCursor(dpy, cross);
    XFreeCursor(dpy, arrows);
    XFreeCursor(dpy, def);
    XFlush(dpy);
    XCloseDisplay(dpy);
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
    menu_deinit();
#ifdef LIN
    XDefineCursor(dpy, window, def);
    XFlush(dpy);
#endif // LIN
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

int init_menu() {
    menu_item_t items[] = {
        { "Set Yoke Cursor", "set_pos", &set_pos, 1 },
        { "Set Rudder Cursor", "set_rudder_pos", &set_rudder_pos, 1 },
        { "Change Cursor Icon", "change_cursor", &change_cursor, 1 },
        { "Return Rudder", "rudder_return", &rudder_return, 1 }
    };
    int num = sizeof(items) / sizeof(items[0]);

    return menu_init(PLUGIN_NAME, items, num);
}

int toggle_yoke_control_cb(XPLMCommandRef cmd, XPLMCommandPhase phase, void *ref) {
    if (phase != xplm_CommandBegin)
        return 1;
    if (yoke_control_enabled) {
        if (change_cursor)
            set_cursor_bmp(CURSOR_ARROW);
        yoke_control_enabled = 0;
        rudder_control = 0;
#ifdef LIN
    deselect_events(dpy, window);
#endif // LIN
    } else {
#ifdef LIN
    select_events(dpy, window);
#endif // LIN
        /* Fetch screen dimensions here because doing it from XPluginEnable
           give unrealiable results. Also the screen size may be changed by
           the user at any time. */
        XPLMGetScreenSize(&screen_width, &screen_height);
        /* Set cursor position to align with current deflection of yoke. */
        if (set_pos)
            set_cursor_from_yoke();
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
                XPLMDrawString(green, cursor_pos[0] - rudder_defl_dist,
                    cursor_pos[1] + 4 - 7 * i, "|", NULL, xplmFont_Basic);
                XPLMDrawString(green, cursor_pos[0] + rudder_defl_dist,
                    cursor_pos[1] + 4 - 7 * i, "|", NULL, xplmFont_Basic);
            }
        }
    }
    return 1;
}

float loop_cb(float last_call, float last_loop, int count, void *ref) {
    static long long _last_time;
    /* If user has disabled mouse yoke control, suspend loop. */
    if (yoke_control_enabled == 0) {
        /* If rudder is still deflected, move it gradually back to zero. */
        if (yaw_ratio != 0 && rudder_return) {
            long long now = get_time_ms();
            float dt = (now - _last_time) / 1000.0f;
            _last_time = now;
            yaw_ratio = yaw_ratio > 0 ? max(0, yaw_ratio - dt * rudder_ret_spd) :
                min(0, yaw_ratio + dt * rudder_ret_spd);
            XPLMSetDataf(yoke_heading_ratio, yaw_ratio);
            /* Call us again next frame until zero. */
            return yaw_ratio ? -1.0f : 0;
        }
        /* Don't call us anymore. */
        return 0;
    }
    int m_x, m_y;
    get_cursor_pos(&m_x, &m_y);
    if (controlling_rudder(&m_x, &m_y)) {
        int dist = min(max(m_x - cursor_pos[0], -rudder_defl_dist),
            rudder_defl_dist);
        _last_time = get_time_ms();
        /* Save value so we don't have to continuously query the dr above. */
        yaw_ratio = dist / (float)rudder_defl_dist;
        XPLMSetDataf(yoke_heading_ratio, yaw_ratio);
    } else {
        float yoke_roll = 2 * (m_x / (float)screen_width) - 1;
        float yoke_pitch = 1 - 2 * (m_y / (float)screen_height);
        XPLMSetDataf(yoke_roll_ratio, yoke_roll);
        XPLMSetDataf(yoke_pitch_ratio, yoke_pitch);
        /* If rudder is still deflected, move it gradually back to zero. */
        if (yaw_ratio != 0 && rudder_return) {
            long long now = get_time_ms();
            float dt = (now - _last_time) / 1000.0f;
            _last_time = now;
            yaw_ratio = yaw_ratio > 0 ? max(0, yaw_ratio - dt * rudder_ret_spd) :
                min(0, yaw_ratio + dt * rudder_ret_spd);
            XPLMSetDataf(yoke_heading_ratio, yaw_ratio);
        }
    }
    /* Call us again next frame. */
    return -1.0f;
}

int left_mouse_down() {
#ifdef IBM
    /* Most significant bit is set if button is being held. */
    return GetAsyncKeyState(VK_LBUTTON) >> 15;
#elif APL
    /* Apparently you can use this also outside of the context of an event. */
    return CGEventSourceButtonState(
        kCGEventSourceStateCombinedSessionState, kCGMouseButtonLeft);
#elif LIN

static int mouse_but = 0;

    XGenericEventCookie *cookie = &ev.xcookie;

    XCheckTypedEvent(dpy, GenericEvent ,&ev);

    if (cookie->type != GenericEvent ||
        !XGetEventData(dpy, cookie))
        return mouse_but;

    xi_event = (XIEvent *) cookie->data;
    xev = (XIRawEvent *) xi_event;

    if(xev->detail == 1)
    {
        switch (cookie->evtype) {
            case XI_RawButtonPress:
                mouse_but = 1;
                break;
            case XI_RawButtonRelease:
                mouse_but = 0;
                break;
            }

        XFreeEventData(dpy, cookie);
    }
    return mouse_but;
#endif
}

int controlling_rudder(int *x, int *y) {
    if (left_mouse_down()) {
        /* Transitioning into rudder control */
        if (!rudder_control) {
            if (change_cursor)
                set_cursor_bmp(CURSOR_RUDDER);
            /* Remember current cursor position. */
            XPLMGetMouseLocationGlobal(cursor_pos, cursor_pos + 1);
            /* Set rudder cursor position, if enabled. */
            if (set_rudder_pos) {
                *x = *x + yaw_ratio * rudder_defl_dist;
                set_cursor_pos(*x, *y);
            }
            rudder_control = 1;
        }
    } else {
        /* Transitioning out of rudder control. */
        if (rudder_control) {
            if (change_cursor)
                set_cursor_bmp(CURSOR_YOKE);
            /* Restore previous cursor position */
            set_cursor_pos(cursor_pos[0], cursor_pos[1]);
            *x = cursor_pos[0];
            *y = cursor_pos[1];
            rudder_control = 0;
        }
    }
    return rudder_control;
}

void get_cursor_pos(int *x, int *y) {
#ifdef APL
    /* On OSX, XPLMGetMouseLocationGlobal still returns old cursor location after
        setting its position for whatever reason, so we query the cursor position
        ourselves. */
    CGEventRef ev = CGEventCreate(NULL);
    CGPoint pt = CGEventGetLocation(ev);
    CFRelease(ev);
    *x = pt.x;
    *y = screen_height - pt.y;
#else
    XPLMGetMouseLocationGlobal(x, y);
#endif
}

void set_cursor_from_yoke() {
    set_cursor_pos(
        0.5 * screen_width  * (XPLMGetDataf(yoke_roll_ratio) + 1),
        0.5 * screen_height * (1 - XPLMGetDataf(yoke_pitch_ratio))
    );
}

void set_cursor_pos(int x, int y) {
#ifdef IBM
    POINT pt = {
        .x = x,
        /* On windows (0,0) is the upper-left corner. */
        .y = screen_height - y
    };
    ClientToScreen(xp_hwnd, &pt);
    SetCursorPos(pt.x, pt.y);
#elif APL
    CGPoint pt = {
        .x = x,
        .y = screen_height - y
    };
    /* CGWarpMouseCursorPosition and CGDisplayMoveCursorToPoint don't generate a mouse
        movement event so they're not a good fit here. */
    CGEventRef ev = CGEventCreateMouseEvent(NULL, kCGEventMouseMoved, pt, 0);
    CGEventPost(kCGHIDEventTap, ev);
    CFRelease(ev);
#elif LIN
    XWarpPointer(dpy, None, window, 0, 0, 0, 0, x, screen_height - y);
    XFlush(dpy);
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
    /* Can probably use NSCursor::set for this but not sure we can hook
       that under OSX to prevent XP from constantly overriding our cursor...*/
#elif LIN
 /*TODO*/
 /*Use custom png cursors*/
    Cursor c = def;
    switch (cursor) {
    case CURSOR_YOKE:
        c = cross;
        break;
    case CURSOR_RUDDER:
        c = arrows;
        break;
    case CURSOR_ARROW:
        c = def;
        break;
    }
    XDefineCursor(dpy, window, c);
    XFlush(dpy);
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
#elif LIN

static int has_xi2(Display *dpy)
{
    int major, minor;
    int rc;

    /* We support XI 2.2 */
    major = 2;
    minor = 2;

    rc = XIQueryVersion(dpy, &major, &minor);
    if (rc == BadRequest) {
    _log("No XI2 support.\n");
    return 0;
    } else if (rc != Success) {
    _log("Internal Error! This is a bug in Xlib.\n");
    }

    _log("XI2 supported.\n");

    return 1;
}

static void select_events(Display *dpy, Window win)
{
    XIEventMask evmasks[1];
    unsigned char mask1[(XI_LASTEVENT + 7)/8];

    memset(mask1, 0, sizeof(mask1));

    /* select for button and key events from all master devices */
    XISetMask(mask1, XI_RawButtonPress);
    XISetMask(mask1, XI_RawButtonRelease);

    evmasks[0].deviceid = XIAllMasterDevices;
    evmasks[0].mask_len = sizeof(mask1);
    evmasks[0].mask = mask1;

    XISelectEvents(dpy, win, evmasks, 1);
    XFlush(dpy);
}

static void deselect_events(Display *dpy, Window win)
{
    XIEventMask evmasks[1];
    unsigned char mask1[(XI_LASTEVENT + 7)/8];

    memset(mask1, 0, sizeof(mask1));

    /* select for button and key events from all master devices */
    XISetMask(mask1, 0);

    evmasks[0].deviceid = XIAllMasterDevices;
    evmasks[0].mask_len = sizeof(mask1);
    evmasks[0].mask = mask1;

    XISelectEvents(dpy, win, evmasks, 1);
    XFlush(dpy);
}

#endif
