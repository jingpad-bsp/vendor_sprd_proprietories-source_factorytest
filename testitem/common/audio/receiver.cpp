#include "common.h"
#include "chnmgr.h"
#include "audiocomm.h"

static sem_t g_sem;
static char t_mode = AUDIO_DEVICE_OUT_EARPIECE;

int SendAudioTestCmd(const uchar * cmd,int bytes)
{
    int fd = -1;
    int ret=-1;
    int bytes_to_read = bytes;

    if(cmd==NULL){
        return -1;
    }

    if(fd < 0) {
        fd = open(AUDCTL, O_WRONLY | O_NONBLOCK);
        if (fd < 0) {
            fd = open(AUDIO_EXT_DATA_CONTROL_PIPE, O_WRONLY | O_NONBLOCK);
        }
    }

    if(fd < 0) {
        return -1;
    }else{
        do {
            ret = write(fd, cmd, bytes);
            if( ret > 0) {
                if(ret <= bytes) {
                    bytes -= ret;
                }
            }else if((!((errno == EAGAIN) || (errno == EINTR))) || (0 == ret)) {
                LOGE("pipe write error ,bytes read is %d",bytes_to_read - bytes);
                break;
            }else {
                LOGD("pipe_write_warning ret is %d",ret);
            }
        }while(bytes);
    }

    if(fd >= 0) {
        close(fd);
    }

    if(bytes == bytes_to_read)
        return ret ;
    else
        return (bytes_to_read - bytes);
}

void* receiver_thread_default(void *)
{
    char write_buf[1024] = {0};
    char filepath[1024] = SPRD_AUDIO_FILE;

    if(AUDIO_DEVICE_OUT_EARPIECE == t_mode ){
#ifdef AUDIO_DRIVER_2
        snprintf(write_buf, sizeof(write_buf) - 1,"out_devices_test=%d;test_stream_route=0x1;samplerate=48000;channels=2;filepath=%s;",AUDIO_DEVICE_OUT_EARPIECE,filepath);
#else
        snprintf(write_buf, sizeof(write_buf) - 1,"out_devices_test=1;test_stream_route=%d;samplerate=44100;channels=2;filepath=%s;",AUDIO_DEVICE_OUT_EARPIECE,filepath);
#endif
        LOGD("write:%s", write_buf);
        SendAudioTestCmd((const uchar *)write_buf,strlen(write_buf));
    }else{
        LOGD("not receiver type t_mode=%d", t_mode);
	  return NULL;
    }

    sem_wait(&g_sem);
    memset(write_buf,0,sizeof(write_buf));
    snprintf(write_buf,sizeof(write_buf) - 1,"out_devices_test=0");
    LOGD("write:%s", write_buf);
    SendAudioTestCmd((const uchar *)write_buf,strlen(write_buf));

    return NULL;
}

void* receiver_thread(void *)
{
    int ret = -1;
    char buff[64] = {0};
    LOGD("sendAT: %s", TEST_AT_AUDIO_RECEIVER_OPEN);
    ret = chnl_send(CHNL_AT, TEST_AT_AUDIO_RECEIVER_OPEN, strlen(TEST_AT_AUDIO_RECEIVER_OPEN), buff, sizeof(buff));
    if (ret == -1)
    {
        LOGD("send AT fail");
        return NULL;
    }
    sem_wait(&g_sem);
    ret = chnl_send(CHNL_AT, TEST_AT_AUDIO_RECEIVER_CLOSE, strlen(TEST_AT_AUDIO_RECEIVER_CLOSE), buff, sizeof(buff));
    LOGD("send AT return: %d", ret);
    return NULL;
}

int test_receiver_start_common(void)
{
    int ret = 0;
    pthread_t t1;
    int row = 2;

    sem_init(&g_sem, 0, 0);
    ui_fill_locked();
    ui_show_title(MENU_TEST_RECEIVER);
    ui_set_color(CL_WHITE);
    ui_show_text(row, 0, TEXT_RECV_PLAYING);
    gr_flip();

    if(chnl_is_support(TEST_AT_AUDIO_RECEIVER_OPEN))
        pthread_create(&t1, NULL, receiver_thread, &t_mode);
    else
        pthread_create(&t1, NULL, receiver_thread_default, &t_mode);

    usleep(10 * 1000);
    ret = ui_handle_button(TEXT_PASS, NULL, TEXT_FAIL); //, TEXT_GOBACK
    sem_post(&g_sem);

    pthread_join(t1, NULL); /* wait "handle key" thread exit. */
    save_result(CASE_TEST_RECEIVER, ret);

    usleep(500 * 1000);
    return ret;
}
