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
#define PLUGIN_VERSION      "1.1"

static XPLMCommandRef toggle_mouse_look;
static XPLMCommandRef hold_mouse_look;
static int show_hint;
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
}

/**
* X-Plane 11 Plugin Callback
*
* Called when a message is sent to the plugin by X-Plane 11 or another plugin.
*/
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID from, int msg, void *param) {
}

int toggle_cb(XPLMCommandRef cmd, XPLMCommandPhase phase, void *data) {
    if (phase == xplm_CommandBegin) {
        short state = GetAsyncKeyState(VK_RBUTTON);
        /* Most significant bit is set if button is being held. */
        show_hint = !(state >> 15);
        INPUT input = { 0 };
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = show_hint ? MOUSEEVENTF_RIGHTDOWN :
            MOUSEEVENTF_RIGHTUP;
        SendInput(1, &input, sizeof(INPUT));
        /* Get screen height for offsetting text indication. */
        XPLMGetScreenSize(NULL, &screen_height);
    }
    return 0;
}

int hold_cb(XPLMCommandRef cmd, XPLMCommandPhase phase, void *data) {
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    switch (phase) {
    case xplm_CommandBegin:
        show_hint = 1;
        input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
        /* Get screen height for offsetting text indication. */
        XPLMGetScreenSize(NULL, &screen_height);
        break;
    case xplm_CommandEnd:
        show_hint = 0;
        input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
        break;
    default:
        return 0;
    }
    SendInput(1, &input, sizeof(INPUT));
    return 0;
}

int draw_cb(XPLMDrawingPhase phase, int before, void *ref) {
    if (show_hint) {
        XPLMDrawString(magenta, 20, screen_height - 20, "MOUSELOOK", NULL,
            xplmFont_Proportional);
    }
    return 1;
}
