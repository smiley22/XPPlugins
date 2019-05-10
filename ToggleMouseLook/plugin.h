/**
 * ToggleMouseLook - X-Plane 11 Plugin
 *
 * Adds two new commands that mimic the mouse look behaviour of Prepar3D.
 *
 * Copyright 2019 Torben Könke.
 */
#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include "../Util/util.h"
#include "../XP/XPLMDisplay.h"
#include "../XP/XPLMGraphics.h"
#include "../XP/XPLMProcessing.h"

int toggle_cb(XPLMCommandRef cmd, XPLMCommandPhase phase, void *data);
int hold_cb(XPLMCommandRef cmd, XPLMCommandPhase phase, void *data);
int draw_cb(XPLMDrawingPhase phase, int before, void *ref);

#ifdef IBM
int hook_wnd_proc();
int unhook_wnd_proc();
#elif APL
void mouse_event(CGEventType type);
int tap_events();
int untap_events();
#endif

#endif /* _PLUGIN_H_ */
