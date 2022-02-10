#include "common.h"
#include "chnmgr.h"
#include <hardware/sensors.h>
#include <hardware/sensors-base.h>
#include<math.h>

static int thread_run;
static int notify_sensor_data = 0;
static sensors_event_t data;
static int cur_row = 2;
//static int col = 6;
#define SENSOR_TEST_TIMEOUT 60

#define TEXT_SENSOR_FACTORY_CALL "get sensor data"
static int x_row, y_row, z_row, open_row;
static int x_pass, y_pass, z_pass;

static int ENABLE_SENSOR = 1;
static int DISABLE_SENSOR = 0;
static int SENSOR_TYPR = SENSOR_TYPE_GYROSCOPE;

static pthread_mutex_t msinfo_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t msinfo_cond = PTHREAD_COND_INITIALIZER;

static time_t begin_time,over_time;
static int sensor_id;
static int gsensor_result=0;
static float x_value, y_value, z_value;
static int first_time = 0;
static int col = 6;

//enable:1;disable:0
static int enable_sensor(int enable,int type)
{
    int ret = -1;

    char cmd[256] = {0};
    char buff[256] = {0};
    LOGD("FT: enable_sensor enable: %d,type=%d",enable,type);
    snprintf(cmd, sizeof(cmd),  "%s=%d,%d", TEST_AT_SENSOR_TEST_FLAG, enable, type);
    LOGD("FT: enable_sensor: close cmd=%s",cmd);
    ret = chnl_send(CHNL_AT,cmd, strlen(cmd), buff, sizeof(buff));
    LOGD("FT: enable_sensor: close ret=%d",ret);

    return ret;
}

static int gsensor_check(int datas)
{
	int ret = -1;

	int start_1 = SPRD_GSENSOR_1G-SPRD_GSENSOR_OFFSET;
	int start_2 = -SPRD_GSENSOR_1G+SPRD_GSENSOR_OFFSET;

	if( ((start_1<datas) || (start_2>datas)) ){
		ret = 0;
	}

	return ret;
}

static int getSensorData(sensors_event_t *pEvent)
{
    LOGD("FT: getSensorData................");
    if(gsensor_result == RL_PASS){
        LOGD("FT: getSensorData already pass!");
        return -1;
    }

    pthread_mutex_lock(&msinfo_mtx);		//add lock

    memset(&data, 0, sizeof(sensors_event_t));
    memcpy(&data, pEvent, sizeof(sensors_event_t));
    LOGD("getSensorData mmitest value=<%5.1f,%5.1f,%5.1f>\n",data.data[0], data.data[1], data.data[2]);
    if (SENSOR_TYPR  == data.type)
    {
 	LOGD("mmitest value=<%5.1f,%5.1f,%5.1f>\n",data.data[0], data.data[1], data.data[2]);
 	if(first_time && data.data[0]&&data.data[1]&&data.data[2]){
 		first_time = 0;
 		x_value = data.data[0];
 		y_value = data.data[1];
 		z_value = data.data[2];
 	}
 	if(x_pass == 0 && gsensor_check(data.data[0]) == 0 && x_value != data.data[0]) {
 		x_pass = 1;
 		LOGD("x_pass");
 		ui_show_text(x_row, col, TEXT_SENSOR_PASS);
 		gr_flip();
 	}
 
 	if(y_pass == 0 && gsensor_check(data.data[1]) == 0 && y_value != data.data[1]) {
 		y_pass = 1;
 		LOGD("y_pass");
 		ui_show_text(y_row, col, TEXT_SENSOR_PASS);
 		gr_flip();
 	}
 
 	if(z_pass == 0 && gsensor_check(data.data[2]) == 0 && z_value != data.data[2]) {
 		z_pass = 1;
 		LOGD("z_pass");
 		ui_show_text(z_row, col, TEXT_SENSOR_PASS);
 		gr_flip();
 	}
    }

    pthread_mutex_unlock(&msinfo_mtx);
    //pthread_cond_signal(&msinfo_cond);

    if(x_pass&y_pass&z_pass){
	gsensor_result = RL_PASS;
	notify_sensor_data = 1;
	enable_sensor(DISABLE_SENSOR,SENSOR_TYPR);
    }
    else
	gsensor_result = RL_FAIL;

    return 0;
}

