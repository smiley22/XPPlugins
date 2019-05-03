#include "util.h"

static FMOD_SYSTEM *fmod_sys;

typedef struct {
    char *name;
    XPLMDataRef dr;
} vol_tbl_t;

static vol_tbl_t vol_tbl[] = {
    { "sim/operation/sound/master_volume_ratio",    NULL },
    { "sim/operation/sound/exterior_volume_ratio",  NULL },
    { "sim/operation/sound/interior_volume_ratio",  NULL },
    { "sim/operation/sound/copilot_volume_ratio",   NULL },
    { "sim/operation/sound/radio_volume_ratio",     NULL },
    { "sim/operation/sound/enviro_volume_ratio",    NULL },
    { "sim/operation/sound/ui_volume_ratio",        NULL }
};
static const int num_vol_tbl = sizeof(vol_tbl) / sizeof(vol_tbl[0]);

int snd_init() {
    FMOD_RESULT err = FMOD_System_Create(&fmod_sys);
    if (err) {
        _log("snd_init: could not create fmod system (%i)", err);
        return 0;
    }
    err = FMOD_System_Init(fmod_sys, 32, FMOD_INIT_NORMAL, 0);
    if (err) {
        _log("snd_init: could not init fmod system (%i)", err);
        return 0;
    }
    /* init the volume datarefs */
    for (int i = 0; i < num_vol_tbl; i++) {
        vol_tbl[i].dr = XPLMFindDataRef(vol_tbl[i].name);
        if (!vol_tbl[i].dr) {
            _log("snd_init: could not find dataref (%s)", vol_tbl[i].name);
            return 0;
        }
    }
    _log("snd_init: initialized");
    return 1;
}

int snd_deinit() {
    if (fmod_sys) {
        FMOD_RESULT err = FMOD_System_Close(fmod_sys);
        if (err) {
            _log("snd_deinit: could not close fmod system (%i)", err);
            return 0;
        }
        err = FMOD_System_Release(fmod_sys);
        if (err)
            _log("snd_deinit: could not release fmod system (%i)", err);
    }
    fmod_sys = NULL;
    _log("snd_deinit: deinitialized");
    return 1;
}

snd_t snd_create(const char *file) {
    if (!fmod_sys) {
        _log("snd_create: sound system not initialized");
        return NULL;
    }
    FMOD_SOUND *s;
    FMOD_RESULT err = FMOD_System_CreateSound(fmod_sys, file, FMOD_DEFAULT,
        0, &s);
    if (err) {
        _log("snd_create: could not create sound '%s' (%i)", file, err);
        return NULL;
    }
    return s;
}

int snd_free(snd_t s) {
    if (!fmod_sys) {
        _log("snd_free: sound system not initialized");
        return 0;
    }
    if (!s)
        return 1;
    FMOD_RESULT err = FMOD_Sound_Release((FMOD_SOUND*)s);
    if (err) {
        _log("snd_free: could not release sound (%i)", err);
        return 0;
    }
    return 1;
}

int snd_play(snd_t s, snd_vol_t vol) {
    if (!fmod_sys) {
        _log("snd_play: sound system not initialized");
        return 0;
    }
    if (!s) {
        _log("snd_play: sound handle is NULL");
        return 0;
    }
    FMOD_CHANNEL *channel;
    FMOD_RESULT err = FMOD_System_PlaySound(fmod_sys, (FMOD_SOUND*)s, 0, 0,
        &channel);
    if (err) {
        _log("snd_play: could not play sound (%i)", err);
        return 0;
    }
    float ratio = XPLMGetDataf(vol_tbl[vol].dr);
    err = FMOD_Channel_SetVolume(channel, ratio);
    if (err)
        _log("snd_play: could not set channel volume (%i)", err);
    return 1;
}