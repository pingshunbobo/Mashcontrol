#include "unp.h"
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "mash.h"

extern fd_set   rset;
extern enum CLIENT_STATUS  client_stat;

int create_work(int *pid, int *fdm)
{
	char    slave_name[20];
        *pid = pty_fork(fdm, slave_name, 20, NULL, NULL);
        if (*pid < 0)
                log_client("fork error");

        else if (*pid == 0) {            /* child with pty slave! */
                if( execl("/usr/bin/bash", "cd ~", NULL)  == -1)
                        log_client("execve error when create bash work!");
		log_client("child process bash exited!\n");
		exit(0);
        }
        int cflags = fcntl(*fdm, F_GETFL, 0);
        fcntl(*fdm, F_SETFL, cflags|O_NONBLOCK);
	return *pid;
}

void out_work(int *work_pid, int *fdm)
{
	*work_pid = 0;
	client_stat = STANDBY;
	FD_CLR(*fdm, &rset);
	close(*fdm);
	*fdm = 0;
	mash_send_cntl("standby!", 8);
}
