#include "common.h"
#include "chnmgr.h"
#include "modem.h"

int notify_wifi_data = 0;
static int rest_result=0;
static int display_wifi_mac = 0;
static int cur_row = 2;
static int MAX_DISPLAY_COUNT = 5;

/*int getWIFIData(bdremote_t *pEvent)
{
    LOGD("FT: getWIFIData................");
    memcpy(&data, pEvent, sizeof(bdremote_t));
    notify_data = 1;
    return 0;
}*/

static void *test_wifi_thread (void *)
{
    int n = 0;
    char buff[2048] = {0};
    int ret = -1;
    notify_wifi_data = 0;

    char *p;
    char *buffer;
    char *delims={ "\n" };

    //Start test
    LOGD("FT: test_wifi_thread: open!!!");
    ret = chnl_send(CHNL_AT,TEST_AT_WIFI_TEST_FLAG, strlen(TEST_AT_WIFI_TEST_FLAG), buff, sizeof(buff));
    LOGD("test_wifi_thread mmitest here while ret=%d",ret);
    /*do {
        LOGD("test_wifi_thread mmitest here while notify_wifi_data=%d",notify_wifi_data);
        if (notify_wifi_data){
        ret = 1;
        }
        usleep(1000*1000);
    } while (ret != 1);*/
    if(display_wifi_mac){
        buffer = strdup(buff);
        LOGD("FT: test_wifi_thread: buffer=%s",buffer);
        p = strtok(buffer,delims);
        int count = 0;
        while(p!=NULL && count < MAX_DISPLAY_COUNT){
            p = strtok(NULL,delims);
            LOGD("FT: test_wifi_thread: word=%s",p);
            ui_set_color(CL_GREEN);
            cur_row = ui_show_text(cur_row, 0, p);
            gr_flip();
            count ++;
        }
        if(buffer != NULL){
            free(buffer);
        }
    }
    usleep(1000*1000);
    if(ret != -1)
        rest_result = RL_PASS;
    else
        rest_result = RL_FAIL;
    return NULL;
}

int test_wifi_start_extern(void)
{
FUN_ENTER;
    LOGD("enter");
    int ret = 0;
    display_wifi_mac = 1;
    cur_row = 2;
    ui_fill_locked();
    ui_show_title(MENU_TEST_WIFI);
    ui_set_color(CL_WHITE);
    cur_row = ui_show_text(cur_row, 0, TEXT_BT_SCANING);
    gr_flip();

    pthread_t thread;
    pthread_create(&thread, NULL, test_wifi_thread, NULL);
    pthread_join(thread, NULL); /* wait "handle key" thread exit. */
    usleep(100);

    flush_wcn_log();
    LOGD("test_wifi_start_extern  rest_result=%d",rest_result);
    if(rest_result == RL_PASS)
    {
        ui_set_color(CL_GREEN);
        ret = 0;
        cur_row = ui_show_text(cur_row, 0, TEXT_TEST_PASS);
    }
    else
    {
        ret = -1;
        ui_set_color(CL_RED);
        cur_row = ui_show_text(cur_row, 0, TEXT_TEST_FAIL);
    }
    gr_flip();
    usleep(1000*1000);
    save_result(CASE_TEST_WIFI, rest_result);
FUN_EXIT;
    return ret;
}

//------------------------------------------------------------------------------
int test_wifi_pretest_extern(void)
{
    LOGD("enter");
    rest_result = RL_FAIL;
    save_result(CASE_TEST_WIFI, rest_result);
    display_wifi_mac = 0;
    int ret = -1;
    pthread_t thread;
    pthread_create(&thread, NULL, test_wifi_thread, NULL);
    pthread_join(thread, NULL); /* wait "handle key" thread exit. */
    usleep(100);
    ret = rest_result;
    usleep(1000*1000);
    save_result(CASE_TEST_WIFI, rest_result);
    LOGD("end");
    return ret;
}
