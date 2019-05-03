/*
 * PluginLoader - X-Plane 11 Plugin
 *
 * Enables dynamic loading und unloading of plugins dll so that one does not
 * need to constantly restart X-Plane 11 during development and testing
 * plugins.
 *
 * Copyright 2019 Torben Könke. All rights reserved.
 */
#include "plugin.h"

#define PLUGIN_NAME         "PluginLoader"
#define PLUGIN_SIG          "S22.PluginLoader"
#define PLUGIN_DESCRIPTION  "Implements a command for reloading plugin Dlls " \
                            "on-the-fly."

#define MAX_PLUGINS         20
static plugin_t plugins[MAX_PLUGINS];
static int num_plugins;
static XPLMCommandRef reload;
static float magenta[] = { 1.0f, 0, 1.0f };
static float cyan[] = { 0, 1.0f, 1.0f };
static long long last_reload;

/*
 * We copy the actual plugin dlls to temp files that then get loaded instead
 * so that Windows will not lock the original dlls.
 */
#define PLUGIN_TEMP_DLL_NAME "z_plugin_%i.tmp"

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
    /* Don't invoke plugins own enable functions just yet because we're going
       to get a XPluginEnable call from X-Plane after this that's going to be
       forwarded to the plugins. */
    num_plugins = load_plugins(0);

    return 1;
}

/**
 * X-Plane 11 Plugin Callback
 *
 * Called when the plugin is about to be unloaded from X-Plane 11.
 */
PLUGIN_API void XPluginStop(void) {
    unload_plugins();
}

/**
 * X-Plane 11 Plugin Callback
 *
 * Called when the plugin is about to be enabled. Return 1 if the plugin
 * started successfully, otherwise 0.
 */
PLUGIN_API int XPluginEnable(void) {
    for (int i = 0; i < num_plugins; i++) {
        plugins[i].XPluginEnable();
    }
    reload = cmd_create("Plugin/Reload", "Reload Plugin Dll(s)", reload_cb,
        NULL);
    return 1;
}

/**
 * X-Plane 11 Plugin Callback
 *
 * Called when the plugin is about to be disabled.
 */
PLUGIN_API void XPluginDisable(void) {
    for (int i = 0; i < num_plugins; i++) {
        plugins[i].XPluginDisable();
    }
    cmd_free(reload);
}

/**
 * X-Plane 11 Plugin Callback
 *
 * Called when a message is sent to the plugin by X-Plane 11 or another plugin.
 */
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID from, int msg, void *param) {
    for (int i = 0; i < num_plugins; i++) {
        plugins[i].XPluginReceiveMessage(from, msg, param);
    }
}

int reload_cb(XPLMCommandRef cmd, XPLMCommandPhase phase, void *data) {
    if (phase == xplm_CommandBegin) {
        load_plugins(1);
        last_reload = get_time_ms();
    }
    return 0;
}

int init_func_ptrs(plugin_t *plugin) {
    if (!(plugin->XPluginStart = (XPluginStartProc)GetProcAddress(
        plugin->mod, "XPluginStart"))) {
        _debug("XPluginStart not found (%s)", plugin->path);
        return 0;
    }
    if (!(plugin->XPluginStop = (XPluginStopProc) GetProcAddress(
        plugin->mod, "XPluginStop"))) {
        _debug("XPluginStop not found (%s)", plugin->path);
        return 0;
    }
    if (!(plugin->XPluginEnable = (XPluginEnableProc) GetProcAddress(
        plugin->mod, "XPluginEnable"))) {
        _debug("XPluginEnable not found (%s)", plugin->path);
        return 0;
    }
    if (!(plugin->XPluginDisable = (XPluginDisableProc) GetProcAddress(
        plugin->mod, "XPluginDisable"))) {
        _debug("XPluginDisable not found (%s)", plugin->path);
        return 0;
    }
    if (!(plugin->XPluginReceiveMessage = (XPluginReceiceMessageProc)
        GetProcAddress(plugin->mod, "XPluginReceiveMessage"))) {
        _debug("XPluginReceiveMessage not found (%s)", plugin->path);
        return 0;
    }
    return 1;
}

int load_plugin(const char *file, int enable, plugin_t *plugin) {
    /* Copy Dll file to temporary file. */
    char buf[MAX_PATH];
    strcpy(buf, file);
    char *p = strrchr(buf, '.');
    if (!p)
        return 0;
    strcpy(p, ".tmp");
    if (!CopyFile(file, buf, FALSE)) {
        _log("load_plugin: could not copy file '%s' to '%s'", file, buf);
        return 0;
    }
    if (!(plugin->mod = LoadLibraryA(buf))) {
        _log("load_plugin: could not load library '%s' (%i)", file,
            GetLastError());
        return 0;
    }
    if (!init_func_ptrs(plugin)) {
        _log("load_plugin: could not init function pointers for '%s'", file);
        FreeLibrary(plugin->mod);
        return 0;
    }
    char name[256], sig[256], desc[256];
    if(!plugin->XPluginStart(name, sig, desc)) {
        _log("load_plugin: XPluginStart returned 0 for '%s'", plugin->path);
        FreeLibrary(plugin->mod);
        return 0;
    }
    _log("load_plugin: plugin loaded (%s, %s, %s)", name, sig, desc);
    if (enable)
        plugin->XPluginEnable();
    return 1;
}

/*
 * Loads all plugin Dlls that we can find and returns the number of Dlls that
 * have been loaded.
 */
int load_plugins(int enable) {
    /* unload plugins first if they have already been loaded */
    unload_plugins();

    int num_files = 0;
    int loaded = 0;
    for (int i = 0; i < num_files; i++) {
        if (load_plugin(NULL, enable, &plugins[loaded])) {
            loaded++;
        }
    }
    _log("%i plugins loaded", loaded);
    return loaded;
}

/*
 * Gracefully unloads all loaded plugins and unmaps them from XP's address
 * space.
 */
void unload_plugins() {
    for (int i = 0; i < num_plugins; i++) {
        _debug("unloading plugin '%s'", plugins[i].path);
        plugins[i].XPluginDisable();
        plugins[i].XPluginStop();
        if (!FreeLibrary(plugins[i].mod)) {
            _log("could not free library (%s)", plugins[i].path);
        }
    }
    _log("%i plugins unloaded", num_plugins);
    num_plugins = 0;
    memset(plugins, 0, sizeof(plugins));
}