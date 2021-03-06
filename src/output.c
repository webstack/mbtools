#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <glib.h>
#include <modbus.h>

#include "output.h"

int output_connect(char* socket_file, gboolean verbose)
{
    int s, len;
    struct sockaddr_un remote;

    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return -1;
    }

    if (verbose)
        g_print("Trying to connect to recorder socket %s\n", socket_file);

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, socket_file);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(s, (struct sockaddr *)&remote, len) == -1) {
        perror("connect");
        return -1;
    }

    if (verbose)
        g_print("Connected to recorder\n");

    return s;
}

void output_close(int *s)
{
    if (*s != -1) {
        close(*s);
        *s = -1;
    }
}

gboolean output_is_connected(int s)
{
    return s > 0 ? TRUE : FALSE;
}

int output_write(int s, option_t *opt, char *name, int addr, int nb_reg, char *type, uint16_t *tab_reg, gboolean verbose)
{
    int rc;
    int i, j;
    int nb;
    char *output = NULL;
    /* A string for each value */
    char **out_array = NULL;
    gboolean is_integer;
    gboolean is_lsb = FALSE;
    gboolean is_server;

    is_server = (opt->mode == OPT_MODE_SLAVE || opt->mode == OPT_MODE_SERVER);

    if (is_server || strcmp(type, "int") == 0) {
        /* Integer */
        is_integer = TRUE;
        nb = nb_reg;
    } else {
        /* Float */
        is_integer = FALSE;
        nb = nb_reg / 2;
        is_lsb = (strcmp(type, "floatlsb") == 0);
    }

    /* nb and NULL marker */
    out_array = g_new(char*, nb + 1);

    for (i = 0, j = 0; i < nb; i++) {
        if (!is_server) {
            /* Type is only handled in this mode */
            if (is_integer) {
                out_array[i] = g_strdup_printf("mb_%s_%d %d|", name, addr + i, tab_reg[i]);
            } else {
                float value;

                if (is_lsb) {
                    value = modbus_get_float_dcba(tab_reg + j);
                } else {
                    value = modbus_get_float(tab_reg + j);
                }
                out_array[i] = g_strdup_printf("mb_%s_%d %f|", name, addr + j, value);
                j += 2;
            }
        } else {
            out_array[i] = g_strdup_printf("mb_%d %d|", addr + i, tab_reg[i]);
        }
    }

    /* Set final marker of the array */
    out_array[i] = NULL;
    output = g_strjoinv(NULL, out_array);

    /* Replace final '|' by '\n' */
    output[strlen(output) - 1] = '\n';

    if (verbose)
        g_print("%s\n", output);

    rc = send(s, output, strlen(output), MSG_NOSIGNAL);

    for (i = 0; i < nb; i++) {
        g_free(out_array[i]);
    }
    g_free(out_array);
    g_free(output);

    return rc;
}
