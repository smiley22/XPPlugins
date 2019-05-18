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
    { M_LEFT,       "Mouse-Left"            },
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

typedef struct {
    int flag;
    const char *name;
} flag_map_t;

static flag_map_t flags[] = {
    { M_MOD_CTRL,   "CTRL"  },
    { M_MOD_SHIFT,  "SHIFT" },
    { M_MOD_ALT,    "ALT"   },
    { M_MOD_LMB,    "LMB"   },
    { M_MOD_RMB,    "RMB"   },
    { M_MOD_MMB,    "MMB"   },
    { M_MOD_FMB,    "FMB"   },
    { M_MOD_BMB,    "BMB"   }
};
static int num_flags = sizeof(flags) / sizeof(flags[0]);

static int parse_modifiers(char *s) {
    int n = 0;
    char *p = strtok(s, "+");
    while (p) {
        for (int i = 0; i < num_flags; i++) {
            if (!strcmp(p, flags[i].name)) {
                n |= flags[i].flag;
                break;
            }
        }
        p = strtok(NULL, "+");
    }
    return n;
}

static char *read_token(char *p, char *buf, int size) {
    /* Skip whitespaces, if any. */
    while (*p == ' ' || *p == '\t')
        p++;
    int i = 0;
    while (*p && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n'
        && i < (size - 1)) {
        buf[i++] = *p++;
    }
    buf[i] = '\0';
    return p;
}

int bindings_init() {
    /* Look for a mouse.prf for the aircraft we're flying first. */
    char name[256], path[512], buf[512];
    XPLMGetNthAircraftModel(0, name, buf);
    get_plugin_dir(path, sizeof(path));
    char *p = strrchr(name, '.');
    if (p)
        strcpy(p + 1, "prf");
    sprintf(buf, "%s/%s", path, name);
    FILE *fp = fopen(buf, "r");
    if (!fp) {
        /* Otherwise probe for mouse.prf in plugin directory. */
        _log("could not load mouse bindings for aircraft from '%s'", path);
        sprintf(path, "%s/mouse.prf", path);
        if (!(fp = fopen(path, "r"))) {
            _log("could not load mouse bindings from '%s'", path);
            return 0;
        }
    }
    num_bindings = 0;
    char line[128], token[64];
    while (fgets(line, sizeof(line), fp)) {
        if (!strcmp("I\n", line) || !strcmp("1005 Version\n", line))
            continue;
        char *p = read_token(line, token, sizeof(token));
        if (token[0] == '#')
            continue;
        mbinding_t *pb = &bindings[num_bindings];
        if (!(pb->mbutton = parse_mbutton(token))) {
            _log("unknown mouse button identifier: %s", token);
            continue;
        }
        p = read_token(p, token, sizeof(token));
        pb->mod = parse_modifiers(token);
        p = read_token(p, token, sizeof(token));
        pb->cmd = XPLMFindCommand(token);
        num_bindings++;
        _debug("binding  mbutton = %i | mod = %x | cmd = %s",
            pb->mbutton, pb->mod, token);
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
