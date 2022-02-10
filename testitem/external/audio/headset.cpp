#include "common.h"
#include "chnmgr.h"
#include "modem.h"
static int headset_status = -1;
static int headset_mic_status = -1;
static int headset_status_event = -1;
static int headset_key = -1;
static int headset_key_event = -1;
static int thread_run;
void* headset_check_thread_extern(void *)
{
    int ret = -1;
    int type = 0;
    char buff[64] = {0};
    int row = 2;
    int headset_key_count = 0;
    int key = -1;
    char str[128];

    while(thread_run == 1)
    {
        //step1.check headset insert
        ret = chnl_send(CHNL_AT, TEST_AT_HEADSET_TYPE, strlen(TEST_AT_HEADSET_TYPE), buff, sizeof(buff));
        if (ret == -1)
        {
            LOGE("send AT fail");
            ui_push_result(RL_FAIL);
            return NULL;
        }
        else
        {
            type = atoi((char *)buff);
            switch (type)
            {
            case 0:
                LOGD("headset not insert");
                row = 2;
                ui_clear_rows(row, 6);
                ui_set_color(CL_RED);
                row = ui_show_text(row, 0, TEXT_HD_UNINSERT);
                gr_flip();
                //setPlugState(1);
                usleep(10 * 1000);
                continue;
            case 1:
                LOGD("found 4P headset");
                row = 2;
                ui_clear_rows(row, 1);
		  gr_flip();
                ui_set_color(CL_GREEN);
                row = ui_show_text(row, 0, TEXT_HD_INSERTED);
                row = ui_show_text(row, 0, TEXT_HD_HAS_MIC);
		  gr_flip();
                ui_set_color(CL_WHITE);
                row = ui_show_text(row, 0, TEXT_HD_MICHD);
                gr_flip();
                //chnl_callback(TEST_AUDIO_CALLBACK_DESC);        //callback function, for send AT
                //Register callback
                LOGD("FT: add callback: %x", sendATCmd);
                chnl_fw_ptrFunc_add(TEST_AUDIO_CALLBACK_DESC, (void **)(&sendATCmd));
                chnl_send(CHNL_AT, TEST_AT_AUDIO_HEADSET_LOOP_OPEN, strlen(TEST_AT_AUDIO_HEADSET_LOOP_OPEN), buff, sizeof(buff));
                usleep(10 * 1000);
                //setPlugState(0);
                break;
            case 2:
                LOGD("found 3P headset");
                row = 2;
                ui_clear_rows(row, 1);
		  gr_flip();
                ui_set_color(CL_GREEN);
                row = ui_show_text(row, 0, TEXT_HD_INSERTED);
                row = ui_show_text(row, 0, TEXT_HD_NO_MIC);
		  gr_flip();
                ui_set_color(CL_WHITE);
                row = ui_show_text(row, 0, TEXT_HD_MICHD);
                gr_flip();
                //chnl_callback(TEST_AUDIO_CALLBACK_DESC);        //callback function, for send AT
                //Register callback
                LOGD("FT: add callback: %x", sendATCmd);
                chnl_fw_ptrFunc_add(TEST_AUDIO_CALLBACK_DESC, (void **)(&sendATCmd));
                chnl_send(CHNL_AT, TEST_AT_AUDIO_HEADSET_LOOP_OPEN, strlen(TEST_AT_AUDIO_HEADSET_LOOP_OPEN), buff, sizeof(buff));
                usleep(10 * 1000);
                //setPlugState(0);
                break;
            default:
                LOGD("unknow type:%d", type);
                continue;
            }

            //step2.check headset key
            while(thread_run == 1)
            {
                LOGD("headset key test, count = %d", headset_key_count);
                ui_clear_rows(row, 1);
                snprintf(str, sizeof(str), "%s%d", TEXT_HD_KEY_TIMES, headset_key_count);
                ui_set_color(CL_WHITE);
                ui_show_text(row, 0, str);
                gr_flip();
                if(headset_key_count >= 2)
                {
                    LOGD("headset test pass");
                    ui_set_color(CL_GREEN);
                    ui_show_text(row + 1, 0, TEXT_PASS);
                    gr_flip();
                    ui_push_result(RL_PASS);
                    chnl_send(CHNL_AT, TEST_AT_AUDIO_HEADSET_LOOP_CLOSE, strlen(TEST_AT_AUDIO_HEADSET_LOOP_CLOSE), buff, sizeof(buff));
                    return NULL;
                }
                if(wait_headset_key() == 1)
                    headset_key_count++;
            }
        }
    }

    chnl_send(CHNL_AT, TEST_AT_AUDIO_HEADSET_LOOP_CLOSE, strlen(TEST_AT_AUDIO_HEADSET_LOOP_CLOSE), buff, sizeof(buff));

    return NULL;
}

int test_headset_start_extern(void)
{
    FUN_ENTER;
    int ret = 0;
    pthread_t t1;
    ui_fill_locked();
    ui_show_title(MENU_TEST_HEADSET);
    gr_flip();

    thread_run = 1;

    pthread_create(&t1, NULL, headset_check_thread_extern, NULL);

    //setPlugState(1);
    ret = ui_handle_button(NULL, NULL, TEXT_FAIL);
    usleep(200*1000);
    thread_run = 0;

    pthread_join(t1, NULL);

    save_result(CASE_TEST_HEADSET, ret);
    //setPlugState(0);

    usleep(500 * 1000);
    FUN_EXIT;
    return ret;
}
