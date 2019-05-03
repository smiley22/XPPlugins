#ifndef _UTIL_H_
#define _UTIL_H_

#define XPLM200
#define XPLM210
#define XPLM300
#ifdef _WIN32
#ifndef _WIN64
#error Only 64-Bit platforms are supported.
#endif
#define _CRT_SECURE_NO_WARNINGS
#define IBM 1
#pragma comment(lib, "../XP/Libs/XPLM_64.lib")
#endif /* _WIN32 */

#include "../XP/XPLMUtilities.h"
#include "../XP/XPLMDataAccess.h"
#include "../XP/XPLMPlugin.h"
#include "../FMOD/fmod.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

#ifndef MAX_PATH
#define MAX_PATH 512
#endif
#ifndef MAX_NAME
#define MAX_NAME 256
#endif

/* ini */
int ini_geti(const char *name, int def);
int ini_seti(const char *name, int val);
int ini_setf(const char *name, float val);
float ini_getf(const char *name, float def);
void ini_gets(const char *name, char *buf, int size, const char *def);
int ini_sets(const char *name, const char *val);

/* log */
void _log(const char *fmt, ...);
void _debug(const char *fmt, ...);

/* path */
int get_plugin_dir(char *buf, int size);
int get_plugin_name(char *buf, int size);
int get_acft_dir(char *buf, int size);
int get_data_path(const char *file, char *buf, int size);

/* snd */
typedef void* snd_t;
typedef enum {
    SND_VOL_MASTER,
    SND_VOL_EXTERIOR,
    SND_VOL_INTERIOR,
    SND_VOL_COPILOT,
    SND_VOL_RADIO,
    SND_VOL_ENV,
    SND_VOL_UI
} snd_vol_t;
int snd_init();
int snd_deinit();
snd_t snd_create(const char *file);
int snd_free(snd_t s);
int snd_play(snd_t s, snd_vol_t vol);

/* cmd */
XPLMCommandRef cmd_create(const char *name, const char *desc,
    XPLMCommandCallback_f cb, void *data);
void cmd_free(XPLMCommandRef *cmd);

/* time */
long long get_time_ms();

#endif /* _UTIL_H_ */