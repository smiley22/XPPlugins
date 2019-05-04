#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include "../Util/util.h"
#include "../XP/XPLMDisplay.h"
#include "../XP/XPLMGraphics.h"
#include "../XP/XPLMProcessing.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

int toggle_cb(XPLMCommandRef cmd, XPLMCommandPhase phase, void *data);
int hold_cb(XPLMCommandRef cmd, XPLMCommandPhase phase, void *data);
int draw_cb(XPLMDrawingPhase phase, int before, void *ref);

#endif /* _PLUGIN_H_ */
