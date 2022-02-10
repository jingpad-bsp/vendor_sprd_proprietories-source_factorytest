#include "common.h"
#include "chnmgr.h"

int enable_vibrator_extern(int vibrator_time_s)
{
    int ret = -1;

    char cmd[256] = {0};
    char buff[256] = {0};
    LOGD("%s: vibrator_time_s = %d", __FUNCTION__,  vibrator_time_s);
    snprintf(cmd, sizeof(cmd),  "%s=%d", TEST_AT_VIBRATOR_TEST_FLAG, vibrator_time_s);
    LOGD("%s cmd=%s", __FUNCTION__ , cmd);
    ret = chnl_send(CHNL_AT,cmd, strlen(cmd), buff, sizeof(buff));
    LOGD("%s ret=%d",__FUNCTION__ , ret);

    return ret;
}

int test_vibrator_start_extern(void)
{
	int ret = 0;
	int row = 2;
	ui_fill_locked();
	ui_show_title(MENU_TEST_VIBRATOR);
	ui_set_color(CL_WHITE);
	row = ui_show_text(row, 0, TEXT_VIB_START);
	gr_flip();

	enable_vibrator_extern(5);
	usleep(1500*1000);

	ui_set_color(CL_GREEN);
	row = ui_show_text(row, 0, TEXT_VIB_FINISH);
	ui_show_text(row, 0, TEXT_FINISH);
	gr_flip();
	ret = ui_handle_button(TEXT_PASS, NULL,TEXT_FAIL);//, TEXT_GOBACK
	return ret;
}
