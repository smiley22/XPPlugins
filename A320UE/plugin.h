/**
 * A320UE - X-Plane 11 Plugin
 *
 * A plugin for the FlightFactor A320 Ultimate that adds a couple of new
 * commands for operating the thrust levers more comfortably as well as a
 * bunch of other little workarounds and/or features.
 *
 * Copyright 2019 Torben Könke.
 */
#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#ifdef _WIN32
#pragma comment(lib, "../FMOD/Libs/fmod64_vc.lib")
#endif /* _WIN32 */

#include "a320.h"
#include "../Util/util.h"
#include "../XP/XPLMProcessing.h"
#include "../XP/XPLMGraphics.h"
#include "../XP/XPLMDisplay.h"
#include <math.h>

/* plugin */
void plugin_init();
void plugin_deinit();

/* ff */
typedef SharedValuesInterface ff_api_t;
typedef void(*ff_init_done_cb)();
int ff_init(ff_init_done_cb cb);
void ff_deinit();
float ff_loop_cb(float last_call, float last_loop, int count, void *data);
int ff_get_id(const char *name);
int ff_get_int(int id);
void ff_set_int(int id, int val);
float ff_get_float(int id);
void ff_set_float(int id, float val);

/* levers */
void levers_init();
void levers_deinit();
int levers_next_detent(XPLMCommandRef cmd, XPLMCommandPhase phase, void *refcon);
int levers_next_step(XPLMCommandRef cmd, XPLMCommandPhase phase, void *refcon);
void levers_draw_string(const char *s);

/* v1 */
void v1_init();
void v1_deinit();
float v1_loop_cb(float last_call, float last_loop, int count, void *ref);

#endif /* _PLUGIN_H_ */
