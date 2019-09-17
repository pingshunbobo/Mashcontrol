#include "unp.h"
#include <termios.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

int create_work(int *pid, int *fdm)
{
	char    slave_name[20];
        *pid = pty_fork(fdm, slave_name, 20, NULL, NULL);
        if (*pid < 0)
                printf("fork error");

        else if (*pid == 0) {            /* child with pty slave! */
                if( execl("/usr/bin/bash", "cd ~", NULL)  == -1)
                        printf("%s execve error!", strerror(errno));
		printf("child process bash exited!\n");
		exit(0);
        }
        int cflags = fcntl(*fdm, F_GETFL,0);
        fcntl(*fdm, F_SETFL, cflags|O_NONBLOCK);
	return 0;
}
