#ifndef _OPTION_H_
#define _OPTION_H_

#include <glib.h>

/* This mode could be extended with more options */
typedef enum {
    OPT_MODE_UNDEFINED,
    OPT_MODE_MASTER,
    OPT_MODE_SLAVE,
    OPT_MODE_CLIENT,
    OPT_MODE_SERVER
} opt_mode_t;

typedef enum {
    OPT_BACKEND_UNDEFINED,
    OPT_BACKEND_RTU,
    OPT_BACKEND_TCP
} opt_backend_t;

typedef struct {
    opt_mode_t mode;
    opt_backend_t backend;
    int id;
    /* RTU */
    char *device;
    int baud;
    char *parity;
    int data_bit;
    int stop_bit;
    /* TCP */
    char *ip;
    int port;
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
void option_parse(option_t *opt, int argc, char **argv);
opt_mode_t option_parse_mode(char *mode_string);
void option_set_mode(option_t *opt, opt_mode_t mode);
int option_set_undefined(option_t *opt);

#endif /* _OPTION_H_ */
