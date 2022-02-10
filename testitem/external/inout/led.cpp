#include "common.h"
#include "chnmgr.h"

typedef enum LED_VALUE
{
    LED_CLOSE_ALL = 0,
    LED_OPEN_ALL = 1,
    LED_CLOSE_RED = 10,
    LED_OPEN_RED = 11,
    LED_CLOSE_GREEN = 20,
    LED_OPEN_GREEN = 21,
    LED_CLOSE_BLUE = 30,
    LED_OPEN_BLUE = 31,
};

typedef enum{
    led_blue = 0,
    led_red = 1,
    led_green = 2,
    color_num
}led_color;

static int thread_run = 0;

static int LedSetValue( int value)
{
    int ret = -1;
    char cmd[256] = {0};
    char buff[256] = {0};
    LOGD("FT: LedSetValue value : %d",value);
    snprintf(cmd, sizeof(cmd),  "%s=%d", TEST_AT_LED_TEST_FLAG, value);
    LOGD("FT: LedSetValue: cmd=%s",cmd);
    ret = chnl_send(CHNL_AT,cmd, strlen(cmd), buff, sizeof(buff));
    LOGD("FT: LedSetValue: ret=%d",ret);
    return ret;
}

static void *led_thread(void *)
{
    int value = 0;
    int pos = 3;
    int loop = led_blue;
    int color = CL_GREEN;
    int open_value = -1;
    char *title;
    while(1 == thread_run) {
        switch(led_color(loop++%color_num)){
        case led_blue:
            //LedSetValue(LED_OPEN_BLUE);
            open_value = LED_OPEN_BLUE;
	     color = CL_BLUE;
	     title = TEXT_LED_BLUE;
            //usleep(1000*1000);
            //LedSetValue(LED_OPEN_BLUE);
            LOGD("get led loop led_blue = %d!", loop);
            break;
        case led_red:
            //LedSetValue(LED_OPEN_RED);
            open_value = LED_OPEN_RED;
	     color = CL_RED;
	     title = TEXT_LED_RED;
            //usleep(1000*1000);
            //LedSetValue(LED_CLOSE_RED);
             LOGD("get led loop led_red = %d!", loop);
            break;
        case led_green:
            //LedSetValue(LED_OPEN_GREEN);
            open_value = LED_OPEN_GREEN;
	     color = CL_GREEN;
	     title = TEXT_LED_GREEN;
            //usleep(1000*1000);

            LOGD("get led loop led_green = %d!", loop);
            break;
        }
	 LOGD("get led title: %s! color = %d!", title, color);
	 LedSetValue(open_value);
	 ui_set_color(color);
        ui_show_text(6, pos, title);
        gr_flip();
	 usleep(1000*1000);
	 LedSetValue(open_value - 1);
        ui_clear_rows(6, 2);
        gr_flip();
    }
    return NULL;
}
int test_led_start_extern(void)
{
    int ret;
    int row = 2;
    pthread_t thead;

    ui_fill_locked();
    ui_show_title(MENU_TEST_LED);
    ui_set_color(CL_GREEN);
    row = ui_show_text(row, 0, TEXT_LED_TIPS);

    gr_flip();

    LOGD("mmitest start");
    thread_run=1;
    pthread_create(&thead, NULL, led_thread, NULL);
    usleep(10*1000);
    ret = ui_handle_button(TEXT_PASS,NULL,TEXT_FAIL);
    thread_run=0;
    pthread_join(thead, NULL);

    LedSetValue(LED_CLOSE_ALL);
    save_result(CASE_TEST_LED,ret);

    usleep(500 * 1000);
    return ret;
}
