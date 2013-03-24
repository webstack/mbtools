#ifndef _OUTPUT_H_
#define _OUTPUT_H_

#include <unistd.h>
#include "keyfile.h"

int output_connect(char* socket_file, gboolean verbose);
void output_close(int s);
int output_write(int s, int slave_id, int addr, int nb_reg, uint16_t *tab_reg);

#endif /* _OUTPUT_H_ */
