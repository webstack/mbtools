#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "daemon.h"

/* Used by the daemon to store its PID in a file */
void pid_file_create(char *pid_file)
{
    char pidbuf[16];
    pid_t pid = getpid();
    int fd, len;

    if ((fd = open(pid_file, O_WRONLY | O_CREAT | O_EXCL, 0666)) == -1) {
        fprintf(stderr, "Failed to create pid file %s: %s\n", pid_file, strerror(errno));
        _exit(0);
    }

    snprintf(pidbuf, sizeof pidbuf, "%ld\n", (long)pid);
    len = strlen(pidbuf);
    if (write(fd, pidbuf, len) != len) {
        fprintf(stderr, "Failed to write in pid file %s: %s\n", pid_file, strerror(errno));
    }
    close(fd);
}

void pid_file_delete(char *pid_file)
{
    unlink(pid_file);
}
