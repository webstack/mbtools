#ifndef _KEYFILE_H_
#define _KEYFILE_H_

#include <glib.h>
#include <modbus.h>
#include "option.h"

#define MBT_LOCAL_INI_FILE "mbcollect.ini"
#define MBT_ETC_INI_FILE ("/etc/" MBT_LOCAL_INI_FILE)

typedef struct {
    /* RTU - Slave ID */
    int id;
    /* TCP - IP address*/
    char *ip;
    /* TCP - Port number */
    int port;
    /* TCP - Modbus context */
    modbus_t *ctx;
    /* Name of host */
    char *name;
    /* Number of addresses to read */
    int n;
    /* List of addresses to read */
    int *addresses;
    /* List of lengths to read at each address */
    int *lengths;
    /* List of data types (int, floatmsb, floatlsb) at each address */
    char **types;
} server_t;

server_t* keyfile_parse(option_t *opt, int *nb_server);
void keyfile_server_free(int nb_server, server_t* servers);

#endif /* _KEYFILE_H_ */
