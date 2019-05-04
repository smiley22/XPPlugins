/**
 * A320UE - X-Plane 11 Plugin
 *
 * A plugin for the FlightFactor A320 Ultimate that adds a couple of new
 * commands for operating the thrust levers more comfortably as well as a
 * bunch of other little workarounds and/or features.
 *
 * Copyright 2019 Torben Könke.
 */
#include "plugin.h"

static int ff_plugin_id = XPLM_NO_PLUGIN_ID;
static int ff_loop_reg = 0;
static ff_api_t ff_api = { 0 };
static ff_init_done_cb ff_on_done_init = NULL;

int ff_init(ff_init_done_cb cb) {
    ff_on_done_init = cb;
    ff_plugin_id = XPLMFindPluginBySignature(XPLM_FF_SIGNATURE);
    if (ff_plugin_id == XPLM_NO_PLUGIN_ID) {
        _log("Could not find FF A320 plugin (%s)", XPLM_FF_SIGNATURE);
        return 0;
    } else {
        /* Try to get reference to api now. */
        XPLMSendMessageToPlugin(ff_plugin_id, XPLM_FF_MSG_GET_SHARED_INTERFACE,
            &ff_api);
        if (ff_api.ValuesCount == NULL) {
            ff_loop_reg = 1;
            XPLMRegisterFlightLoopCallback(ff_loop_cb, 1, NULL);
        } else {
            ff_on_done_init();
        }
    }
    return 1;
}

float ff_loop_cb(float last_call, float last_loop, int count, void *data) {
    if (ff_plugin_id < 0)
        return 0;
    XPLMSendMessageToPlugin(ff_plugin_id, XPLM_FF_MSG_GET_SHARED_INTERFACE,
        &ff_api);
    /* Keep trying until FF A320 has finished loading and returns a valid
       data... */
    if (ff_api.ValuesCount == NULL)
        return 1;
    /* Once we got the stupid FF interface we can continue with init'ing. */
    ff_on_done_init();
    /* No need to schedule loop again once we're here. */
    return 0;
}

void ff_deinit() {
    if (ff_loop_reg)
        XPLMUnregisterFlightLoopCallback(ff_loop_cb, NULL);
    ff_loop_reg = 0;
    ff_plugin_id = XPLM_NO_PLUGIN_ID;
    memset(&ff_api, 0, sizeof(ff_api_t));
    ff_on_done_init = NULL;
}

int ff_get_id(const char *name) {
    if (!ff_api.ValueIdByName)
        return -1;
    return ff_api.ValueIdByName(name);
}

int ff_get_int(int id) {
    if (!ff_api.ValueGet)
        return 0;
    int retval;
    ff_api.ValueGet(id, &retval);
    return retval;
}

void ff_set_int(int id, int val) {
    if (!ff_api.ValueSet)
        return;
    ff_api.ValueSet(id, &val);
}

float ff_get_float(int id) {
    if (!ff_api.ValueGet)
        return 0;
    float retval;
    ff_api.ValueGet(id, &retval);
    return retval;
}

void ff_set_float(int id, float val) {
    if (!ff_api.ValueSet)
        return;
    ff_api.ValueSet(id, &val);
}
