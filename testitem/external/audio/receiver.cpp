#include "common.h"
#include "chnmgr.h"

static sem_t g_sem;
static char t_mode = AUDIO_DEVICE_OUT_EARPIECE;
void* receiver_thread_extern(void *)
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

int test_receiver_start_extern(void)
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

    pthread_create(&t1, NULL, receiver_thread_extern, &t_mode);

    usleep(10 * 1000);
    ret = ui_handle_button(TEXT_PASS, NULL, TEXT_FAIL); //, TEXT_GOBACK
    sem_post(&g_sem);

    pthread_join(t1, NULL); /* wait "handle key" thread exit. */
    save_result(CASE_TEST_RECEIVER, ret);

    usleep(500 * 1000);
    return ret;
}
