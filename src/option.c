#include <string.h>
#include <glib.h>
#include <config.h>

#include "option.h"
#include "keyfile.h"

option_t* option_new(void)
{
    option_t *opt;
    opt = g_slice_new(option_t);

    opt->mode = OPT_MODE_UNDEFINED;
    opt->backend = OPT_BACKEND_UNDEFINED;

    /* RTU */
    opt->id = -1;
    opt->device = NULL;
    opt->baud = -1;
    opt->parity = NULL;
    opt->data_bit = -1;
    opt->stop_bit = -1;

    /* TCP */
    opt->ip = NULL;
    opt->port = -1;

    opt->interval = -1;
    opt->socket_file = NULL;
    opt->ini_file = NULL;
    opt->daemon = FALSE;
    opt->pid_file = NULL;
    opt->verbose = FALSE;

    return opt;
}

void option_free(option_t *opt)
{
    g_free(opt->pid_file);
    g_free(opt->device);
    g_free(opt->parity);
    g_free(opt->ip);
    g_free(opt->socket_file);
    g_free(opt->ini_file);
    g_slice_free(option_t, opt);
}

void option_parse(option_t* opt, int argc, char **argv)
{
    int i;
    int rc;
    int argc_copy;
    gchar **argv_copy = NULL;
    char *mode_string = NULL;

    GOptionContext *context;
    GError *error = NULL;
    GOptionEntry entries[] = {
        {"mode", 'm', 0, G_OPTION_ARG_STRING, &mode_string,
         "Run in 'master' (default), 'slave', 'client' or 'server' mode", NULL},
        {"id", 0, 0, G_OPTION_ARG_INT, &(opt->id), "Slave ID in slave mode", "1"},
        {"device", 0, 0, G_OPTION_ARG_FILENAME, &(opt->device), "Device (eg. /dev/ttyUSB0)", NULL},
        {"baud", 'b', 0, G_OPTION_ARG_INT, &(opt->baud), "Baud", "115200"},
        {"parity", 'p', 0, G_OPTION_ARG_STRING, &(opt->parity),
         "Parity is 'O' for Odd, 'E' for Even or 'N' for None", "N"},
        {"databit", 'd', 0, G_OPTION_ARG_INT, &(opt->data_bit), "Bits of data (7 or 8)", "8"},
        {"stopbit", 's', 0, G_OPTION_ARG_INT, &(opt->stop_bit), "Bits of stop (1 or 2)", "1"},
        {"ip", 0, 0, G_OPTION_ARG_STRING, &(opt->ip), "IP address of server (eg. 127.0.0.1)", NULL},
        {"port", 0, 0, G_OPTION_ARG_INT, &(opt->port), "Port number of server (eg. 1502)", NULL},
        {"interval", 'i', 0, G_OPTION_ARG_INT, &(opt->interval), "Interval in seconds", NULL},
        {"socketfile", 0, 0, G_OPTION_ARG_FILENAME, &(opt->socket_file),
         "Local Unix socket file (eg. /tmp/mbsocket)", NULL},
        {"inifile", 'f', 0, G_OPTION_ARG_FILENAME, &(opt->ini_file), "Filename of config file (.ini-like)", NULL},
        {"daemon", 0, 0, G_OPTION_ARG_NONE, &(opt->daemon), "Run in daemon mode", NULL},
        {"pidfile", 0, 0, G_OPTION_ARG_FILENAME, &(opt->pid_file), "File to save thee PID", "PIDFILE"},
        {"verbose", 'v', 0, G_OPTION_ARG_NONE, &(opt->verbose), "Verbose mode", NULL},
        {NULL}
    };

    /* Make an extra copy of argv, since g_option_context_parse()
     * changes them and they are required on reload */
    argc_copy = argc;
    argv_copy = g_new(gchar*, argc + 1);
    for (i = 0; i <= argc; i++) {
        argv_copy[i] = argv[i];
    }

    context = g_option_context_new("- Modbus data collector");
    g_option_context_add_main_entries(context, entries, PACKAGE_NAME);
    /* Config file settings are overrided by command line options */
    rc = g_option_context_parse(context, &argc_copy, &argv_copy, &error);
    g_option_context_free(context);
    g_free(argv_copy);

    if (rc == FALSE) {
        g_error("option parsing failed: %s\n", error->message);
    }

    option_set_mode(opt, option_parse_mode(mode_string));

    g_free(mode_string);

    if (opt->ini_file == NULL) {
        /* Check existing config file (.ini-like config files) */
        if (g_file_test(MBT_LOCAL_INI_FILE, G_FILE_TEST_EXISTS)) {
            opt->ini_file = g_strdup(MBT_LOCAL_INI_FILE);
        } else if (g_file_test(MBT_ETC_INI_FILE, G_FILE_TEST_EXISTS)) {
            opt->ini_file = g_strdup(MBT_ETC_INI_FILE);
        }
    }
}

opt_mode_t option_parse_mode(char *mode_string)
{
    if (mode_string == NULL)
        return OPT_MODE_UNDEFINED;

    if (strcmp(mode_string, "master") == 0)
        return OPT_MODE_MASTER;

    if (strcmp(mode_string, "slave") == 0)
        return OPT_MODE_SLAVE;

    if (strcmp(mode_string, "client") == 0)
        return OPT_MODE_CLIENT;

    if (strcmp(mode_string, "server") == 0)
        return OPT_MODE_SERVER;

    g_error("invalid mode '%s'", mode_string);
    return OPT_MODE_UNKNOWN;
}

/* Set mode and backend accordingly */
void option_set_mode(option_t *opt, opt_mode_t mode)
{
    opt->mode = mode;
    if (mode == OPT_MODE_MASTER || mode == OPT_MODE_SLAVE) {
        opt->backend = OPT_BACKEND_RTU;
    } else if (mode == OPT_MODE_CLIENT || mode == OPT_MODE_SERVER) {
        opt->backend = OPT_BACKEND_TCP;
    }
}

int option_set_undefined(option_t *opt)
{
    /* Set the default values */
    if (opt->mode == OPT_MODE_UNDEFINED) {
        option_set_mode(opt, OPT_MODE_MASTER);
    }

    if (opt->backend == OPT_BACKEND_RTU) {
        if (opt->id == -1)
            opt->id = 1;

        if (opt->interval == -1)
            opt->interval = 10;

        if (opt->device == NULL)
            opt->device = g_strdup("/dev/ttyUSB0");

        if (opt->baud == -1)
            opt->baud = 115200;

        if (opt->parity == NULL)
            opt->parity = g_strdup("N");

        if (opt->data_bit == -1)
            opt->data_bit = 8;

        if (opt->stop_bit == -1)
            opt->stop_bit = 1;
    } else {
        if (opt->ip == NULL)
            opt->ip = g_strdup("127.0.0.1");

        if (opt->port == -1)
            opt->port = 502;
    }

    if (opt->socket_file == NULL)
        opt->socket_file = g_strdup("/tmp/mbsocket");

    if (opt->daemon && opt->pid_file == NULL)
        opt->pid_file = g_strdup("/var/run/mbcollect.pid");

    return 0;
}
