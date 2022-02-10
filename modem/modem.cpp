#include "common.h"
#include "modem.h"
#include "eng_tok.h"
#include <string.h>

#define NUM_ELEMS(x) (sizeof(x)/sizeof(x[0]))

char s_modem_ver[1024];
char s_cali_info[1024];
char s_cali_info1[1024];
char s_cali_info2[1024];
//support cdma2000
char s_cali_info_cdma2000[1024];
//support 5g
char s_cali_info_nr[1024];

int s_modem_conf = -1;
int s_sim_state = 2;
char imei_buf1[128];
char imei_buf2[128];

static char s_ATBuffer[AT_BUFFER_SIZE];
static char s_ATDictBuf[AT_BUFFER_SIZE];
static char *s_ATBufferCur = s_ATBuffer;
static int thread_run = 0;
//static int modem_fd[MAX_MODEM_COUNT];
//static char modem_port[MAX_MODEM_COUNT][BUF_LEN];
static char  modem_count_prop[BUF_LEN];

static int s_modem_count = 0;
static int init_mfalg= 0;
pthread_mutex_t tel_mutex = PTHREAD_MUTEX_INITIALIZER;

#define SOCKET_NAME_MODEM_CTL "modemd"
#define CMDLINE_SIZE    (0x1000)

char* sim_name[] = {
    TEXT_SIM1,
    TEXT_SIM2
};

static const char * s_finalResponse[] = {
    "OK",
    "ERROR",
};

int s_modem_type = MODEM_TYPE_LTE;

#define MAX_AT_CHANNEL_NAME_LENGHT (32)
#define MAX_PHONE_MUM (4)
char s_lte_normal_at_channel[MAX_PHONE_MUM][MAX_AT_CHANNEL_NAME_LENGHT]={
    "/dev/stty_lte1",
    "/dev/stty_lte4",
    "/dev/stty_lte7",
    "/dev/stty_lte10",
};

char s_w_normal_at_channel[MAX_PHONE_MUM][MAX_AT_CHANNEL_NAME_LENGHT]={
    "/dev/stty_w1",
    "/dev/stty_w4",
    "/dev/stty_w7",
    "/dev/stty_w10",
};

char s_lte_tel_at_channel[MAX_PHONE_MUM][MAX_AT_CHANNEL_NAME_LENGHT]={
    "/dev/stty_lte2",
    "/dev/stty_lte5",
    "/dev/stty_lte8",
    "/dev/stty_lte11",
};

char s_w_tel_at_channel[MAX_PHONE_MUM][MAX_AT_CHANNEL_NAME_LENGHT]={
    "/dev/stty_w2",
    "/dev/stty_w5",
    "/dev/stty_w8",
    "/dev/stty_w11",
};

//Add for 5G
char s_nr_normal_at_channel[MAX_PHONE_MUM][MAX_AT_CHANNEL_NAME_LENGHT]={
    "/dev/stty_nr1",
    "/dev/stty_nr4",
    "/dev/stty_nr7",
    "/dev/stty_nr10",
};

char s_nr_tel_at_channel[MAX_PHONE_MUM][MAX_AT_CHANNEL_NAME_LENGHT]={
    "/dev/stty_nr2",
    "/dev/stty_nr5",
    "/dev/stty_nr8",
    "/dev/stty_nr11",
};

/** returns 1 if line starts with prefix, 0 if it does not */
static int strStartsWith(const char *line, const char *prefix)
{
    for ( ; *line != '\0' && *prefix != '\0' ; line++, prefix++) {
        if (*line != *prefix) {
            return 0;
        }
    }

    return *prefix == '\0';
}
/**
 * returns 1 if line is a final response, either  error or success
 * See 27.007 annex B
 * WARNING: NO CARRIER and others are sometimes unsolicited
 */

static int isFinalResponse(const char *line)
{
    size_t i;

    for (i = 0 ; i < NUM_ELEMS(s_finalResponse) ; i++) {
        if (strStartsWith(line, s_finalResponse[i])) {
            return 1;
        }
    }

    return 0;
}

