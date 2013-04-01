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
#include "collect.h"

#define BITS_NB 0
#define INPUT_BITS_NB 0
#define REGISTERS_NB 400
#define INPUT_REGISTERS_NB 0

static volatile gboolean stop = FALSE;

int collect_listen(modbus_t *ctx, option_t *opt)
{
    modbus_mapping_t *mb_mapping = NULL;
    uint8_t query[MODBUS_RTU_MAX_ADU_LENGTH];
    int s = 0;
    int header_length = modbus_get_header_length(ctx);

    mb_mapping = modbus_mapping_new(BITS_NB, INPUT_BITS_NB, REGISTERS_NB, INPUT_REGISTERS_NB);
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n", modbus_strerror(errno));
        return -1;
    }
    modbus_set_slave(ctx, opt->id);
    while (!stop) {
        int rc;

        rc = modbus_receive(ctx, query);
        if (rc > 0) {
            modbus_reply(ctx, query, rc, mb_mapping);
            /* Write multiple registers */
            if (query[header_length] == 0x10) {
                int addr = MODBUS_GET_INT16_FROM_INT8(query, header_length + 1);
                int nb = MODBUS_GET_INT16_FROM_INT8(query, header_length + 3);

                /* Write to local unix socket */
                if (opt->verbose)
                  g_print("Addr %d: %d values\n", addr, nb);
                
                if (!output_is_connected(s))
                  s = output_connect(opt->socket_file, opt->verbose);
                
                if (output_is_connected(s))
                {
                  if (output_write(s, -1, addr, nb, mb_mapping->tab_registers + addr, opt->verbose) == -1)
                  {
                    output_close(s);
                    s = -1;
                  }
                }
            }
        }
    }

    modbus_mapping_free(mb_mapping);
    output_close(s);

    return 0;
}

void collect_poll(modbus_t *ctx, option_t *opt, client_t *clients, int nb_client)
{
    int i;
    int n;
    uint16_t tab_reg[MODBUS_MAX_READ_REGISTERS];
    /* Local unix socket to output */
    int s = 0;

    while (!stop) {
        GTimeVal tv;
        int delta;

        g_get_current_time(&tv);
        /* Seconds until the next multiple of RECORDING_INTERVAL */
        delta = (((tv.tv_sec / opt->interval) + 1) * opt->interval) - tv.tv_sec;
        if (opt->verbose) {
            g_print("Going to sleep for %d seconds...\n", delta);
        }

        sleep(delta);

        if (opt->verbose) {
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

                if (opt->verbose) {
                    g_print("ID: %d, addr:%d l:%d\n", client->id, client->addresses[n], client->lengths[n]);
                }

                nb_reg = modbus_read_registers(ctx, client->addresses[n], client->lengths[n], tab_reg);
                if (nb_reg == -1) {
                    g_warning("ID: %d, addr:%d l:%d %s\n", client->id, client->addresses[n], client->lengths[n],
                              modbus_strerror(errno));
                } else {
                    /* Write to local unix socket */
                    if (!output_is_connected(s))
                      s = output_connect(opt->socket_file, opt->verbose);
                    
                    if (output_is_connected(s))
                    {
                      if (output_write(s, client->id, client->addresses[n], nb_reg, tab_reg, opt->verbose) == -1)
                      {
                        output_close(s);
                        s = -1;
                      }
                    }
                }
            }
        }
    }
    output_close(s);
}

static void sigint_stop(int dummy)
{
    /* Stop the main process */
    stop = TRUE;
}

int main(int argc, char *argv[])
{
    int rc = 0;
    option_t *opt = NULL;
    int nb_client = 0;
    client_t *clients = NULL;
    modbus_t *ctx = NULL;

    /* Parse command line options */
    opt = option_new();
    option_parse(opt, &argc, &argv);

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

    modbus_set_debug(ctx, opt->verbose);
    if (modbus_connect(ctx) == -1) {
        goto quit;
    }

    if (opt->listen) {
        if (opt->verbose) {
            g_print("Running in slave mode\n");
        }
        collect_listen(ctx, opt);
    } else {
        collect_poll(ctx, opt, clients, nb_client);
    }

quit:
    if (opt->daemon) {
        pid_file_delete(opt->pid_file);
    }

    modbus_close(ctx);
    modbus_free(ctx);

    keyfile_client_free(nb_client, clients);
    option_free(opt);

    g_print("mbcollect is stopped.\n");

    return rc;
}
