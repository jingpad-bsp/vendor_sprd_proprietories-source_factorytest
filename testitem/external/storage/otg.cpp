#include "common.h"
#include "chnmgr.h"

static int test_result = -1;
static int cur_row = 2;
#define TEST_TIMEOUT 30
static int thread_run = 0;
static int scaning_row = 0;

static int test_otg_extern()
{
    int ret = -1;

    char cmd[256] = {0};
    char buff[256] = {0};
    snprintf(cmd, sizeof(cmd),  "%s", TEST_AT_OTG_TEST_FLAG );
    LOGD("FT: test_otg_extern:  cmd=%s",cmd);
    ret = chnl_send(CHNL_AT, cmd, strlen(cmd), buff, sizeof(buff));
    LOGD("FT: test_otg_extern:  ret=%d",ret);

    return ret;
}

static void *test_timeout_thread (void *)
{
    int n = TEST_TIMEOUT;
    char buff[256] = {0};
    scaning_row = cur_row;
    do {
	 snprintf(buff, sizeof(buff), "%s, %ds", TEXT_OTG_UDISK_SCANING, n--);
	 ui_clear_rows(scaning_row, 1);
	 ui_set_color(CL_GREEN);
        ui_show_text(scaning_row, 0, buff);
        gr_flip();
        usleep(1000 * 1000);
    } while (n > 0 && thread_run);
    if(thread_run){
	   LOGD("test_timeout_thread exit!");
	   ui_clear_rows(scaning_row, 1);
	   ui_set_color(CL_RED);
	   ui_show_text(scaning_row, 0, TEXT_TEST_TIMEOUT);
	   gr_flip();
	   ui_push_result(RL_NA);
	   test_result = 0;
    }else{
	   ui_clear_rows(scaning_row, 1);
    }
    return NULL;
}

static void *test_otg_thread (void *)
{
    thread_run = 1;
    test_result = test_otg_extern();
    LOGD("test_otg_thread test_result=%d",test_result);
    thread_run = 0;
    if(test_result == RL_PASS){
	   ui_clear_rows(scaning_row, 1);
	   ui_set_color(CL_GREEN);
	   ui_show_text(cur_row + 2, 0, TEXT_TEST_PASS);
	   gr_flip();
    }else{
          ui_clear_rows(scaning_row, 1);
	   ui_set_color(CL_RED);
	   ui_show_text(cur_row + 2, 0, TEXT_TEST_FAIL);
	   gr_flip();
    }
    return NULL;
}

int test_otg_start_extern(void)
{
    int ret;
    cur_row = 2;
    pthread_t thread,thread_timeout;

    ui_fill_locked();
    ui_show_title(MENU_TEST_OTG);
    ui_set_color(CL_WHITE);
    cur_row = ui_show_text(cur_row, 0, OTG_TEST_START);
    gr_flip();

    //Start test
    pthread_create(&thread_timeout, NULL, test_timeout_thread, NULL);
    usleep(1000 * 1000);	
    pthread_create(&thread, NULL, test_otg_thread, NULL);

    pthread_join(thread_timeout, NULL); /* wait "handle key" thread exit. */
    //Start test end

    usleep(100*1000);
    if(test_result == RL_PASS){
	    ret = ui_handle_button(TEXT_PASS, NULL,TEXT_FAIL);
    }else{
	    ret = ui_handle_button(NULL, NULL,TEXT_FAIL);
    }
    save_result(CASE_TEST_OTG,ret);

    usleep(500 * 1000);
    return ret;
}
