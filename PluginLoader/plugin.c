/*
 * PluginLoader - X-Plane 11 Plugin
 *
 * Enables dynamic loading und unloading of a plugin's dll so that one does
 * not need to constantly restart X-Plane 11 during development and testing
 * of a plugin.
 *
 * Copyright 2019 Torben Könke. All rights reserved.
 */
#include "plugin.h"

#define PLUGIN_NAME         "PluginLoader"
#define PLUGIN_SIG          "S22.PluginLoader"
#define PLUGIN_DESCRIPTION  "Allows dynamically loading and unloading plugin " \
                            "DLLs on the fly."

static plugin_t plugin;
static XPLMCommandRef reload_cmd;
static float magenta[] = { 1.0f, 0, 1.0f };
static float cyan[] = { 0, 1.0f, 1.0f };
static long last_reload_time;

/*
 * We copy the actual plugin dll to a file with this name that then gets loaded
 * so that Windows will not lock our original dll.
 */
#define PLUGIN_TEMP_DLL_NAME "z_plugin.tmp"

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
    return 1;
}

/**
 * X-Plane 11 Plugin Callback
 *
 * Called when the plugin is about to be disabled.
 */
PLUGIN_API void XPluginDisable(void) {
}

/**
 * X-Plane 11 Plugin Callback
 *
 * Called when a message is sent to the plugin by X-Plane 11 or another plugin.
 */
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID from, int msg, void *param) {
}