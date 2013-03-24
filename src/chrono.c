/* Copyright © 2002-2013 Stéphane Raimbault */

#define _XOPEN_SOURCE /* glibc2 needs this */
#include <glib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include "chrono.h"

char* new_date_time_string(void)
{
    time_t time_now = time(NULL);
    struct tm tm_now;
    const int LENGTH_DATE_TIME = 20;
    char *s_date_time;

    s_date_time = g_malloc(sizeof(char) * (LENGTH_DATE_TIME));

    /* Date and time */
    localtime_r(&time_now, &tm_now);
    strftime(s_date_time, LENGTH_DATE_TIME, "%F %T", &tm_now);

    return s_date_time;
}

void print_date_time(void)
{
    char *s_date_time;
    s_date_time = new_date_time_string();
    g_print("%s", s_date_time);
    g_free(s_date_time);
}

/* Display seconds since Epoch (00:00:00 UTC, January 1, 1970) */
chrono_t* chrono_start(const char *message)
{
    chrono_t *chrono = g_new(chrono_t, 1);
    char *s_date_time;

    g_get_current_time(&(chrono->start));
    s_date_time = new_date_time_string();

    if (message != NULL) {
        chrono->message = g_strdup(message);
        g_print("|----------------\n| Start %s: %s\n|\n",
                message, s_date_time);
    } else {
        chrono->message = NULL;
        g_print("|----------------\n| Start: %s\n|\n",
                s_date_time);
    }
    g_free(s_date_time);

    return chrono;
}

void chrono_stop(chrono_t *chrono)
{
    char *s_date_time;
    GTimeVal stop;

    g_get_current_time(&stop);
    s_date_time = new_date_time_string();

    if (chrono->message != NULL) {
        g_print("|\n| Stop %s: %s\n",
                chrono->message, s_date_time);
        g_free(chrono->message);
    } else
        g_print("|\n| Stop: %s\n",
                s_date_time);
    g_free(s_date_time);

    g_print("| Elapsed time: %.3f second(s)\n|--------------\n",
            stop.tv_sec
            + ((double)(stop.tv_usec) / 1000000.0)
            - (double)(chrono->start).tv_sec
            - ((double)(chrono->start).tv_usec / 1000000.0));
    g_free(chrono);
}

/* Split a delay in hour, minute, second and tenth of second */
void split_delay_in_hmsd(int delay,
                         int *hour, int *minute, int *second,
                         int *tenth)
{
    *tenth = delay - ((delay/10) * 10);
    delay = delay / 10;

    /* Convert in HH:MM:SS */
    *hour = delay / 3600;
    *minute = (delay / 60) - ((*hour)*60);
    *second = delay % 60;
}
