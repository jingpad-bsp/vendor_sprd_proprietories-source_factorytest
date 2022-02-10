#include "common.h"
#include "chnmgr.h"
#include "modem.h"
int test_assisloopback_start_extern(void)
{
        char buff[64] = {0};
        int ret = 0;
        int row = 2;

        if(chnl_is_support_callback(TEST_AUDIO_CALLBACK_DESC))
        {
            //chnl_callback(TEST_AUDIO_CALLBACK_DESC);
            //Register callback
            LOGD("FT: add callback: %x", sendATCmd);
            chnl_fw_ptrFunc_add(TEST_AUDIO_CALLBACK_DESC, (void **)(&sendATCmd));

            ui_fill_locked();
            ui_show_title(MENU_TEST_ASSISLOOP);
            ui_clear_rows(row, 2);
            ui_set_color(CL_WHITE);
            row = ui_show_text(row, 0, TEXT_LB_MICRECEIVER);
            gr_flip();

            chnl_send(CHNL_AT, TEST_AT_AUDIO_ASSIS_LOOP_OPEN, strlen(TEST_AT_AUDIO_ASSIS_LOOP_OPEN), buff, sizeof(buff));
            ret = ui_handle_button(TEXT_PASS, NULL, TEXT_FAIL);
            chnl_send(CHNL_AT, TEST_AT_AUDIO_ASSIS_LOOP_CLOSE, strlen(TEST_AT_AUDIO_ASSIS_LOOP_CLOSE), buff, sizeof(buff));

            save_result(CASE_TEST_ASSISLOOP,ret);
            return ret;
        }else{
        LOGE("chnl_is_support_callback not support!");
        }
        LOGD("chnl_is_support_callback not support ret=%d",ret);
        return ret;
}

int test_mainloopback_start_extern(void)
{
        char buff[64] = {0};
        int ret = 0;
        int row = 2;

        if(chnl_is_support_callback(TEST_AUDIO_CALLBACK_DESC))
        {
            //chnl_callback(TEST_AUDIO_CALLBACK_DESC);
            //Register callback
            LOGD("FT: add callback: %x", sendATCmd);
            chnl_fw_ptrFunc_add(TEST_AUDIO_CALLBACK_DESC, (void **)(&sendATCmd));

            ui_fill_locked();
            ui_show_title(MENU_TEST_MAINLOOP);
            ui_set_color(CL_WHITE);

            ui_clear_rows(row, 2);
            ui_set_color(CL_WHITE);
            row = ui_show_text(row, 0, TEXT_LB_MICSPEAKER);
            gr_flip();
            chnl_send(CHNL_AT, TEST_AT_AUDIO_MAIN_LOOP_OPEN, strlen(TEST_AT_AUDIO_MAIN_LOOP_OPEN), buff, sizeof(buff));
            ret = ui_handle_button(TEXT_PASS, NULL, TEXT_FAIL);
            chnl_send(CHNL_AT, TEST_AT_AUDIO_MAIN_LOOP_CLOSE, strlen(TEST_AT_AUDIO_MAIN_LOOP_CLOSE), buff, sizeof(buff));

            save_result(CASE_TEST_MAINLOOP, ret);
            return ret;
        }else{
        LOGE("chnl_is_support_callback not support!");
        }
        LOGD("chnl_is_support_callback not support ret=%d",ret);
        return ret;
}