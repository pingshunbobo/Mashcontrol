#include	<sys/epoll.h>

enum MASH_DATA_TYPE {MASH_CTL, MASH_INFO, MASH_CMD, MASH_DATA, MASH_NOTE, MASH_UNKNOW};
enum CONTROL_STATUS {MASHCMD, INTERFACE};

enum MASH_DATA_TYPE mash_type(char *reply, int nbytes);

void save_termios(int fd);
void restore_termios(int fd);

int mash_ctl(char *reply, int nbytes);
int mash_data(char *reply, int nbytes);
