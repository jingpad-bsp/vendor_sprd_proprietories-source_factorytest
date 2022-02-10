#include "common.h"
#include "chnmgr.h"
#include <hardware/sensors.h>
#include <hardware/sensors-base.h>


static int ENABLE_CALI = 1;
static int IS_AUTO_TEST = 0;
static int SENSOR_TYPE = SENSOR_TYPE_LIGHT;
static int SENSOR_TEST_RESULT = CASE_CALI_LSOR;
static char* TEST_TITLE = "Light Sensor";
static char* TEST_MENU = MENU_CALI_LSOR;
static char* TEST_TEXT_OPER = TEXT_LSENSOR_OPENING;

static int test_result = 0;
static int thread_run = 0;
#define SENSOR_TEST_TIMEOUT 60

static int do_cali_work(void ){
    char buf[64];
    char buff[64] = {0};
    char *cmd = buf;
    LOGD("do_cali_work mmitest");
    int ret = -1;
    ret = snprintf(cmd, sizeof(buf), "%s%d,%d,%d", TEST_AT_SENSOR_CALI, ENABLE_CALI , IS_AUTO_TEST , SENSOR_TYPE);
    LOGD("do_cali_work mmitest ret=%d,SENSOR_TYPE=%d",ret,SENSOR_TYPE);
    if(ret < 0){
           return -1;
    }
    LOGD("do_cali_work send cmd cmd=%s",cmd);
    ret = chnl_send(CHNL_AT, cmd, strlen(cmd), buff, sizeof(buff));
    LOGD("do_cali_work mmitest ret2=%d",ret);
    return ret;
}

static void *test_timeout_thread (void *)
{
    time_t start_time,now_time;
    start_time=time(NULL);

    do {
        now_time=time(NULL);
        //LOGD("test_timeout_thread mmitest here while now_time=%d,start_time=%d",  now_time,start_time);
        if((now_time-start_time) >= SENSOR_TEST_TIMEOUT) {
	     test_result = -1;
	     thread_run = 0;
            LOGD("test_timeout_thread SENSOR_TEST_TIMEOUT! test_result==0");
            break;
        }
        usleep(20 * 1000);
    } while (thread_run);
    LOGD("test_timeout_thread exit!");
    return NULL;
}

static void *test_sensor_cali_thread (void *)
{
    test_result = do_cali_work();
    thread_run = 0;
    LOGD("test_msensor_cali_thread exit! test_result=%d",test_result);
    return NULL;
}

int cali_lsensor_start_extern(void)
 {

    int ret = 0;
    int row = 2;
    int test_ret = RL_NA;
	pthread_t thread,thread_timeout;
    char * ptr = TEST_TITLE;

    ui_fill_locked();
    ui_show_title(TEST_MENU);
    ui_set_color(CL_WHITE);

    ui_clear_rows(row, 2);
    ui_set_color(CL_WHITE);
    row = ui_show_text(row, 0, TEXT_SENSOR_DEV_INFO);
    row = ui_show_text(row, 0, ptr);
    gr_flip();

    ui_set_color(CL_GREEN);
    row = ui_show_text(row, 0, TEST_TEXT_OPER);
    gr_flip();

    //work start
    //ret = do_cali_work();
	pthread_create(&thread, NULL, test_sensor_cali_thread, NULL);
	thread_run = 1;
	pthread_create(&thread_timeout, NULL, test_timeout_thread, NULL);

	pthread_join(thread_timeout, NULL); /* wait "handle key" thread exit. */
	//pthread_join(thread, NULL); /* wait "handle key" thread exit. */

	LOGD("test_result=%d",test_result);
	ret = test_result;
       sleep(2);

       if(ret == 0){
               ui_set_color(CL_GREEN);
               ui_show_text(row+1, 0, TEXT_CALI_PASS);
               test_ret = RL_PASS;
       }else{
               ui_set_color(CL_RED);
               ui_show_text(row+1, 0, TEXT_CALI_FAIL);
               test_ret = RL_FAIL;
       }
       gr_flip();
       sleep(2);
       //Work end
       LOGD("%s test_ret=%d",__FUNCTION__ , test_ret);
        save_result(SENSOR_TEST_RESULT, test_ret);
        return ret;
 }
