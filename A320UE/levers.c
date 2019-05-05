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

typedef struct {
    char *name;
    char *desc;
    XPLMCommandCallback_f cb;
    void *ref;
    XPLMCommandRef cmd;
} lever_cmd_t;

static lever_cmd_t lever_cmds[] = {
    { "A320UE/ThrustDetentUp", "Thrust levers into next detent position",
      levers_next_detent, (void*)1, NULL },
    { "A320UE/ThrustDetentDown", "Thrust levers into previous detent position",
      levers_next_detent, 0, NULL },
    { "A320UE/ThrustStepUp", "Thrust up a notch",
      levers_next_step, (void*)1, NULL },
    { "A320UE/ThrustStepDown", "Thrust down a notch",
      levers_next_step, 0, NULL }
};

static int lever_id;
static XPLMDataRef dr_throttle;
static snd_t click_sound;
static int thrust_inc_delay;
static int thrust_inc_speed;
static int thrust_detent_stop;
static int thrust_show_hints;
static int draw_cb_registered;

#define THRUST_INC_SPEED     6 /* per second */
#define THRUST_INC_DELAY   500 /* ms */
#define THRUST_DETENT_STOP   0
#define THRUST_SHOW_HINTS    1

/**
 * A320U object that hold the position of engine lever one. Alas it can only
 * be read but not written.
 */
#define ENGINE_LEVER_ONE "Aircraft.Cockpit.Pedestal.EngineLever1"
 /**
  * Unfortunately this undocumented data-ref appears to be the only way to
  * manipulate the thrust-levers of the A320U.
  */
#define DATAREF_THROTTLE "a320/throttleComm"

void levers_init() {
    lever_id = ff_get_id(ENGINE_LEVER_ONE);
    if (lever_id < 0) {
        _log("init fail: could not find A320U object %s", ENGINE_LEVER_ONE);
        return;
    }
    dr_throttle = XPLMFindDataRef(DATAREF_THROTTLE);
    if (NULL == dr_throttle) {
        _log("init fail: could not find data-ref %s", DATAREF_THROTTLE);
        return;
    }
    char path[MAX_PATH];
    get_data_path("a320_detent_click.wav", path, MAX_PATH);
    if (!(click_sound = snd_create(path))) {
        _log("init warn: could not create sound (%s)", path);
    }
    /* create and install command handlers */
    for (int i = 0; i < sizeof(lever_cmds) / sizeof(lever_cmds[0]); i++) {
        lever_cmds[i].cmd = cmd_create(
            lever_cmds[i].name,
            lever_cmds[i].desc,
            lever_cmds[i].cb,
            lever_cmds[i].ref
        );
    }
    _log("registered A320UE lever commands");
    /* get a bunch of config settings */
    thrust_inc_delay = ini_geti("thrust_inc_delay", THRUST_INC_DELAY);
    thrust_inc_speed = ini_geti("thrust_inc_speed", THRUST_INC_SPEED);
    thrust_detent_stop = ini_geti("thrust_detent_stop", THRUST_DETENT_STOP);
    thrust_show_hints = ini_geti("thrust_show_hints", THRUST_SHOW_HINTS);
}

void levers_deinit() {
    /* uninstall command handlers */
    for (int i = 0; i < sizeof(lever_cmds) / sizeof(lever_cmds[0]); i++) {
        cmd_free(lever_cmds[i].cmd);
    }
    /* need to free sound memory */
    if (click_sound)
        snd_free(click_sound);
    click_sound = NULL;
    _log("unregistered A320UE lever commands");
}

typedef struct {
    float lever; /* value of A320U lever object */
    float pos; /* value for throttle_comm data-ref */
    char *name;
    int sound; /* true to play click sound */
} lever_detent_t;

static lever_detent_t lever_detents[] = {
    /* detent clicks are missing for some detents in A320U for some
       reason.*/
    {  0.0f,    -1.0f,    "Full Rev",    1 },
    { 14.0f,    -0.1f,    "Rev Idle",    1 },
    { 20.0f,     0.0f,    "Idle",        1 },
    { 45.0f,     0.6f,    "Climb",       0 },
    { 55.0f,     0.8f,    "Flex",        0 },
    { 65.0f,     1.0f,    "TOGA",        0 }
};

