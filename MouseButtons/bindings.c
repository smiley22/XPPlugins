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
        else
            _log("Unknown modifier key %s", p);
        p = strtok(NULL, "+");
    }
    return n;
}

int bindings_init() {
    /* Look for a mouse.prf for the aircraft we're flying first. */
    char name[256], path[512];
    XPLMGetNthAircraftModel(0, name, path);

    /* Otherwise we'll look for mouse.prf in plugin directory. */


    //
    FILE *fp = fopen(path, "r");
    if (!fp) {
        _log("could not load mouse bindings from '%s'", path);
        return 0;
    }
    char line[128], token[64];
    while (fgets(line, sizeof(line), fp)) {
        if (!strcmp("I\n", line) || !strcmp("1005 Version\n", line))
            continue;
        char *p = read_token(line, token, sizeof(token));
        if (token[0] == '#')
            continue;
        mbinding_t *pb = &bindings[num_bindings];
        if (!(pb->cmd = parse_mbutton(token))) {
            _log("unknown mouse button identifier: %s", token);
            continue;
        }
        p = read_token(p, token, sizeof(token));
        pb->mod = parse_modifiers(token);
        p = read_token(p, token, sizeof(token));
        pb->cmd = XPLMCreateCommand(token, "");
        num_bindings++;
    }
    fclose(fp);
    return num_bindings;
}

XPLMCommandRef bindings_get(mbutton_t mbutton, int mod) {
    for (int i = 0; i < num_bindings; i++) {
        if (bindings[i].mbutton == mbutton && bindings[i].mod == mod)
            return bindings[i].cmd;
    }
    return NULL;
}
