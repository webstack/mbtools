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

void output_close(int s)
{
    close(s);
}

gboolean output_is_connected(int s)
{
    return s > 0 ? TRUE : FALSE;
}

/* 7 arguments :-\ */
int output_write(int s, int slave_id, int addr, int nb_reg, char *type, uint16_t *tab_reg, gboolean verbose)
{
    int rc;
    int i;
    int j;
    int nb;
    char *output = NULL;
    /* A string for each value */
    char **out_array = NULL;
    gboolean is_integer;
    gboolean is_swapped = FALSE;

    if (type == NULL || strcmp(type, "int") == 0) {
        /* Integer */
        nb = nb_reg;
        is_integer = TRUE;
    } else {
        /* Float */
        nb = nb_reg / 2;
        is_integer = FALSE;
        is_swapped = (strcmp(type, "floatlsb") == 0);
    }

    out_array = g_new(char*, nb + 1);

    for (i = 0, j = 0; i < nb; i++) {
        if (slave_id > 0) {
            /* Type is only handled in this mode (master) */
            if (is_integer) {
                out_array[i] = g_strdup_printf("mb_%d_%d %d", slave_id, addr + i, tab_reg[i]);
            } else {
                float value;

                if (is_swapped) {
                    value = modbus_get_float_swapped(tab_reg + j);
                } else {
                    value = modbus_get_float(tab_reg + j);
                }
                j += 2;
                out_array[i] = g_strdup_printf("mb_%d_%d %f", slave_id, addr + i, value);
            }
        } else {
            out_array[i] = g_strdup_printf("mb_%d %d", addr + i, tab_reg[i]);
        }
    }

    /* Set final marker */
    out_array[i] = NULL;

    output = g_strjoinv("|", out_array);
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
