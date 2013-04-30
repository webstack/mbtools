#ifndef _KEYFILE_H_
#define _KEYFILE_H_

#include <glib.h>
#include <modbus.h>
#include "option.h"

#define MBT_LOCAL_INI_FILE "mbcollect.ini"
#define MBT_ETC_INI_FILE ("/etc/" MBT_LOCAL_INI_FILE)

typedef struct {
    modbus_t *ctx;
    int id;
    char *name;
    int n;
    int *addresses;
    int *lengths;
    char **types;
} client_t;

client_t* keyfile_parse(option_t *opt, int *nb_client);
void keyfile_client_free(int nb_client, client_t* clients);

#endif /* _KEYFILE_H_ */
