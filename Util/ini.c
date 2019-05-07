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

static char ini_path[MAX_PATH];

#define INI_FILE_NAME "settings.ini"
#define INI_SECT_NAME "settings"

static char *ini_get_path() {
    if (ini_path[0])
        return ini_path;
    if (!get_plugin_dir(ini_path, MAX_PATH)) {
        _log("ini_get_path: could not get plugin dir");
        return NULL;
    }
    return strcat(ini_path, INI_FILE_NAME);
}

int ini_geti(const char *name, int def) {
    char *f = ini_get_path();
#ifdef IBM
    return GetPrivateProfileIntA(INI_SECT_NAME, name, def, f);
#endif
}

int ini_seti(const char *name, int val) {
    char *f = ini_get_path();
#ifdef IBM
    char buf[32];
    snprintf(buf, sizeof(buf), "%i", val);
    return WritePrivateProfileStringA(INI_SECT_NAME, name, buf, f);
#endif
}

float ini_getf(const char *name, float def) {
    char *f = ini_get_path();
#ifdef IBM
    char buf[32];
    ini_gets(name, buf, 32, "nan");
    if (!strcmp(name, "nan"))
        return def;
    return atof(buf);
#endif
}

int ini_setf(const char *name, float val) {

}

void ini_gets(const char *name, char *buf, int size, const char *def) {
    char *f = ini_get_path();
#ifdef IBM
    GetPrivateProfileStringA(INI_SECT_NAME, name, def, buf, size, f);
#endif
}

int ini_sets(const char *name, const char *val) {
    char *f = ini_get_path();
#ifdef IBM
    return WritePrivateProfileStringA(INI_SECT_NAME, name, val, f);
#endif
}