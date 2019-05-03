#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include "../Util/util.h"
#include "../XP/XPLMDisplay.h"
#include "../XP/XPLMGraphics.h"
#include "../XP/XPLMProcessing.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

int toggle_yoke_control_cb(XPLMCommandRef cmd, XPLMCommandPhase phase, void *ref);
int draw_cb(XPLMDrawingPhase phase, int before, void *ref);
float loop_cb(float last_call, float last_loop, int count, void *ref);
#endif /* _PLUGIN_H_ */
