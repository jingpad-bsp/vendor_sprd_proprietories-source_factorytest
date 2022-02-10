#ifndef _MODEM_H_
#define _MODEM_H_

#define MODEM_TYPE_TD 0x3434
#define MODEM_TYPE_W 0x5656
#define MODEM_TYPE_LTE 0x7878
#define MODEM_TYPE_NR 0x9A9A
#define MODEM_RADIO_TYPE "ro.vendor.radio.modemtype"
#define TTY_DEV_PROP "ro.vendor.modem.tty"

int modem_send_at(int fd, char* cmd, char* buf, unsigned int buf_len, int wait);
int sendATCmd(int phoneId, char* cmd, char* buf, unsigned int buf_len, int wait);
int wcn_send_at(char* cmd);
int slogmodem_send(char* cmd);
void flush_wcn_log(void);
int telSendAt(int phoneId, char* cmd, char* buf, unsigned int buf_len, int wait);
int tel_send_at(int fd, char* cmd, char* buf, unsigned int buf_len, int wait);
void* modem_init_func(void *);
int test_modem_getlteconf(void);
char* test_modem_get_ver(void);
char* test_modem_get_caliinfo(void);
void* sim_check_thread(void *);
int test_sim_pretest_common(void);
int test_sim_start_common(void);

#endif