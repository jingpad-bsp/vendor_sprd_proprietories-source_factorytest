#include "common.h"
#include "chnmgr.h"

extern int enable_vibrator_extern(int ms);

int en_key_backlight_extern(int brightness)
{
    int ret = -1;

    char cmd[256] = {0};
    char buff[256] = {0};
    LOGD("%s: brightness = %d", __FUNCTION__,  brightness);
    snprintf(cmd, sizeof(cmd),  "%s=%d", TEST_AT_BACKLIGHT_TEST_FLAG, brightness);
    LOGD("%s cmd=%s", __FUNCTION__ , cmd);
    ret = chnl_send(CHNL_AT,cmd, strlen(cmd), buff, sizeof(buff));
    LOGD("%s ret=%d",__FUNCTION__ , ret);

    return ret;
}

int test_vb_bl_start_extern(void)
{
	int i = 0;
	int ret = 0;
	int row = 2;
	int lcd_max = 255;
	ui_fill_locked();
	ui_show_title(MENU_TEST_VB_BL);
	row = ui_show_text(row, 0, TEXT_BL_ILLUSTRATE);
	row = ui_show_text(row, 0, TEXT_VIB_START);
	gr_flip();

	enable_vibrator_extern(3);
	for(i = 0; i < 5; i++) {
		en_key_backlight_extern(lcd_max>>i);
		usleep(500*1000);
	}
	//Close vibrator
	enable_vibrator_extern(0);
	ui_set_color(CL_GREEN);
	row = ui_show_text(row, 0, TEXT_VIB_FINISH);
	gr_flip();

	en_key_backlight_extern(255);

	usleep(10*1000);
	ret = ui_handle_button(TEXT_PASS,NULL,TEXT_FAIL);

	save_result(CASE_TEST_BACKLIGHT,ret);
	save_result(CASE_TEST_VIBRATOR,ret);

	usleep(500 * 1000);
	return ret;
}
