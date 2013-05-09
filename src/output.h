#ifndef _OUTPUT_H_
#define _OUTPUT_H_

#include <glib.h>
#include <inttypes.h>

#include "option.h"

int output_connect(char* socket_file, gboolean verbose);
void output_close(int *s);
int output_write(int s, option_t *opt, char *name, int addr, int nb_reg, char *type, uint16_t *tab_reg,
                 gboolean verbose);
gboolean output_is_connected(int s);

#endif /* _OUTPUT_H_ */
