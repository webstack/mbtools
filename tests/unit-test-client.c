/*
 * Copyright © 2013 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>


#define SERVER_ID 1

enum {
    TCP,
    TCP_PI,
    RTU
};

int main(int argc, char *argv[])
{
    modbus_t *ctx;
    uint16_t *tab_reg;
    int rc;
    int use_backend;
    const uint16_t values[] = {5678, 9012};

    if (argc > 1) {
        if (strcmp(argv[1], "tcp") == 0) {
            use_backend = TCP;
        } else if (strcmp(argv[1], "tcppi") == 0) {
            use_backend = TCP_PI;
        } else if (strcmp(argv[1], "rtu") == 0) {
            use_backend = RTU;
        } else {
            printf("Usage:\n  %s [tcp|tcppi|rtu] - Modbus client for unit testing\n\n", argv[0]);
            exit(1);
        }
    } else {
        /* By default */
        use_backend = TCP;
    }

    if (use_backend == TCP) {
        ctx = modbus_new_tcp("127.0.0.1", 1502);
    } else if (use_backend == TCP_PI) {
        ctx = modbus_new_tcp_pi("::1", "1502");
    } else {
        ctx = modbus_new_rtu("/dev/ttyUSB1", 115200, 'N', 8, 1);
    }
    if (ctx == NULL) {
        fprintf(stderr, "Unable to allocate libmodbus context\n");
        return -1;
    }
    modbus_set_debug(ctx, TRUE);
    modbus_set_error_recovery(ctx,
                              MODBUS_ERROR_RECOVERY_LINK |
                              MODBUS_ERROR_RECOVERY_PROTOCOL);

    if (use_backend == RTU) {
        modbus_set_slave(ctx, SERVER_ID);
    }

    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    /* Allocate and initialize the memory to store the registers */
    tab_reg = (uint16_t *) malloc(10 * sizeof(uint16_t));

    /* Single register */
    printf("1/2 modbus_write_register");
    rc = modbus_write_register(ctx, 0, 1234);
    if (rc != 1) {
        goto close;
    }

    printf("2/2 modbus_read_registers");
    rc = modbus_read_registers(ctx, 0, 1, tab_reg);
    if (rc != 1) {
        goto close;
    }

    if (tab_reg[0] != 1234) {
        goto close;
    }

    /* Many registers */
    printf("1/2 modbus_write_registers");
    rc = modbus_write_registers(ctx, 1, 2, values);
    if (rc != 2) {
        goto close;
    }

    printf("2/2 modbus_read_registers");
    rc = modbus_read_registers(ctx, 1, 2, tab_reg);
    if (rc != 2) {
        goto close;
    }

close:
    /* Free the memory */
    free(tab_reg);

    /* Close the connection */
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
