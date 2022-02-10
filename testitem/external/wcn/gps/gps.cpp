#include "common.h"
#include "chnmgr.h"
#include "modem.h"

int notify_gps_data = 0;
static int test_result=0;
int gps_number = 0;
static int display_gps_snr=0;
static int cur_row = 2;

/*int getWIFIData(bdremote_t *pEvent)
{
    LOGD("FT: getWIFIData................");
    memcpy(&data, pEvent, sizeof(bdremote_t));
    notify_data = 1;
    return 0;
}*/

static void *test_gps_thread (void *)
{
    int n = 0;
    //char buff[256] = {0};
    char *buff = (char *)malloc (2048);
    //char *display_buff;
    int ret = -1;
    notify_gps_data = 0;
    char *p;
    char *delims={ "," };
    char text_buff[56] = {0};
    int count = 1;

    //Start test
    LOGD("FT: test_gps_thread: open");
    ret = chnl_send(CHNL_AT,TEST_AT_GPS_TEST_FLAG, strlen(TEST_AT_GPS_TEST_FLAG), buff, 2048);
    LOGD("test_gps_thread mmitest here while ret=%d",ret);
    /*do {
        LOGD("test_wifi_thread mmitest here while notify_wifi_data=%d",notify_wifi_data);
        if (notify_wifi_data){
        ret = 1;
        }
        usleep(1000*1000);
    } while (ret != 1);*/
    LOGD("%s: display_gps_snr=%d",__FUNCTION__, display_gps_snr);
    if(display_gps_snr){
        //display_buff = strdup(buff);
        LOGD("FT: %s: display_buff=%s",__FUNCTION__, buff);
        p = strtok(buff,delims);
	 if(p != NULL){
		memset(text_buff, 0, sizeof(text_buff));
              snprintf(text_buff, sizeof(text_buff),  "%s%d=%s", "GPS SNR", count ++ , p);
		LOGD("FT: %s: word=%s", __FUNCTION__, text_buff);
		ui_set_color(CL_GREEN);
		cur_row = ui_show_text(cur_row, 0, text_buff);
		gr_flip();
	 }
        while(p!=NULL){
	     p = strtok(NULL,delims);
	     if(p != NULL)
	     {
		memset(text_buff, 0, sizeof(text_buff));
              snprintf(text_buff, sizeof(text_buff),  "%s%d=%s", "GPS SNR", count ++ , p);
              LOGD("FT: %s: word=%s", __FUNCTION__, text_buff);
              ui_set_color(CL_GREEN);
              cur_row = ui_show_text(cur_row, 0, text_buff);
              gr_flip();
	     }
        }
    }
    gps_number = ret;
    if(ret > 0){
        test_result = RL_PASS;
    }else{
        test_result = RL_FAIL;
    }
    LOGD("%s: test_result1 =%d",__FUNCTION__, test_result);
    free(buff);
    return NULL;
}

int test_gps_start_extern(void)
{
FUN_ENTER;
    LOGD("enter");
    int ret = 0;
    int row = 2;
    char buffer[64];
    cur_row = 2;

    display_gps_snr = 1;
    ui_fill_locked();
    ui_show_title(MENU_TEST_GPS);
    ui_set_color(CL_WHITE);
    cur_row = ui_show_text(cur_row, 0, TEXT_WAIT_TIPS);
    gr_flip();

    pthread_t thread;
    pthread_create(&thread, NULL, test_gps_thread, NULL);
    pthread_join(thread, NULL); /* wait "handle key" thread exit. */
    usleep(100);

    flush_wcn_log();
    LOGD("test_gps_start_extern  test_result=%d",test_result);
    if(test_result == RL_PASS)
    {
        memset(buffer, 0, sizeof(buffer));
        snprintf(buffer, sizeof(buffer), "GPS NUM: %d",gps_number);
        cur_row = ui_show_text(cur_row, 0, buffer);
        ui_set_color(CL_GREEN);
        ret = 0;
        cur_row  = ui_show_text(cur_row, 0, TEXT_TEST_PASS);
    }
    else
    {
        ret = -1;
        ui_set_color(CL_RED);
        cur_row = ui_show_text(cur_row, 0, TEXT_TEST_FAIL);
    }
    gr_flip();
    usleep(3000*1000);
    save_result(CASE_TEST_GPS, test_result);
FUN_EXIT;
    return ret;
}

int test_gps_pretest_extern(void)
{
    pthread_t thread;
    test_result = RL_FAIL;
    display_gps_snr = 0;
    save_result(CASE_TEST_GPS,test_result);
    pthread_create(&thread, NULL, test_gps_thread, NULL);
    pthread_join(thread, NULL); /* wait "handle key" thread exit. */
    usleep(100);

    LOGD("pretest:sPreTest test_result= %d,gps_number=%d", test_result,gps_number);
    save_result(CASE_TEST_GPS,test_result);
    return test_result;
}

