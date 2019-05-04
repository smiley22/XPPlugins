/**
 * A320UE - X-Plane 11 Plugin
 *
 * A plugin for the FlightFactor A320 Ultimate that adds a couple of new
 * commands for operating the thrust levers more comfortably as well as a
 * bunch of other little workarounds and/or features.
 *
 * Copyright 2019 Torben Könke.
 */
#include "plugin.h"

#define PLUGIN_NAME         "A320 Ultimate Extended"
#define PLUGIN_SIG          "S22.A320UE"
#define PLUGIN_DESCRIPTION  "Adds two new commands for cycling through a plane's " \
                            "configured quick looks."
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
    /* This is really lame, we get called too early in the process when the
       FF API hasn't been initialized yet, so we have to keep polling until
       it is and then continue with the rest of our initialization. */
    return ff_init(plugin_init);
}

/**
* X-Plane 11 Plugin Callback
*
* Called when the plugin is about to be disabled.
*/
PLUGIN_API void XPluginDisable(void) {
    /* clean up */
    plugin_deinit();
}

/**
* X-Plane 11 Plugin Callback
*
* Called when a message is sent to the plugin by X-Plane 11 or another plugin.
*/
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID from, int msg, void *param) {
}

/**
 * Initializes the plugin.
 *
 * Gets called once the FF API has been initialized so we can make use of
 * their provided functions for manipulating A320U values.
 */
void plugin_init() {
    snd_init();
    levers_init();
    v1_init();
}

void plugin_deinit() {
    snd_deinit();
    ff_deinit();
    levers_deinit();
    v1_deinit();
}
