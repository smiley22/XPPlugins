#include "util.h"

int get_plugin_dir(char *buf, int size) {
    XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);
    XPLMGetPluginInfo(XPLMGetMyID(), NULL, buf, NULL, NULL);
    /* skip plugin filename */
    char *p = strrchr(buf, '/');
    if (!p)
        return 0;
    *p = 0;
    /* skip /64 directory */
    p = strrchr(buf, '/');
    if (!p)
        return 0;
    *(p + 1) = 0;
    return 1;
}

int get_acft_dir(char *buf, int size) {

}

int get_data_path(const char *file, char *buf, int size) {
    if (!get_plugin_dir(buf, size))
        return 0;
    strncat(buf, "data/", size);
    strncat(buf, file, size);
    return 1;
}