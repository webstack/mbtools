#ifndef _KEYFILE_H_
#define _KEYFILE_H_

#include <glib.h>
#include <modbus.h>
#include "option.h"

typedef struct {
    modbus_t *ctx;
    int id;
    int n;
    int *addresses;
    int *lengths;
} client_t;

client_t* keyfile_parse(option_t *opt, int *nb_client);
void keyfile_client_free(int nb_client, client_t* clients);

#endif /* _KEYFILE_H_ */
