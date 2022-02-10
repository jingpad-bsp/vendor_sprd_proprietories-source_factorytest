#include "common.h"
#include "chnmgr.h"
#include "modem.h"

 void * fm_test_thread(void*)
{
    int ret = -1;

    char cmd[256] = {0};
    char buff[256] = {0};
    snprintf(cmd, sizeof(cmd),  "%s", TEST_AT_FM_TEST_FLAG);
    LOGD("%s, cmd=%s",__LINE__, cmd);
    ret = chnl_send(CHNL_AT,cmd, strlen(cmd), buff, sizeof(buff));
    LOGD("%s,  ret=%d",ret);
    return NULL;
}

int test_fm_start_extern(void)
{
FUN_ENTER;
    LOGD("enter");
    int ret = 0;
    int row = 2;
    char buffer[64];

    ui_fill_locked();
    ui_show_title(MENU_TEST_FM);
    ui_set_color(CL_WHITE);
    row = ui_show_text(row, 0, TEXT_WAIT_TIPS);
    gr_flip();

    pthread_t thread;
    pthread_create(&thread, NULL, fm_test_thread, NULL);
    ret = ui_handle_button(TEXT_PASS,NULL,TEXT_FAIL);//, TEXT_GOBACK
    pthread_join(thread, NULL); /* wait "handle key" thread exit. */
    usleep(100);

    LOGD("test_gps_start_extern  test_result=%d",ret);
    gr_flip();
    usleep(2000*1000);
    save_result(CASE_TEST_FM, ret);
FUN_EXIT;
    return ret;
}