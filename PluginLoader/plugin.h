/*
 * PluginLoader - X-Plane 11 Plugin
 *
 * Enables dynamic loading und unloading of plugins dll so that one does not
 * need to constantly restart X-Plane 11 during development and testing
 * plugins.
 *
 * Copyright 2019 Torben Könke. All rights reserved.
 */
#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include "../Util/util.h"
#include "../XP/XPLMPlugin.h"

typedef int(__cdecl *XPluginStartProc)(char *name, char *sig, char *desc);
typedef void(__cdecl *XPluginStopProc)(void);
typedef int(__cdecl *XPluginEnableProc)(void);
typedef void(__cdecl *XPluginDisableProc)(void);
typedef void(__cdecl *XPluginReceiceMessageProc)(XPLMPluginID from, int msg,
    void *param);

typedef struct {
    HMODULE mod;
    char path[MAX_PATH];
    char modified[128];
    XPluginStartProc XPluginStart;
    XPluginStopProc XPluginStop;
    XPluginEnableProc XPluginEnable;
    XPluginDisableProc XPluginDisable;
    XPluginReceiceMessageProc XPluginReceiveMessage;
} plugin_t;

int reload_cb(XPLMCommandRef cmd, XPLMCommandPhase phase, void *data);
int setup_function_pointers(HMODULE mod);
int load_plugins(int enable);
void unload_plugins();

#endif /* _PLUGIN_H_ */