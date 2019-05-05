/**
 * Utility library for X-Plane 11 Plugins.
 *
 * Static library containing common functionality for stuff like logging and
 * dealing with configuration files. Linked against by most plugins in the
 * solution.
 *
 * Copyright 2019 Torben Könke.
 */
#include "util.h"

XPLMCommandRef cmd_create(const char *name, const char *desc,
    XPLMCommandCallback_f cb, void *data) {
    XPLMCommandRef cmd = XPLMCreateCommand(name, desc);
    XPLMRegisterCommandHandler(cmd, cb, 0, data);
    return cmd;

}

void cmd_free(XPLMCommandRef *cmd, XPLMCommandCallback_f cb, void *data) {
    /* You actually have to pass in the cb and data pointer to be able to
       unregister the handler... */
    XPLMUnregisterCommandHandler(cmd, cb, 0, data);
}
