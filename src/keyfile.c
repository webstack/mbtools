#include <string.h>
#include <glib.h>

#include "keyfile.h"

static gboolean keyfile_set_integer(GKeyFile *key_file, const gchar *group_name, const gchar *key, int *value);

/* Parse config file (.ini-like file) */
server_t* keyfile_parse(option_t *opt, int *nb_server)
{
    int i;
    GKeyFile *key_file = NULL;
    server_t* servers = NULL;

    *nb_server = 0;

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

    if (opt->mode == OPT_MODE_UNDEFINED) {
        char *mode_string = g_key_file_get_string(key_file, "settings", "mode", NULL);
        option_set_mode(opt, option_parse_mode(mode_string));
        g_free(mode_string);
    }

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

    if (opt->mode == OPT_MODE_MASTER) {
        gchar** groups = g_key_file_get_groups(key_file, NULL);

        /* Count [slave] sections to allocate servers */
        i = 0;
        while (groups[i] != NULL) {
            if ((strncmp(groups[i++], "slave", 5) == 0)) {
                (*nb_server)++;
            }
        }

        if ((*nb_server) > 0) {
            int c;

            /* Allocate servers */
            servers = g_slice_alloc(sizeof(server_t) * (*nb_server));

            i = 0;
            c = 0;
            while (groups[i] != NULL) {
                if ((strncmp(groups[i], "slave", 5) == 0)) {
                    gsize n_address;
                    gsize n_length;
                    gsize n_types;

                    /* Returns 0 if not found */
                    servers[c].id = g_key_file_get_integer(key_file, groups[i], "id", NULL);

                    /* 'slave' + space + " + ... + " */
                    if (strlen(groups[i]) > 8)
                        servers[c].name = g_strndup(groups[i] + 7, strlen(groups[i]) - 8);
                    else
                        servers[c].name = g_strdup_printf("%d", servers[c].id);

                    servers[c].addresses = g_key_file_get_integer_list(key_file, groups[i], "addresses",
                                                                       &n_address, NULL);
                    servers[c].lengths = g_key_file_get_integer_list(key_file, groups[i], "lengths",
                                                                     &n_length, NULL);
                    /* Types are optional (integer by default) but it's all or nothing */
                    servers[c].types = g_key_file_get_string_list(key_file, groups[i], "types", &n_types, NULL);

                    /* Check list to be sure each address is associated to a length */
                    if (n_address != n_length) {
                        g_error("Not same number of addresses (%zd) and lengths (%zd)", n_address, n_length);
                    }

                    if (servers[c].types != NULL && n_types != n_address) {
                        g_error("Not same number of addresses (%zd) and types (%zd)", n_address, n_length);
                    }


                    /* FIXME Check mutliple of two for float types */

                    servers[c].n = n_address;
                    if (opt->verbose) {
                        int n;

                        g_print("Name %s, ID %d\n", servers[c].name, servers[c].id);
                        for (n=0; n < servers[c].n; n++) {
                            g_print("Address %d => %d values", servers[c].addresses[n], servers[c].lengths[n]);
                            if (servers[c].types != NULL) {
                                g_print(" (%s)\n", servers[c].types[n]);
                            } else {
                                g_print(" (int)\n");
                            }
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

    return servers;
}

void keyfile_server_free(int nb_server, server_t* servers)
{
    int i;

    if (nb_server > 0) {
        for (i=0; i < nb_server; i++) {
            g_free(servers[i].name);
            g_free(servers[i].addresses);
            g_free(servers[i].lengths);
            g_strfreev(servers[i].types);
        }
        g_slice_free1(sizeof(server_t) * nb_server, servers);
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
