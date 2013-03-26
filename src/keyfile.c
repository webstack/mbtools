#include <string.h>
#include <glib.h>
#include "keyfile.h"

/* Parse config file (.ini-like file) */
client_t* keyfile_parse(option_t *opt, int *nb_client)
{
    int i;
    GKeyFile *key_file = NULL;
    client_t* clients = NULL;

    *nb_client = 0;

    if (opt->ini_file == NULL) {
        return NULL;
    }

    if (opt->verbose)
        g_print("Loading of %s config file.\n", opt->ini_file);

    key_file = g_key_file_new();

    if (!g_key_file_load_from_file(key_file, opt->ini_file, G_KEY_FILE_NONE, NULL)) {
        g_key_file_free(key_file);
        return NULL;
    }

    if (opt->listen == FALSE) {
        opt->listen = g_key_file_get_boolean(key_file, "settings", "listen", NULL);
    }

    if (opt->id == -1) {
        opt->id = g_key_file_get_integer(key_file, "settings", "id", NULL);
        if (opt->id == 0)
            opt->id = -1;
    }

    if (opt->device == NULL)
        opt->device = g_key_file_get_string(key_file, "settings", "device", NULL);

    if (opt->baud == -1) {
        opt->baud = g_key_file_get_integer(key_file, "settings", "baud", NULL);
        if (opt->baud == 0)
            opt->baud = -1;
    }

    if (opt->parity == NULL)
        opt->parity = g_key_file_get_string(key_file, "settings", "parity", NULL);

    if (opt->data_bit == -1) {
        opt->data_bit = g_key_file_get_integer(key_file, "settings", "databit", NULL);
        if (opt->data_bit == 0)
            opt->data_bit = -1;
    }

    if (opt->stop_bit == -1) {
        opt->stop_bit = g_key_file_get_integer(key_file, "settings", "stopbit", NULL);
        if (opt->stop_bit == 0)
            opt->stop_bit = -1;
    }

    if (opt->socket_file == NULL)
        opt->socket_file = g_key_file_get_string(key_file, "settings", "socketfile", NULL);

    if (!opt->listen) {
        gchar** groups = g_key_file_get_groups(key_file, NULL);

        /* Count [slave] sections to allocate clients */
        i = 0;
        while (groups[i] != NULL) {
            if ((strncmp(groups[i++], "slave", 5) == 0)) {
                (*nb_client)++;
            }
        }

        if ((*nb_client) > 0) {
            int c;

            /* Allocate clients */
            clients = g_slice_alloc(sizeof(client_t) * (*nb_client));

            i = 0;
            c = 0;
            while (groups[i] != NULL) {
                if ((strncmp(groups[i], "slave", 5) == 0)) {
                    gsize n_address;
                    gsize n_length;

                    /* Returns 0 if not found */
                    clients[c].id = g_key_file_get_integer(key_file, groups[i], "id", NULL);
                    clients[c].addresses = g_key_file_get_integer_list(key_file, groups[i], "addresses",
                                                                       &n_address, NULL);
                    clients[c].lengths = g_key_file_get_integer_list(key_file, groups[i], "lengths",
                                                                     &n_length, NULL);
                    /* Minimum of both list to be sure each address is associated to a length */
                    clients[c].n = MIN(n_address, n_length);

                    if (opt->verbose) {
                        int n;

                        g_print("%s id: %d\n", groups[i], clients[c].id);
                        for (n=0; n < clients[c].n; n++) {
                            g_print("Address %d => %d values\n",
                                    clients[c].addresses[n], clients[c].lengths[n]);
                        }
                    }
                    c++;
                }
                i++;
            }
        }
        g_strfreev(groups);
    }

    g_key_file_free(key_file);

    return clients;
}

void keyfile_client_free(int nb_client, client_t* clients)
{
    int i;

    if (nb_client > 0) {
        for (i=0; i < nb_client; i++) {
            g_free(clients[i].addresses);
            g_free(clients[i].lengths);
        }
        g_slice_free1(sizeof(client_t) * nb_client, clients);
    }
}
