#ifndef _COLLECT_H_
#define _COLLECT_H_

#include <modbus.h>

#include "option.h"
#include "keyfile.h"

int collect_listen(modbus_t *ctx, option_t *opt);
void collect_poll(modbus_t *ctx, option_t *opt, server_t *servers, int nb_server);

#endif /* _COLLECT_H_ */