static void *test_sensor_thread (void *)
{
    int n = 0;
    char buff[256] = {0};
    int ret = -1;
    char cmd[256] = {0};
    notify_sensor_data = 0;

    time_t start_time,now_time;
    start_time=time(NULL);

    do {
        now_time=time(NULL);
        if (notify_sensor_data){
            LOGD("test_sensor_thread mmitest here while notify_sensor_data=%d",notify_sensor_data);
            ret = 1;
        }
        if((now_time-start_time) >= SENSOR_TEST_TIMEOUT) {
            LOGD(" SENSOR_TEST_TIMEOUT!",__FUNCTION__);
            break;
        }
     usleep(20 * 1000);
    } while (ret != 1);
    enable_sensor(DISABLE_SENSOR,SENSOR_TYPR);
    LOGD("test_sensor_thread exit!");
    return NULL;
}

int test_gsensor_start_extern(void)
{
    //char buff[256] = {0};
    int ret = -1;
    cur_row = 2;
    LOGD("FT: %s start.....",__FUNCTION__);

    pthread_t thread;
    //pthread_t thread_show;
    LOGD("enter");
    //char cmd[56] = {0};
    first_time = 1;
    notify_sensor_data = 0;
    gsensor_result = RL_FAIL;
    first_time = 1;
    x_pass = 0;
    y_pass = 0;
    z_pass = 0;

    ui_fill_locked();
    ui_show_title(MENU_TEST_GSENSOR);
    ui_set_color(CL_WHITE);
    open_row = cur_row;
    cur_row = ui_show_text(cur_row, 0, TEXT_SENSOR_OPEN);
    gr_flip();

    cur_row = ui_show_text(cur_row, 0, TEXT_GS_OPER1);
    cur_row = ui_show_text(cur_row, 0, TEXT_GS_OPER2);
    ui_set_color(CL_GREEN);
    x_row = cur_row;
    cur_row = ui_show_text(cur_row, 0, TEXT_GS_X);
    y_row = cur_row;
    cur_row = ui_show_text(cur_row, 0, TEXT_GS_Y);
    z_row = cur_row;
    cur_row = ui_show_text(cur_row, 0, TEXT_GS_Z);
    gr_flip();

    thread_run = 1;
    //ret = ui_handle_button(NULL, NULL, TEXT_FAIL);
    pthread_create(&thread, NULL, test_sensor_thread, NULL);
    //pthread_create(&thread_show, NULL, test_sensor_show, NULL);
    usleep(10 * 1000);

    //Register callback
    LOGD("FT: add callback: %x", getSensorData);
    chnl_fw_ptrFunc_add(TEXT_SENSOR_FACTORY_CALL, (void **)(&getSensorData));

    enable_sensor(ENABLE_SENSOR,SENSOR_TYPR);

    pthread_join(thread, NULL); /* wait "handle key" thread exit. */
    thread_run = 0;
    pthread_cond_signal(&msinfo_cond);
    //pthread_join(thread_show, NULL);

    //enable_sensor(0);
    chnl_fw_ptrFunc_remove(TEXT_SENSOR_FACTORY_CALL);

    if(RL_PASS == gsensor_result){
	    ui_set_color(CL_GREEN);
	    ui_show_text(cur_row, 0, TEXT_TEST_PASS);
    }
    else if(RL_FAIL == gsensor_result){
	    ui_set_color(CL_RED);
	    ui_show_text(cur_row, 0, TEXT_TEST_FAIL);
    }else{
	    ui_set_color(CL_WHITE);
	    ui_show_text(cur_row, 0, TEXT_TEST_NA);
    }
    gr_flip();
    save_result(CASE_TEST_GYRSOR,gsensor_result);
    LOGD("exit");
    return ret;
}
