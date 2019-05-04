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

#define V1_CALLOUT      1
#define V1_SOUND        "a320_v_one.wav"
#define A320U_V1_SPEED  "Aircraft.TakeoffDecision"
#define A320U_AIRSPEED  "Aircraft.AirSpeed"

static int v1_id;
static int airspeed_id;
static XPLMFlightLoopID loop_id;
static snd_t v1_sound;

void v1_init() {
    /* If it's not enabled, we don't need to set up anything in the
       first place. */
    if (!ini_geti("v1_callout", V1_CALLOUT))
        return;
    v1_id = ff_get_id(A320U_V1_SPEED);
    if (v1_id < 0) {
        _log("init fail: could not find A320U object %s", A320U_V1_SPEED);
        return;
    }
    airspeed_id = ff_get_id(A320U_AIRSPEED);
    if (airspeed_id < 0) {
        _log("init fail: could not find A320U object %s", A320U_AIRSPEED);
        return;
    }
    char path[MAX_PATH];
    get_data_path(V1_SOUND, path, MAX_PATH);
    if (!(v1_sound = snd_create(path))) {
        _log("init fail: could not create sound (%s)", path);
        return;
    }
    /* Register and schedule flightloop. */
    XPLMCreateFlightLoop_t params = {
        .structSize = sizeof(XPLMCreateFlightLoop_t),
        .phase = xplm_FlightLoop_Phase_BeforeFlightModel,
        .refcon = NULL,
        .callbackFunc = v1_loop_cb
    };
    loop_id = XPLMCreateFlightLoop(&params);
    XPLMScheduleFlightLoop(loop_id, -10.0f, 0);
    _log("initialized v1 module");
}

float v1_loop_cb(float last_call, float last_loop, int count, void *data) {
    float ias = ff_get_float(airspeed_id);
    if (ias > 40) {
        float v1 = ff_get_float(v1_id);
        if (ias >= v1) {
            snd_play(v1_sound, SND_VOL_INTERIOR);
            /* Don't need to call us back anymore after this. */
            return 0;
        }
    }
    /* It's probably enough to call us back every once in a while. */
    return -10.0f;
}

void v1_deinit() {
    if (loop_id)
        XPLMDestroyFlightLoop(loop_id);
    loop_id = NULL;
    if (v1_sound)
        snd_free(v1_sound);
    v1_sound = NULL;
    _log("deinitialized v1 module");
}
