#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <glib.h>
#include <string.h>
#include <errno.h>
#include <modbus.h>

#include "daemon.h"
#include "chrono.h"
#include "option.h"
#include "keyfile.h"
#include "output.h"

static volatile gboolean stop = FALSE;

static void sigint_stop(int dummy)
{
    /* Stop the main process */
    stop = TRUE;
}

int main(int argc, char *argv[])
{
    int rc = 0;
    int i;
    int n;
    option_t *opt = NULL;
    int verbose;
    int nb_client = 0;
    client_t *clients = NULL;
    modbus_t *ctx = NULL;
    uint16_t tab_reg[MODBUS_MAX_READ_REGISTERS];
    /* Local unix socket to output */
    int s = -1;

    /* Parse command line options */
    opt = option_new();
    option_parse(opt, &argc, &argv);
    verbose = opt->verbose;

    /* Parse .ini file */
    clients = keyfile_parse(opt, &nb_client);

    if (option_set_undefined(opt) == -1) {
        goto quit;
    }

    /* Launched as daemon */
    if (opt->daemon) {
        pid_t pid = fork();
        if (pid) {
            if (pid < 0) {
                fprintf(stderr, "Failed to fork the main: %s\n", strerror(errno));
            }
            _exit(0);
        }
        pid_file_create(opt->pid_file);
        /* detach from the terminal */
        setsid();
    }

    /* Signal */
    signal(SIGINT, sigint_stop);

    ctx = modbus_new_rtu(opt->device, opt->baud, opt->parity[0], opt->data_bit, opt->stop_bit);
    if (ctx == NULL) {
        fprintf(stderr, "Modbus init error\n");
        rc = -1;
        goto quit;
    }

    modbus_set_debug(ctx, !verbose);
    if (modbus_connect(ctx) == -1) {
        goto quit;
    }

    s = output_connect(opt->socket_file, verbose);

    while (!stop) {
        GTimeVal tv;
        int delta;

        g_get_current_time(&tv);
        /* Seconds until the next multiple of RECORDING_INTERVAL */
        delta = (((tv.tv_sec / opt->interval) + 1) * opt->interval) - tv.tv_sec;
        if (verbose) {
            g_print("Going to sleep for %d seconds...\n", delta);
        }

        sleep(delta);
        if (verbose) {
            g_print("Wake up: ");
            print_date_time();
            g_print("\n");
        }

        for (i = 0; i < nb_client; i++) {
            client_t *client = &(clients[i]);

            if (client->id > 0) {
                modbus_set_slave(ctx, client->id);
            }
            for (n = 0; n < client->n; n++) {
                int nb_reg;

                if (verbose) {
                    g_print("ID: %d, addr:%d l:%d\n", client->id, client->addresses[n], client->lengths[n]);
                }

                nb_reg = modbus_read_registers(ctx, client->addresses[n], client->lengths[n], tab_reg);
                if (nb_reg == -1) {
                    g_warning("ID: %d, addr:%d l:%d %s\n", client->id, client->addresses[n], client->lengths[n],
                              modbus_strerror(errno));
                } else {
                    /* Write to local unix socket */
                    while ((output_write(s, client->id, client->addresses[n], nb_reg, tab_reg) == -1) && !stop) {
                        sleep(1);
                        output_close(s);
                        output_connect(opt->socket_file, verbose);
                    }
                }
            }
        }
    }

quit:
    if (opt->daemon) {
        pid_file_delete(opt->pid_file);
    }

    modbus_close(ctx);
    modbus_free(ctx);

    keyfile_client_free(nb_client, clients);
    option_free(opt);
    output_close(s);

    g_print("mbcollect is stopped.\n");

    return rc;
}
