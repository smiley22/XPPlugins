/**
* MouseButtons - X-Plane 11 Plugin
*
* Enables the use of extra mouse buttons and allows the right mouse button
* and mouse wheel to be re-assigned to arbitrary commands.
*
* Copyright 2019 Torben Könke.
*/
#include "plugin.h"

typedef struct {
    mbutton_t mbutton;
    int mod;
    XPLMCommandRef cmd;
} mbinding_t;

#define MAX_NUM_BINDINGS 64

static mbinding_t bindings[MAX_NUM_BINDINGS];
static int num_bindings;

typedef struct {
    mbutton_t button;
    const char *name;
} mbutton_map_t;

static mbutton_map_t mbuttons[] = {
    { M_RIGHT,      "Mouse-Right"           },
    { M_MIDDLE,     "Mouse-Middle"          },
    { M_FORWARD,    "Mouse-Forward"         },
    { M_BACKWARD,   "Mouse-Backward"        },
    { M_W_FORWARD,  "Mouse-Wheel-Forward"   },
    { M_W_BACKWARD, "Mouse-Wheel-Backward"  },
    { M_W_LEFT,     "Mouse-Wheel-Left"      },
    { M_W_RIGHT,    "Mouse-Wheel-Right"     }
};
static int num_mbuttons = sizeof(mbuttons) / sizeof(mbuttons[0]);

static mbutton_t parse_mbutton(const char *s) {
    for (int i = 0; i < num_mbuttons; i++) {
        if (!_stricmp(s, mbuttons[i].name))
            return mbuttons[i].button;
    }
    return M_NONE;
}

static int parse_modifiers(const char *s) {
    int n = 0;
    char *p = strtok(s, "+");
    while (p) {
        if (!strcmp(p, "CTRL"))
            n |= MOD_CTRL;
        else if (!strcmp(p, "SHIFT"))
            n |= MOD_SHIFT;
        else if (!strcmp(p, "ALT"))
            n |= MOD_ALT;
        p = strtok(NULL, "+");
    }
    return n;
}

int bindings_init() {
    /* look for mouse.prf in current aircrafts directory first */
    /* otherwise take the one from plugin directory */
    return 1;
}

XPLMCommandRef bindings_get(mbutton_t mbutton, int mod) {
    for (int i = 0; i < num_bindings; i++) {
        if (bindings[i].mbutton == mbutton && bindings[i].mod == mod)
            return bindings[i].cmd;
    }
    return NULL;
}
