#define BUF_SIZE    16384

enum MASH_DATA_TYPE {MASH_CTL, MASH_INFO, MASH_DATA, MASH_UNKNOW};
enum CLIENT_STATUS {WORK, STANDBY};

enum MASH_DATA_TYPE mash_type(char *reply, int nbytes);
int mash_proc_ctl(char *reply, int nbytes);
int mash_proc_data(char *reply, int nbytes);
int mash_send_ctl(char *reply, int nbytes);
int mash_send_data(char *reply, int nbytes);
