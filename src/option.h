#ifndef _OPTION_H_
#define _OPTION_H_

#include <glib.h>

typedef struct {
    /* RTU */
    char *device;
    int baud;
    char *parity;
    int data_bit;
    int stop_bit;
    /* Recorder */
    int interval;
    char *socket_file;
    /* System */
    gboolean daemon;
    char *pid_file;
    gboolean verbose;
    char *ini_file;
} option_t;

option_t* option_new(void);
void option_free(option_t *opt);
void option_parse(option_t *opt, gint *argc, gchar ***argv);
int option_set_undefined(option_t *opt);

#endif /* _OPTION_H_ */