/*
 * Returns a pointer to the end of the next line
 * special-cases the "> " SMS prompt
 *
 * returns NULL if there is no complete line
 */
static char * findNextEOL(char *cur)
{
    if (cur[0] == '>' && cur[1] == ' ' && cur[2] == '\0') {
        /* SMS prompt character...not \r terminated */
        return cur+2;
    }

    // Find next newline
    while (*cur != '\0' && *cur != '\r' && *cur != '\n') cur++;

    return *cur == '\0' ? NULL : cur;
}

static char *eng_readline(int modemfd)
{
    ssize_t count;

    char *p_read = NULL;
    char *p_eol = NULL;
    char *ret;

    /* this is a little odd. I use *s_ATBufferCur == 0 to
     * mean "buffer consumed completely". If it points to a character, than
     * the buffer continues until a \0
     */

    if (*s_ATBufferCur == '\0') {
        /* empty buffer */
        s_ATBufferCur = s_ATBuffer;
        *s_ATBufferCur = '\0';
        p_read = s_ATBuffer;
    } else {   /* *s_ATBufferCur != '\0' */
        /* there's data in the buffer from the last read */

        // skip over leading newlines
        while (*s_ATBufferCur == '\r' || *s_ATBufferCur == '\n')
            s_ATBufferCur++;

        p_eol = findNextEOL(s_ATBufferCur);

        if (p_eol == NULL) {
            /* a partial line. move it up and prepare to read more */
            size_t len;

            len = strlen(s_ATBufferCur);

            memmove(s_ATBuffer, s_ATBufferCur, len + 1);
            p_read = s_ATBuffer + len;
            s_ATBufferCur = s_ATBuffer;
        }
        /* Otherwise, (p_eol !- NULL) there is a complete line  */
        /* that will be returned the while () loop below        */
    }


    while (p_eol == NULL) {
        if (0 == AT_BUFFER_SIZE - (p_read - s_ATBuffer)) {
            /* ditch buffer and start over again */
            s_ATBufferCur = s_ATBuffer;
            *s_ATBufferCur = '\0';
            p_read = s_ATBuffer;
        }
        do {
            count = read(modemfd, p_read,
                    AT_BUFFER_SIZE - (p_read - s_ATBuffer));
        } while (count < 0 && errno == EINTR);

        if (count > 0) {
            p_read[count] = '\0';
            // skip over leading newlines
            while (*s_ATBufferCur == '\r' || *s_ATBufferCur == '\n')
                s_ATBufferCur++;

            p_eol = findNextEOL(s_ATBufferCur);
            p_read += count;
        } else if (count <= 0) {
            return NULL;
        }
    }

    /* a full line in the buffer. Place a \0 over the \r and return */
    ret = s_ATBufferCur;
    *p_eol = '\0';
    s_ATBufferCur = p_eol + 1; /* this will always be <= p_read,    */
    /* and there will be a \0 at *p_read */
    return ret;
}

int modem_ctrl_int_modem_type(void) {
  char modem_type[PROPERTY_VALUE_MAX];

  /*get modem mode from property*/
  property_get(MODEM_RADIO_TYPE, modem_type, "not_find");

  LOGD("%s: modem type is %s", __FUNCTION__, modem_type);

  if (strcmp("not_find", modem_type) == 0) {
    LOGE("%s: %s %s", __FUNCTION__, MODEM_RADIO_TYPE, "not_find");
    return -1;
  }

  if (0 == strcmp("t", modem_type)) {
    s_modem_type = MODEM_TYPE_TD;
  } else if (0 == strcmp("w", modem_type)) {
    s_modem_type = MODEM_TYPE_W;
  } else if (0 == strcmp("l", modem_type)) {
    s_modem_type = MODEM_TYPE_LTE;
  } else if (0 == strcmp("nr", modem_type)) {
    s_modem_type = MODEM_TYPE_NR;
  }
  else {
    LOGD("%s: modem type(%s) error!", __FUNCTION__, modem_type);
    return -1;
  }

  return 0;
}