static int num_lever_detents = sizeof(lever_detents) / sizeof(lever_detents[0]);

static void levers_set_pos(float pos, const char *message, int sound) {
    XPLMSetDataf(dr_throttle, pos);
    if(sound)
        snd_play(click_sound, SND_VOL_INTERIOR);
    if (message && thrust_show_hints)
        levers_draw_string(message);
}

int levers_next_detent(XPLMCommandRef cmd, XPLMCommandPhase phase, void *ref) {
    if (phase != xplm_CommandBegin)
        return 0;
    float lever_pos = ff_get_float(lever_id);
    /* move forward into next detent position */
    if (ref) {
        for (int i = 0; i < num_lever_detents; i++) {
            if (lever_detents[i].lever > lever_pos) {
                /* set levers */
                levers_set_pos(
                    lever_detents[i].pos,
                    lever_detents[i].name,
                    lever_detents[i].sound
                );
                return 1;
            }
        }
    } else {
        for (int i = num_lever_detents - 1; i >= 0; i--) {
            if (lever_detents[i].lever < lever_pos) {
                /* set levers */
                levers_set_pos(
                    lever_detents[i].pos,
                    lever_detents[i].name,
                    lever_detents[i].sound
                );
                return 1;
            }
        }
    }
    return 0;
}

static int levers_in_detent(float pos) {
    const float threshold = 0.05f;
    for (int i = 0; i < num_lever_detents; i++) {
        double dist = fabs(lever_detents[i].pos - pos);
        if (dist <= threshold)
            return i;
    }
    return -1;
}

static long long step_start_time;
static long long step_last_time;
static int  step_stop;

int levers_next_step(XPLMCommandRef cmd, XPLMCommandPhase phase, void *ref) {
    float pos = XPLMGetDataf(dr_throttle);
    float amt = 0.05f; /* initial amount */
    long long now = get_time_ms();
    int before = levers_in_detent(pos);
    switch (phase) {
    case xplm_CommandBegin:
        step_stop = 0;
        step_start_time = now;
        break;
    case xplm_CommandContinue:
        if ((step_start_time + thrust_inc_delay) > now) {
            step_last_time = now;
            return 0;
        }
        amt = ((now - step_last_time) / 1000.0f) * (thrust_inc_speed / 10.0f);
        step_last_time = now;
        break;
    default:
        return 1;
    }
    if (step_stop)
        return 1;
    if (amt > 0) {
        float newpos = pos + (ref ? 1 : -1) * amt;
        if (newpos >= -1.0 && newpos <= 1.0) {
            int after = levers_in_detent(newpos);
            if (before != after && after != -1) {
                levers_set_pos(
                    lever_detents[after].pos,
                    lever_detents[after].name,
                    lever_detents[after].sound
                );
                if (thrust_detent_stop)
                    step_stop = 1;
            }
            else {
                char buf[128];
                sprintf(buf, "Setting thrust to %02.2f", newpos);
                levers_set_pos(newpos, buf, 0);
            }
        }
    }
    return 1;
}

static char levers_message[128];
static long long levers_message_timeout;
static int screen_height;
static float cyan[] = { 0, 1.0f, 1.0f };
int draw_cb(XPLMDrawingPhase phase, int before, void *ref) {
    /* show a text indication in top left corner of screen */
    if (levers_message_timeout < get_time_ms()) {
        /* if not drawing anything might as well unregister the callback */
        XPLMUnregisterDrawCallback(draw_cb, xplm_Phase_Window, 0, NULL);
        draw_cb_registered = 0;
        return 1;
    }
    XPLMDrawString(cyan, 20, screen_height - 50, (char*)levers_message, NULL,
        xplmFont_Proportional);
    return 1;
}

void levers_draw_string(const char *s) {
    if (!draw_cb_registered) {
        XPLMGetScreenSize(NULL, &screen_height);
        draw_cb_registered = XPLMRegisterDrawCallback(draw_cb, xplm_Phase_Window, 0, NULL);
    }
    strncpy(levers_message, s, sizeof(levers_message));
    levers_message_timeout = get_time_ms() + 3000;
}
