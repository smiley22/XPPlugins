#ifndef _UTIL_H_
#define _UTIL_H_

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
int get_acft_dir(char *buf, int size);
int get_data_path(const char *file, char *buf, int size);

/* snd */
typedef int snd_t;

#endif /* _UTIL_H_ */