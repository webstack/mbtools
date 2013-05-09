#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define RECV_MAX 256
#define SOCK_PATH "/tmp/mbsocket"

static volatile int stop = 0;
static volatile int s = -1;

static void sigint_stop(int dummy)
{
    /* Stop the main process */
    stop = 1;
    close(s);
    s = -1;
}

int main(int argc, char **argv)
{
    struct sockaddr_un server;
    char str[RECV_MAX];
    int msgsock = -1;
    int error;

    /* Disable buffering */
    setbuf(stdout, NULL);
    signal(SIGINT, sigint_stop);

    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, SOCK_PATH);
    if (bind(s, (struct sockaddr *)&server, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(s, 5) == -1) {
        perror("listen");
        exit(1);
    }

    while (!stop) {
        msgsock = accept4(s, 0, 0, SOCK_CLOEXEC);
        if (msgsock == -1) {
            unlink(server.sun_path);
            perror("accept");
            exit(1);
        }
        error = 0;
        while (!error && !stop) {
            int n;

            n = read(msgsock, str, RECV_MAX - 1);
            if (n <= 0) {
                perror("recv");
                error = 1;
            } else {
                str[n] = '\0';
                printf("%s", str);
            }
        }
        close(msgsock);
        msgsock = -1;
    }

    if (msgsock != -1) {
        close(msgsock);
    }

    /* Close socket first to not wait locked file */
    if (s != -1) {
        close(s);
    }
    unlink(server.sun_path);

    return 0;
}