int getModemType(){
    return s_modem_type;
}

char* getAtChannelPath(int phoneId){
    if (phoneId < 0 || phoneId >= MAX_PHONE_MUM) {
        return NULL;
    }

    //Add for 5G
    if (getModemType() == MODEM_TYPE_NR){
        return s_nr_normal_at_channel[phoneId];
    }else if (getModemType() == MODEM_TYPE_LTE){
        return s_lte_normal_at_channel[phoneId];
    }else{
        return s_w_normal_at_channel[phoneId];
    }
}

/**************************************************************************************/
int modem_send_at(int fd, char* cmd, char* buf, unsigned int buf_len, int wait)
{
    struct timeval timeout;
    int ret = -1;
    int cmd_len = 0;
    int rsp_len = 0;
    char* line = NULL;
    fd_set readfs;

    if(NULL == cmd) {
        LOGE("error param");
        return -1;
    }

    LOGD("[fd:%d] >>>> at_cmd: %s", fd, cmd);
    cmd_len = strlen(cmd);
    ret = write(fd, cmd, cmd_len);
    if(ret != cmd_len) {
        LOGE("mmitest write err, ret=%d, cmd_len=%d\n",  ret, cmd_len);
        return -1;
    }
    write(fd, "\r", 1);

    if(wait <= 0) wait = 5;

    for(;;) {
        timeout.tv_sec = wait;
        timeout.tv_usec = 0;
        FD_ZERO(&readfs);
        FD_SET(fd, &readfs);
        ret = select(fd+1, &readfs, NULL, NULL, &timeout);
        if (ret < 0) {
            LOGE("mmitest select err ");
            if(errno == EINTR || errno == EAGAIN) {
                continue;
            } else {
                return -1;
            }
        } else if(ret == 0) {
            LOGD("mmitest select time out");
            return -1;
        } else {
            /* go to recv response*/
            break;
        }
    }

    for(;;) {
        line = eng_readline(fd);
        LOGD("mmitest %s [fd:%d] <<<< at_rsp: %s",cmd, fd, line);
        if(strstr(line, "OK"))
        {
            ret = rsp_len;
            break;
        } else if(strstr(line, "ERROR"))
        {
            ret = -1;
            break;
        }
        else {
            if(buf_len == 0 || buf == NULL) {
                continue;
            }
            if(rsp_len + strlen(line) > buf_len) {
                LOGD("mmitest  recv too many word, (%d) > (%d)\n",
                         (rsp_len + strlen(line)), buf_len);
                ret = -1;
                break;
            }
            //Add '\n' to end of line.
            char* s="\n";
            char tmp[AT_BUFFER_SIZE]= {0};
            sprintf(tmp, "%s%s", line,s);
            memcpy(buf+rsp_len, tmp, strlen(tmp));
            rsp_len += strlen(tmp);
        }
    }
    return ret;
}

int sendATCmd(int phoneId, char* cmd, char* buf, unsigned int buf_len, int wait)
{
    int fd = -1;
    int ret = -1;
    char *path = NULL;

    LOGD("sendATCmd: phoneId = %d, cmd = %s", phoneId, cmd);
    path = getAtChannelPath(phoneId);
    if (path == NULL) {
        LOGE("get at channel path fail!");
        return -1;
    }

    LOGD("get at channel return: %s", path);
    fd = open(path, O_RDWR);
    if (fd == -1) {
        LOGD("open at channel fail!");
        return -1;
    }

    ret = modem_send_at(fd, cmd, buf, buf_len, wait);
    if (ret == -1){
        LOGE("modem send at fail: ret = %d", ret);
    }
    close(fd);

    return ret;
}

