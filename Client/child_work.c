#include "unp.h"
#include <termios.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

static void set_noecho(int fd)          /* turn off echo (for slave pty) */
{
        struct termios  stermios;

        if (tcgetattr(fd, &stermios) < 0)
                printf("tcgetattr error");

        stermios.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
        stermios.c_oflag &= ~(ONLCR);
                        /* also turn off NL to CR/NL mapping on output */

        if (tcsetattr(fd, TCSANOW, &stermios) < 0)
                printf("tcsetattr error");
}

int create_work(int *pid, int *fdm)
{
	char    slave_name[20];
        *pid = pty_fork(fdm, slave_name, 20, NULL, NULL);
        if (*pid < 0)
                printf("fork error");

        else if (*pid == 0) {            /* child with pty slave! */
                set_noecho(STDIN_FILENO);
                if( execl("/usr/bin/bash", NULL)  == -1)
                        printf("%s execve error!", strerror(errno));
		printf("child process bash exited!\n");
		exit(0);
        }
        int cflags = fcntl(*fdm, F_GETFL,0);
        fcntl(*fdm, F_SETFL, cflags|O_NONBLOCK);
	return 0;
}
