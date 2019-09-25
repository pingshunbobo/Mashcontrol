#include	"unp.h"
#include	<stdlib.h>
#include	<stdbool.h>
#include	<sys/epoll.h>
#include	<termios.h>
#include 	"mash.h"

extern enum CONTROL_STATUS control_stat;
extern struct termios saved_stermios;
extern int client_fd;
extern fd_set  rset;

void set_nobrk(int fd)
{
        struct termios  stermios;
        if (tcgetattr(fd, &stermios) < 0)
                printf("tcgetattr error");
        stermios.c_iflag |= (IGNPAR | ICRNL);
        stermios.c_iflag &= ~( IGNCR | IXON );
        stermios.c_lflag &= ~( ISIG | ICANON | IEXTEN | ECHO | ECHOE | ECHOK );

        if (tcsetattr(fd, TCSANOW, &stermios) < 0)
                printf("tcsetattr error");
}

void save_termios(int fd)
{
        struct termios  stermios;
        if (tcgetattr(fd, &stermios) < 0)
                printf("tcgetattr error");
        memcpy(&saved_stermios, &stermios, sizeof(struct termios));
}

void restore_termios(int fd)
{
        if (tcsetattr(fd, TCSANOW, &saved_stermios) < 0)
                printf("tcsetattr error %s\n", strerror(errno));
}

enum MASH_DATA_TYPE mash_type(char *reply, int nbytes)
{
	if(!strncmp(reply, "Mashctl:", 8))
                return MASH_CTL;
	if(!strncmp(reply, "Mashnote:", 9))
                return MASH_NOTE;
	if(!strncmp(reply, "Mashinfo:", 9))
                return MASH_INFO;
	if(!strncmp(reply, "Mashcmd:", 8))
                return MASH_CMD;
	if(!strncmp(reply, "Mashdata:", 9))
                return MASH_DATA;
	return MASH_UNKNOW;
}

int mash_ctl(char *reply, int nbytes)
{
	if(!strncmp(reply + 8, "interface!", 11)){
		control_stat = INTERFACE;
		set_nobrk(STDIN_FILENO);
	}
	if(!strncmp(reply + 8, "mashcmd!", 8)){
		control_stat = MASHCTL;
		restore_termios(STDIN_FILENO);
	}
	return 0;
}

int mash_cmd(char *reply, int nbytes)
{
	if(!strncmp(reply + 8, "interface!", 21)){
		control_stat = INTERFACE;
		set_nobrk(STDIN_FILENO);
	}
	if(!strncmp(reply + 8, "mashctl!", 19)){
		control_stat = MASHCTL;
		restore_termios(STDIN_FILENO);
	}
	return 0;
}

int mash_note(char *reply, int nbytes)
{
	if (writen(STDOUT_FILENO, reply + 9, nbytes - 9) != nbytes - 9)
		printf("writen stdout error.\n");
	return 0;
}

int mash_data(char *reply, int nbytes)
{
	if (writen(STDOUT_FILENO, reply + 9, nbytes - 9) != nbytes - 9)
		printf("writen stdout error.\n");
	return 0;
}