int wcn_send_at(char* cmd)
{
    int fd = -1;
    int  times = 10;

    LOGD("connect socket <%s>...", SOCKET_NAME_WCND);

    while(0 < times--)
    {
        usleep(10 * 1000);
        fd = socket_local_client(SOCKET_NAME_WCND, ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
        if(fd > 0)
        {
            LOGD("connect <%s> success", SOCKET_NAME_WCND);
            break;
        }
        else
        {
            LOGE("connect <%s> faild, try again...", SOCKET_NAME_WCND);
        }
    }

    if(fd <= 0)
    {
        LOGE("connect <%s> faild", SOCKET_NAME_WCND);
        return -1;
    }
    else
    {
        LOGD("send <%s> to <%s>...",cmd, SOCKET_NAME_WCND);

        if(write(fd, cmd, strlen(cmd) + 1) <= 0)
        {
            LOGE("send AT cmd failed");
        }
        else
        {
            LOGD("send AT cmd success!");
        }
    }

    close(fd);
    return 0;
}

int slogmodem_send(char* cmd)
{
    int fd = -1;
    int  times = 10;

    LOGD("connect socket <%s>...", SOCKET_NAME_SLOG);

    while(0 < times--)
    {
        usleep(10 * 1000);
        fd = socket_local_client(SOCKET_NAME_SLOG, ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
        if(fd > 0)
        {
            LOGD("connect <%s> success", SOCKET_NAME_SLOG);
            break;
        }
        else
        {
            LOGE("connect <%s> faild, try again...", SOCKET_NAME_SLOG);
        }
    }

    if(fd <= 0)
    {
        LOGE("connect <%s> faild", SOCKET_NAME_SLOG);
        return -1;
    }
    else
    {
        LOGD("send <%s> to <%s>...",cmd, SOCKET_NAME_SLOG);

        if(write(fd, cmd, strlen(cmd) + 1) <= 0)
        {
            LOGE("send AT cmd failed");
        }
        else
        {
            LOGD("send AT cmd success!");
        }
    }

    close(fd);
    return 0;
}

void flush_wcn_log(void)
{

    //send AT for dump memory
    wcn_send_at("wcn poweron\0");
    wcn_send_at("wcn at+flushwcnlog\0");
    slogmodem_send("SAVE_LAST_LOG WCN\0");

}

char* getTelChannelPath(int phoneId){
    if (phoneId < 0 || phoneId >= MAX_PHONE_MUM) {
        return NULL;
    }

    //Add for 5G
    if (getModemType() == MODEM_TYPE_NR){
        return s_nr_tel_at_channel[phoneId];
    }else if (getModemType() == MODEM_TYPE_LTE){
        return s_lte_tel_at_channel[phoneId];
    }else{
        return s_w_tel_at_channel[phoneId];
    }
}

int telSendAt(int phoneId, char* cmd, char* buf, unsigned int buf_len, int wait)
{
    int fd = -1;
    int ret = -1;
    char *path = NULL;

    LOGD("telSendAt: phoneId = %d, cmd = %s", phoneId, cmd);
    path = getTelChannelPath(phoneId);
    if (path == NULL) {
        LOGE("get tel channel path fail!");
        return -1;
    }

    LOGD("get tel channel return: %s", path);
    fd = open(path, O_RDWR);
    if (fd == -1) {
        LOGD("open tel channel fail!");
        return -1;
    }

    ret = tel_send_at(fd, cmd, buf, buf_len, wait);
    if (ret == -1){
        LOGE("tel send at fail: ret = %d", ret);
    }

    close(fd);

    return ret;
}

int tel_send_at(int fd, char* cmd, char* buf, unsigned int buf_len, int wait)
{
    struct timeval timeout;
    int ret = -1;
    int cmd_len = 0;
    int rsp_len = 0;
    char* line = NULL;
    fd_set readfs;

    if(NULL == cmd) {
        LOGD("error param");
        return -1;
    }

    LOGD("[fd:%d] >>>> at_cmd: %s", fd, cmd);
    cmd_len = strlen(cmd);
    ret = write(fd, cmd, cmd_len);
    if(ret != cmd_len) {
        LOGE("mmitest write err, ret=%d, cmd_len=%d", ret, cmd_len);
        return -1;
    }
    write(fd, "\r", 1);

    if(wait <= 0) wait = 5;

    for(;;) {
        timeout.tv_sec = wait;
        timeout.tv_usec = 0;
        FD_ZERO(&readfs);
        FD_SET(fd, &readfs);
        ret = select(fd+1, &readfs, NULL, NULL, &timeout);
        if (ret < 0) {
            LOGE("mmitest select err");
            if(errno == EINTR || errno == EAGAIN) {
                continue;
            } else {
                return -1;
            }
        } else if(ret == 0) {
            LOGD("mmitest select time out");
            return -1;
        } else {
            /* go to recv response*/
            break;
        }
    }

    for(;;) {
        line = eng_readline(fd);
        LOGD("mmitest %s [fd:%d] <<<< at_rsp: %s",cmd, fd, line);
        if(strstr(line, "OK"))
        {
            ret = rsp_len;
            break;
        } else if(strstr(line, "ERROR")){
            ret = -1;
            break;
        }else {
            if(buf_len == 0 || buf == NULL) {
                continue;
            }
            if(rsp_len + strlen(line) > buf_len) {
                LOGD("mmitest recv too many word, (%d) > (%d)",
                         (rsp_len + strlen(line)), buf_len);
                ret = -1;
                break;
            }
            memcpy(buf+rsp_len, line, strlen(line));
            rsp_len += strlen(line);
        }
    }
    return ret;
}

int initModemType(){
    int i = 0;
    char modem_prop[512];
    char property[PROPERTY_VALUE_MAX];
    int fd = -1;

    //Add for 5G
    modem_ctrl_int_modem_type();

    if (getModemType() == MODEM_TYPE_NR){
	snprintf(modem_count_prop, sizeof(modem_count_prop), PROP_MODEM_PHONE_COUNT);
	property_get(modem_count_prop, modem_prop, "");
    }else if (getModemType() == MODEM_TYPE_LTE){
	snprintf(modem_count_prop, sizeof(modem_count_prop), PROP_MODEM_PHONE_COUNT);
	property_get(modem_count_prop, modem_prop, "");
    }else{
	snprintf(modem_count_prop, sizeof(modem_count_prop), PROP_MODEM_W_COUNT);
	property_get(modem_count_prop, modem_prop, "");
    }

    s_modem_count = atoi(modem_prop);
    LOGD("mmitest get %s = %d", modem_count_prop, s_modem_count);
    if(s_modem_count > 0 && s_modem_count <= MAX_MODEM_COUNT) {
        for(i = 0; i < s_modem_count; i++) {
            while(1) {
                char *path = getTelChannelPath(i);
                if(path == NULL) break;
                fd = open(path, O_RDWR);
                LOGD("open %s = %d",path,fd);
                if(fd > 0) {
                    //Maybe cause fd leak.
                    close(fd);
                    break;
                }
                usleep(1000);
            }
        }
    } else {
        return -1;
    }

    return 0;
}

static void detect_modem_control()
{
    int control_fd = -1;
    int  numRead;
    char buf[128];

reconnect:
    LOGD("try to connect socket %s...", SOCKET_NAME_MODEM_CTL);

    do
    {
        usleep(10 * 1000);
        control_fd = socket_local_client(SOCKET_NAME_MODEM_CTL,
        ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
        LOGD("connect socket %s control_fd=%d", SOCKET_NAME_MODEM_CTL, control_fd);
    }
    while(control_fd < 0);
    LOGD("connect socket %s success", SOCKET_NAME_MODEM_CTL);

    do
    {
        memset(buf, 0, sizeof(buf));
        LOGD("Monioring modem state on socket %s...", SOCKET_NAME_MODEM_CTL);
        do
        {
            numRead = read(control_fd, buf, sizeof(buf));
            LOGD("read %d from fd %d", numRead, control_fd);
        }
        while(numRead < 0 && errno == EINTR);

        if(numRead <= 0)
        {
            close(control_fd);
            goto reconnect;
        }

        LOGD("read numRead=%d, buf=%s", numRead, buf);
        if (strstr(buf, "Modem Alive"))
        {
            break;
        }else if(strstr(buf, "Modem State: Alive")){
            break;
	 }
    }
    while(1);

    close(control_fd);
    return;
}

static int factrory_parse_cmdline(char * cmdvalue)
{
    int fd = 0, ret = 0;
    char cmdline[CMDLINE_SIZE] = {0};
    char *str = NULL;
    int val;

    if(cmdvalue == NULL)
    {
        LOGD("cmd_value = NULL");
        return -1;
    }
    fd = open("/proc/cmdline", O_RDONLY);
    if (fd >= 0)
    {
        if ((ret = read(fd, cmdline, sizeof(cmdline) - 1)) > 0)
        {
            cmdline[ret] = '\0';
            LOGD("cmdline %s", cmdline);
            str = strstr(cmdline, "modem=");
            if ( str != NULL)
            {
                str += (sizeof("modem=") - 1);
                *(strchr(str, ' ')) = '\0';
            }
            else
            {
                LOGD("cmdline 'modem=' is not exist");
                goto ERROR;
            }
            LOGD("cmdline len = %d, str=%s", strlen(str), str);
            if(!strcmp(cmdvalue, str))
                val = 1;
            else
                val = 0;
            close(fd);
            return val;
        }
        else
        {
            LOGD("cmdline modem=NULL");
            goto ERROR;
        }
    }
    else
    {
        LOGD("/proc/cmdline open error");
        return 0;
    }
ERROR:
    close(fd);
    return 0;
}

void* modem_init_func(void *)
{
    char* at_rsp = NULL;
    int pos = 0;
    int i = 0;
    char tmp[512] = {0};
    char* ptmp = NULL;
    int sim_state;
    pthread_detach(pthread_self());     //free by itself

    //This code will be block,if modem not ready.
    if(factrory_parse_cmdline("shutdown")){
        detect_modem_control();
    }

    if (initModemType() == -1){
        LOGD("init modem type fail!");
        return NULL;
    }
    pthread_mutex_lock(&tel_mutex);
    if(sendATCmd(0, "AT", NULL, 0, 0) < 0) return NULL;
    switch(s_modem_count) {
        case 2:
            if(sendATCmd(0, "AT+SMMSWAP=0", NULL, 0, 0) < 0) return NULL;
            break;
        case 3:
            if(sendATCmd(0, "AT+SMMSWAP=1", NULL, 0, 0) < 0) return NULL;
            break;
    }

    if((pos = sendATCmd(0, "AT+CGMR", s_modem_ver, sizeof(s_modem_ver), 0)) < 0) return NULL;
    s_modem_ver[pos] = '\0';
    LOGD("get modem version[%d]: %s",strlen(s_modem_ver), s_modem_ver);

    if((pos = sendATCmd(0, "AT+SGMR=1,0,3,0", s_cali_info, sizeof(s_cali_info), 0)) < 0) return NULL;
    strcat(s_cali_info,"BIT");
    s_cali_info[pos+3] = '\0';
    LOGD("get cali info[%d]: %s",strlen(s_cali_info), s_cali_info);


    if((pos = sendATCmd(0, "AT+SGMR=1,0,3,1", s_cali_info1, sizeof(s_cali_info1), 0)) < 0) return NULL;
    strcat(s_cali_info1,"BIT");
    s_cali_info1[pos+3] = '\0';
    LOGD("get cali info1[%d]: %s",strlen(s_cali_info1), s_cali_info1);

    if(getModemType() == MODEM_TYPE_LTE){
        if((pos = sendATCmd(0, "AT+SGMR=1,0,3,3,1", s_cali_info2, sizeof(s_cali_info2), 0)) < 0) return NULL;
        strcat(s_cali_info2,"BIT");
        s_cali_info2[pos+3] = '\0';
        LOGD("get cali info2[%d]: %s", strlen(s_cali_info2), s_cali_info2);
    }else if(getModemType() == MODEM_TYPE_NR){
        if((pos = sendATCmd(0, "AT+SGMR=1,0,3,3,1", s_cali_info2, sizeof(s_cali_info2), 0)) < 0) return NULL;
        //strcat(s_cali_info2,"BIT");
        s_cali_info2[pos] = '\0';
        LOGD("get cali info2[%d]: %s", strlen(s_cali_info2), s_cali_info2);
	 //Add for 5G
        if((pos = sendATCmd(0, "AT+SGMR=1,0,3,4", s_cali_info_nr, sizeof(s_cali_info_nr), 0)) < 0) return NULL;
        //strcat(s_cali_info_nr,"BIT");
        s_cali_info_nr[pos] = '\0';
        LOGD("get cali s_cali_info_nr[%d]: %s", strlen(s_cali_info_nr), s_cali_info_nr);
    }
    if((pos = sendATCmd(0, "AT+CGSN", imei_buf1, sizeof(imei_buf1), 0)) < 0) return NULL;

    imei_buf1[pos] = '\0';
    LOGD("get imei[%d]: %s", strlen(imei_buf1), imei_buf1);

    if(s_modem_count == 2){
        if((pos = sendATCmd(1, "AT+CGSN", imei_buf2, sizeof(imei_buf2), 0)) < 0) return NULL;

        imei_buf2[pos] = '\0';
        LOGD("get imei[%d]: %s", strlen(imei_buf2), imei_buf2);
    }
    if(getModemType() != MODEM_TYPE_LTE && getModemType() != MODEM_TYPE_NR){
        for(i = 0; i < s_modem_count; i++) {
                if(sendATCmd(i, "AT+SFUN=2", NULL, 0, 10)<0){
                    LOGD("AT+SFUN=2 open sim card %d failed",i);
                }
        }
    }

    if(getModemType() == MODEM_TYPE_LTE || getModemType() == MODEM_TYPE_NR){
        if((pos = sendATCmd(0, "AT+SPCAPABILITY=51,0", tmp, sizeof(tmp), 0)) >= 0) {
            tmp[pos] = '\0';
        }
        ptmp = tmp;
        ptmp = strstr(ptmp, "SPCAPABILITY");
        if (ptmp) {
                int value = -1;
                eng_tok_start(&ptmp);
                eng_tok_nextint(&ptmp, &value);
                if (value == 51){
                  value = -1;
                    eng_tok_nextint(&ptmp, &value);
                    if (value == 0){
                        value = -1;
                        eng_tok_nextint(&ptmp, &value);
                        s_modem_conf = value;
                        LOGD("s_modem_conf = %d",s_modem_conf);
                    }
                }
        }
    }
    //support cdma2000
    if(s_modem_conf == 32){
           if((pos = sendATCmd(0, "AT+SGMR=0,0,3,2", s_cali_info_cdma2000, sizeof(s_cali_info_cdma2000), 0)) < 0) return NULL;
           strcat(s_cali_info_cdma2000,"BIT");
           s_cali_info_cdma2000[pos+3] = '\0';
           LOGD("get cali s_cali_info_cdma2000[%d]: %s", strlen(s_cali_info_cdma2000), s_cali_info_cdma2000);
        //support cdma2000 end.
    }

    init_mfalg = 1;
    pthread_mutex_unlock(&tel_mutex);

    return NULL;
}

/*
AT+SPCAPABILITY=51,0
return:
+SPCAPABILITY: 51,0,lte_product_mode

lte_product_mode:
    0: L+G (5mod)
    1: L+W (5mod)
    2: L+L (5mod)
    4: WW (5mod)
    8: L+W (5mod)
    16: L+L (5mod)
    32: L+L+C (6mod)
*/

int test_modem_getlteconf(void)
{
    return s_modem_conf;
}

char* test_modem_get_ver(void)
{
    return s_modem_ver;
}

char* test_modem_get_caliinfo(void)
{
    return s_cali_info;
}

void* sim_check_thread(void *)
{
    int i;
    char* at_rsp = NULL;
    char tmp[512];
    char* ptmp = NULL;
    int sim_state;
    int test_result = 0;
    int cur_row = 5;
    int ret = RL_FAIL;

    for(i = 0; i < s_modem_count; i++) {
        if(thread_run != 1) return NULL;
        if(sendATCmd(i, "AT+EUICC?", tmp, sizeof(tmp), 0) < 0) {
            sim_state = -1;
        } else {
            ptmp = tmp;
            ptmp = strstr(ptmp, "EUICC");
            LOGD("sim_check_thread line =%s", ptmp);
            if(ptmp){
                eng_tok_start(&ptmp);
                eng_tok_nextint(&ptmp, &sim_state);
            }else{
                sim_state = -1;
            }
        }
        LOGD("get sim%d, state=%d", i, sim_state);
        ui_set_color(CL_WHITE);
        cur_row = ui_show_text(cur_row+1, 0, sim_name[i]);
        if(sim_state == 0) {
            ui_set_color(CL_GREEN);
            cur_row = ui_show_text(cur_row, 0, TEXT_PASS);
            test_result++;
        } else {
            ui_set_color(CL_RED);
            cur_row = ui_show_text(cur_row, 0, TEXT_FAIL);
        }
    }
    s_sim_state = test_result;
    //update
    if(test_result != s_modem_count) {
        ret = RL_FAIL; //fail
        ui_set_color(CL_RED);
        cur_row = ui_show_text(cur_row+1, 0, TEXT_TEST_FAIL);
    } else {
        ret = RL_PASS; //pass
        ui_set_color(CL_GREEN);
        cur_row = ui_show_text(cur_row+1, 0, TEXT_TEST_PASS);
    }
    ui_push_result(ret);
    ui_clear_rows(4, 1);
    ui_set_color(CL_WHITE);
    ui_show_text(4, 0, TEXT_SIM_RESULT);
    gr_flip();
    sleep(1);
    return NULL;
}

int test_sim_pretest_common(void)
{
    int i;
    char tmp[512];
    char* ptmp = NULL;
    int sim_state;
    int test_result = 0;
    int ret = RL_FAIL;

    for(i = 0; i < s_modem_count; i++) {
        if(sendATCmd(i, "AT+EUICC?", tmp, sizeof(tmp), 0) < 0) {
            sim_state = -1;
        } else {
            ptmp = tmp;
            ptmp = strstr(ptmp, "EUICC");
            LOGD("test_sim_pretest   line =%s", ptmp);
            if(ptmp){
                eng_tok_start(&ptmp);
                eng_tok_nextint(&ptmp, &sim_state);
            }else{
                sim_state = -1;
            }
        }
        LOGD("mmitest get sim%d, state=%d", i, sim_state);
        if(sim_state == 0) {
            test_result++;
        }
    }
    s_sim_state = test_result;

    if(s_sim_state != s_modem_count) {
        ret= RL_FAIL;
    } else {
        ret= RL_PASS;
    }
    save_result(CASE_TEST_SIMCARD,ret);
    return ret;
}

int test_sim_start_common(void)
{
    int ret = 0;
    pthread_t t;
    char property[PROPERTY_VALUE_MAX];
    property_get(PROP_MODEM_LTE_ENABLE, property, "not_find");

    ui_fill_locked();
    ui_show_title(MENU_TEST_SIMCARD);
    if((!strcmp(property, "1")) && (0 == init_mfalg) ){   //7731 should send init sim card AT first
        ui_set_color(CL_WHITE);
        ui_show_text(2,0,TEXT_MODEM_INITING);
        gr_flip();
        ret=RL_NA;
        sleep(1);
    }else{
        ui_set_color(CL_WHITE);
        ui_show_text(2, 0, TEXT_SIM_SCANING);
        gr_flip();
        thread_run = 1;
        pthread_create(&t, NULL, sim_check_thread, NULL);
        ret = ui_handle_button(NULL,NULL,NULL);
        thread_run = 0;
        pthread_join(t, NULL);
    }

    save_result(CASE_TEST_SIMCARD,ret);

    usleep(500 * 1000);
    return ret;
}
