#include <string.h>
#include <glib.h>
#include "keyfile.h"

static gboolean keyfile_set_integer(GKeyFile *key_file, const gchar *group_name, const gchar *key, int *value);

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

    if (opt->listen == FALSE)
        opt->listen = g_key_file_get_boolean(key_file, "settings", "listen", NULL);

    keyfile_set_integer(key_file, "settings", "id", &(opt->id));

    if (opt->device == NULL)
        opt->device = g_key_file_get_string(key_file, "settings", "device", NULL);

    keyfile_set_integer(key_file, "settings", "baud", &(opt->baud));

    if (opt->parity == NULL)
        opt->parity = g_key_file_get_string(key_file, "settings", "parity", NULL);

    keyfile_set_integer(key_file, "settings", "databit", &(opt->data_bit));
    keyfile_set_integer(key_file, "settings", "stopbit", &(opt->stop_bit));
    keyfile_set_integer(key_file, "settings", "interval", &(opt->interval));

    if (opt->socket_file == NULL)
        opt->socket_file = g_key_file_get_string(key_file, "settings", "socketfile", NULL);

    if (opt->daemon == FALSE)
        opt->daemon = g_key_file_get_boolean(key_file, "settings", "daemon", NULL);

    if (opt->pid_file == NULL)
        opt->pid_file = g_key_file_get_string(key_file, "settings", "pidfile", NULL);

    if (opt->verbose == FALSE)
        opt->verbose = g_key_file_get_boolean(key_file, "settings", "verbose", NULL);

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


/* Set value from config file if defined and not already set by command line.
   Returns TRUE when value is modified, FALSE otherwise.
 */
static gboolean keyfile_set_integer(GKeyFile *key_file, const gchar *group_name, const gchar *key, int *value)
{
    /* Don't override command line option */
    if (*value == -1) {
        *value = g_key_file_get_integer(key_file, group_name, key, NULL);
        if (*value == 0) {
            *value = -1;
            return FALSE;
        } else {
            return TRUE;
        }

    }
    return FALSE;
}
