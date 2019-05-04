/**
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
#include "../XP/XPLMDisplay.h"
#include "../XP/XPLMGraphics.h"
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef int(__cdecl *XPluginStartProc)(char *name, char *sig, char *desc);
typedef void(__cdecl *XPluginStopProc)(void);
typedef int(__cdecl *XPluginEnableProc)(void);
typedef void(__cdecl *XPluginDisableProc)(void);
typedef void(__cdecl *XPluginReceiceMessageProc)(XPLMPluginID from, int msg,
    void *param);

typedef struct {
    HMODULE mod;
    char path[MAX_PATH];
    char name[256];
    XPluginStartProc XPluginStart;
    XPluginStopProc XPluginStop;
    XPluginEnableProc XPluginEnable;
    XPluginDisableProc XPluginDisable;
    XPluginReceiceMessageProc XPluginReceiveMessage;
} plugin_t;

int reload_cb(XPLMCommandRef cmd, XPLMCommandPhase phase, void *data);
int draw_cb(XPLMDrawingPhase phase, int before, void *data);
int load_plugins(int enable);
void unload_plugins();

#endif /* _PLUGIN_H_ */
