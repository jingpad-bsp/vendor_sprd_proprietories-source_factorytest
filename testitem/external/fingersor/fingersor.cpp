#include "common.h"
#include "chnmgr.h"

enum fp_test_step
{
    FACTORY_INIT,
    SPI_TEST,
    INTERRRUPT_TEST,
    DEADPIXEL_TEST,
    FINGER_DETECT,
    FACTORY_EXIT,
    STEP_NUMBER,
};

//enable:1;disable:0
static int test_fp_sensor(int step)
{
    int ret = -1;
    char cmd[256] = {0};
    char buff[256] = {0};
    LOGD("FT: test_fp_sensor step: %d",step);
    snprintf(cmd, sizeof(cmd),  "%s=%d", TEST_AT_FINGER_TEST_FLAG,step);
    LOGD("FT: test_fp_sensor: cmd=%s",cmd);
    ret = chnl_send(CHNL_AT,cmd, strlen(cmd), buff, sizeof(buff));
    LOGD("FT: test_fp_sensor: ret=%d",ret);
    return ret;
}

int test_fingersor_start_extern(void)
{
    int ret = RL_NA;
    int ret_exit = RL_NA;
    int row = 2;
    int pressed = 0;
    int times = 100;
    int detect_result = -1;

    char *error;

    ui_fill_locked();
    ui_show_title(MENU_TEST_FINGERSOR);
    ui_set_color(CL_WHITE);
    ui_show_text(row, 0, FINGERSOR_TEST_TIPS);
    gr_flip();

    while(KEY_VIR_NEXT_PAGE != pressed)
    {
        pressed = ui_wait_button(NULL, TEXT_START, NULL);
    }
    ui_set_color(CL_SCREEN_BG);
    gr_fill(0, ui_getTitleHeight(), gr_fb_width(), gr_fb_height());

    ui_set_color(CL_WHITE);
    ui_show_text(row, 0, FINGER_INIT_TIPS);
    gr_flip();

    LOGD("do factory_init");
    ret = test_fp_sensor(FACTORY_INIT);
    if(ret == -1)
    {
        ret = RL_FAIL;
        ui_clear_rows(row, 2);
        ui_set_color(CL_RED);
        row = ui_show_text(row, 0, FINGER_INIT_FAIL);
        gr_flip();
        LOGE("factory_init fail");
        goto end;
    }

    ui_clear_rows(row, 2);
    ui_set_color(CL_GREEN);
    row = ui_show_text(row, 0, FINGER_INIT_PASS);
    gr_flip();

    LOGD("do spi_test");
    ret = test_fp_sensor(SPI_TEST);
    if(ret == -1)
    {
        ret = RL_FAIL;
        LOGE("spi_test fail");
        goto end;
    }

    LOGD("do interrupt_test");
    ret = test_fp_sensor(INTERRRUPT_TEST);
    if(ret == -1)
    {
        ret = RL_FAIL;
        LOGE("interrupt_test fail");
        goto end;
    }

    ui_set_color(CL_WHITE);
    row = ui_show_text(row, 0, FINGERSOR_START_TIPS);
    gr_flip();

    LOGD("do deadpixel_test");
    ret = test_fp_sensor(DEADPIXEL_TEST);
    if(ret == -1)
    {
        ret = RL_FAIL;
        LOGE("deadpixel_test fail");
        goto end;
    }


    LOGD("do finger_detect");
    ret = test_fp_sensor(FINGER_DETECT);
    while(times > 0)
    {
        detect_result = test_fp_sensor(FINGER_DETECT);
        if(detect_result != -1)
        {
            ret = RL_PASS;
            break;
        }
        times--;
        LOGD("not detect fingerprint, try again...");
        usleep(1000*100);
    }
    if(detect_result < 0)
    {
        ret = RL_FAIL;
        LOGE("finger_detect %d times but failed", times);
    }

    LOGD("do factory_exit");
    ret_exit = test_fp_sensor(FACTORY_EXIT);
    if(ret_exit == -1)
    {
	 ret = RL_FAIL;
	 LOGE("factory_exit fail");
        goto end;
    }

end:
    if(ret == RL_PASS) {
        ui_set_color(CL_GREEN);
        ui_show_text(row, 0, TEXT_TEST_PASS);
    } else {
        ui_set_color(CL_RED);
        ui_show_text(row, 0, TEXT_TEST_FAIL);
    }
    gr_flip();

    save_result(CASE_TEST_FINGERSOR,ret);
    usleep(500 * 1000);
    return ret;

}
