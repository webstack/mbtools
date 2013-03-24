/* Copyright © 2002-2013 Stéphane Raimbault */

#ifndef _CHRONO_H_
#define _CHRONO_H_

#include <glib.h>

typedef struct {
    GTimeVal start;
    char *message;
} chrono_t;

char* new_date_time_string(void);
void print_date_time(void);

chrono_t* chrono_start(const char *message);
void chrono_stop(chrono_t *chrono);

void split_delay_in_hmsd(int delay,
                         int *hour, int *minute, int *second,
                         int *tenth);

#endif /* _CHRONO_H_ */
