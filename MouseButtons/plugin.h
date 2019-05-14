/**
 * MouseButtons - X-Plane 11 Plugin
 *
 * Enables the use of extra mouse buttons and allows the right mouse button
 * and mouse wheel to be re-assigned to arbitrary commands.
 *
 * Copyright 2019 Torben Könke.
 */
#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include "../Util/util.h"

typedef enum {
    M_NONE,
    M_RIGHT,
    M_MIDDLE,
    M_FORWARD,
    M_BACKWARD,
    M_W_FORWARD,
    M_W_BACKWARD,
    M_W_LEFT,
    M_W_RIGHT
} mbutton_t;

#define M_MOD_CTRL    (1 << 0)
#define M_MOD_SHIFT   (1 << 1)
#define M_MOD_ALT     (1 << 2)
/* Left Mouse Button */
#define M_MOD_LMB     (1 << 3)
/* Right Mouse Button */
#define M_MOD_RMB     (1 << 4)
/* Middle Mouse Button */
#define M_MOD_MMB     (1 << 5)
/* Forward Mouse Button (X1) */
#define M_MOD_FMB     (1 << 6)
/* Backward Mouse Button (X2) */
#define M_MOD_BMB     (1 << 7)

#define M_STATE_DOWN  (1 << 0)
#define M_STATE_UP    (1 << 1)

/* bindings */
int bindings_init();
XPLMCommandRef bindings_get(mbutton_t mbutton, int mod);

#ifdef IBM
int hook_wnd_proc();
int unhook_wnd_proc();
#elif APL
/**
 * Quartz supports up to 32 mouse buttons. The first 3 buttons
 * are specified using constants. Additional buttons are specified
 * in USB order using the integers 3 to 31.
 * XButton1 and XButton2 appear to be mapped to integer 3 and 4.
 */
#ifndef kCGMouseButtonForward
#define kCGMouseButtonForward 3
#endif
#ifndef kCGMouseButtonBackward
#define kCGMouseButtonBackward 4
#endif

int tap_events();
int untap_events();
#endif

#endif /* _PLUGIN_H_ */
