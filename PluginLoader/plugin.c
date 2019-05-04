/**
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
#define PLUGIN_VERSION      "1.1"

#define MAX_PLUGINS         20
static plugin_t plugins[MAX_PLUGINS];
static int num_plugins;
static XPLMCommandRef reload;
static float magenta[] = { 1.0f, 0, 1.0f };
static float cyan[] = { 0, 1.0f, 1.0f };
static long long last_reload;
static char info[3][128];

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
    XPLMRegisterDrawCallback(draw_cb, xplm_Phase_Window, 0, NULL);
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
    XPLMUnregisterDrawCallback(draw_cb, xplm_Phase_Window, 0, NULL);
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
    }
    return 0;
}

int draw_cb(XPLMDrawingPhase phase, int before, void *data) {
    if (num_plugins > 0) {
        /* Fade colour from white to magenta if just reloaded as visual
           indicator. */
        long long dt = min(1000, get_time_ms() - last_reload);
        magenta[1] = (1000 - dt) / 1000.0f;
        XPLMDrawString(magenta, 20, 50, info[0], NULL, xplmFont_Proportional);
        XPLMDrawString(magenta, 20, 30, info[1], NULL, xplmFont_Proportional);
        XPLMDrawString(magenta, 20, 10, info[2], NULL, xplmFont_Proportional);
    } else {
        XPLMDrawString(cyan, 20, 50, "No plugin loaded", NULL, xplmFont_Proportional);
    }
    return 1;
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
    strcpy(plugin->path, file);
    /* Copy Dll file to temporary file. */
    char buf[MAX_PATH];
    strcpy(buf, file);
    char *p = strrchr(buf, '.');
    if (!p)
        return 0;
    strcpy(p, ".tmp");
    if (!CopyFileA(file, buf, FALSE)) {
        _log("could not copy file '%s' to '%s'", file, buf);
        return 0;
    }
    if (!(plugin->mod = LoadLibraryA(buf))) {
        _log("could not load library '%s' (%i)", file, GetLastError());
        return 0;
    }
    if (!init_func_ptrs(plugin)) {
        _log("could not init function pointers for '%s'", file);
        FreeLibrary(plugin->mod);
        return 0;
    }
    char sig[256], desc[256];
    if(!plugin->XPluginStart(plugin->name, sig, desc)) {
        _log("XPluginStart returned 0 for '%s'", plugin->path);
        FreeLibrary(plugin->mod);
        return 0;
    }
    _log("plugin loaded (%s, %s, %s)", plugin->name, sig, desc);
    if (enable)
        plugin->XPluginEnable();
    return 1;
}

/**
 * Loads all plugin Dlls that we can find and returns the number of Dlls that
 * have been loaded.
 */
int load_plugins(int enable) {
    /* Unload plugins first if they have already been loaded. */
    unload_plugins();
    char dir[MAX_PATH];
    if (!get_plugin_dir(dir, MAX_PATH))
        return 0;
    char buf[MAX_PATH];
    sprintf(buf, "%s64/*.dll", dir);
    WIN32_FIND_DATAA ffd;
    HANDLE h;
    if ((h = FindFirstFileA(buf, &ffd)) == INVALID_HANDLE_VALUE) {
        _log("FindFirstFileA failed for %s (%i)", buf, GetLastError());
        return 0;
    }
    int loaded = 0;
    do {
        sprintf(buf, "%s64/%s", dir, ffd.cFileName);
        if (load_plugin(buf, enable, &plugins[loaded])) {
            loaded++;
        }
    } while (FindNextFileA(h, &ffd) != 0);
    FindClose(h);
    _log("%i plugins loaded", loaded);
    last_reload = get_time_ms();
    /* Update information shown on screen. */
    sprintf(info[0], "%i plugin(s) loaded", loaded);
    strcpy(info[1], "[");
    for (int i = 0; i < loaded; i++) {
        strcat(info[1], plugins[i].name);
        strcat(info[1], i < (loaded - 1) ? ", " : "]");
    }
    time_t t = time(NULL);
    struct tm *lt = localtime(&t);
    strftime(info[2], sizeof(info[2]), "Last reload at %d/%m/%y - %T", lt);
    return loaded;
}

/**
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
}
