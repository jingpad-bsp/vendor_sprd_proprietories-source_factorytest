#include "common.h"
#include "chnmgr.h"
#include <hardware/sensors.h>
#include <hardware/sensors-base.h>

static int notify_sensor_data = 0;
static int test_result=0;
static sensors_event_t data;
static int cur_row = 2;
static int col = 6;
#define SENSOR_TEST_TIMEOUT 60

static int light_value=0;
static int light_pass=0;

#define TEXT_SENSOR_FACTORY_CALL "get sensor data"
static int x_row, y_row, z_row, open_row;
static int x_pass, y_pass, z_pass;

static int ENABLE_SENSOR = 1;
static int DISABLE_SENSOR = 0;

static int flush_value = 0;
static int cnt = 0;
static float proximity_value=1;
static int proximity_modifies=0;

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

static void pxysensor_show_ex()
{
    char buf[64];
    int row = cur_row;

    ui_clear_rows(row, 2);

    if(proximity_modifies >= 2) {
        ui_set_color(CL_GREEN);
    } else {
        ui_set_color(CL_RED);
    }

    if(proximity_value == 0){
        row = ui_show_text(row, 0, TEXT_PS_NEAR);
    } else {
        row = ui_show_text(row, 0, TEXT_PS_FAR);
    }

    /*memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "%s %d", TEXT_LS_LUX, light_value);

    if(light_pass == 1) {
        ui_set_color(CL_GREEN);
    } else {
        ui_set_color(CL_RED);
    }
    cur_row = ui_show_text(row, 0, buf);*/
    gr_flip();

}
static int getSensorData(sensors_event_t *pEvent)
{
    LOGD("FT: getSensorData_lp................");
    if(test_result == RL_PASS){
        LOGD("FT: getSensorData_lp already pass!");
        return -1;
    }

    memset(&data, 0, sizeof(sensors_event_t));
    memcpy(&data, pEvent, sizeof(sensors_event_t));
    ui_clear_rows(open_row, 1);
    ui_set_color(CL_GREEN);
    ui_show_text(open_row, 0, TEXT_PXYSENSOR_OPENING);
    gr_flip();

    LOGD("getSensorData_lp mmitest data.type=%d\n",data.type);
    switch(data.type){
    case SENSOR_TYPE_META_DATA:
        LOGD("data.meta_data.what = %d ,  flush_value = %d, ! %d IN", data.meta_data.what, flush_value, __LINE__);
        if(data.meta_data.what == META_DATA_FLUSH_COMPLETE)
            flush_value++;
        break;
    case SENSOR_TYPE_LIGHT:
        LOGD("mmitest light value=<%5.1f>, flush_value = %d\n",data.light, flush_value);
        if(flush_value >= 0 && light_value != data.light){
            cnt++;
            if(cnt >= 5)
                light_pass = 1;
            light_value = data.light;
            pxysensor_show_ex();
        }
        break;
    case SENSOR_TYPE_PROXIMITY:
        LOGD("mmitest Proximity:%5.1f, flush_value = %d", data.distance, flush_value);
        if(flush_value >= 0 && proximity_value != data.distance){
            proximity_modifies++;
            proximity_value = data.distance;
            pxysensor_show_ex();
        }
        break;
    default:
        LOGD("ERROR DATA.TYPE! %d IN", __LINE__);
        break;
    }

    LOGD("getSensorData_lp light_pass=%d,proximity_modifies = %d", light_pass,proximity_modifies);
    if(proximity_modifies > 2){
        notify_sensor_data = 1;
        test_result = RL_PASS;
        enable_sensor(DISABLE_SENSOR , SENSOR_TYPE_PROXIMITY);
    }else
            test_result = RL_FAIL;
    return 0;
}

static void *test_lpsensor_thread (void *)
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
         LOGD("test_lpsensor_thread mmitest here while notify_sensor_data=%d",notify_sensor_data);
            ret = 1;
        }
        if((now_time-start_time) >= SENSOR_TEST_TIMEOUT) {
            LOGD("test_lpsensor_thread SENSOR_TEST_TIMEOUT!");
            break;
        }
     usleep(20 * 1000);
    } while (ret != 1);
    enable_sensor(DISABLE_SENSOR , SENSOR_TYPE_PROXIMITY);
    LOGD("test_lpsensor_thread exit!");
    return NULL;
}

static void *test_Psensor_thread (void *)
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
         LOGD("test_lpsensor_thread mmitest here while notify_sensor_data=%d",notify_sensor_data);
            ret = 1;
        }
        if((now_time-start_time) >= SENSOR_TEST_TIMEOUT) {
            LOGD("test_lpsensor_thread SENSOR_TEST_TIMEOUT!");
            break;
        }
     usleep(20 * 1000);
    } while (ret != 1);
    enable_sensor(DISABLE_SENSOR , SENSOR_TYPE_PROXIMITY);  
    LOGD("test_Psensor_thread exit!");  
    return NULL;
}

int test_pxysensor_start_extern(void)
{
    char buff[256] = {0};
    int ret = -1;
    cur_row = 2;
    LOGD("FT: test_asensor_start_extern start.....");

    pthread_t thread;
    LOGD("enter");
    char cmd[56] = {0};
    test_result = RL_FAIL;
    flush_value = 0;
    cnt = 0;
    proximity_modifies = 0;
    proximity_value=1;

    ui_fill_locked();
    ui_show_title(MENU_TEST_PXYSENSOR);
    ui_set_color(CL_WHITE);
    open_row = cur_row;
    cur_row = ui_show_text(cur_row, 0, TEXT_SENSOR_OPEN);
    gr_flip();

    //ret = ui_handle_button(NULL, NULL, TEXT_FAIL);
    pthread_create(&thread, NULL, test_Psensor_thread, NULL);
    usleep(10 * 1000);
    //ret = ui_handle_button(NULL,NULL,NULL);//, TEXT_GOBACK
    //Register callback
    LOGD("FT: add callback: %x", getSensorData);
    chnl_fw_ptrFunc_add(TEXT_SENSOR_FACTORY_CALL, (void **)(&getSensorData));

    enable_sensor(ENABLE_SENSOR , SENSOR_TYPE_PROXIMITY);
    
    pthread_join(thread, NULL); /* wait "handle key" thread exit. */

    if(test_result == RL_PASS)
    {
        ui_set_color(CL_GREEN);
        ret = 0;
        ui_show_text(cur_row + 1, 0, TEXT_TEST_PASS);
    }
    else
    {
        ret = -1;
        ui_set_color(CL_RED);
        ui_show_text(cur_row + 1, 0, TEXT_TEST_FAIL);
    }
    gr_flip();
    sleep(1);
    chnl_fw_ptrFunc_remove(TEXT_SENSOR_FACTORY_CALL);
    LOGD("exit");
    LOGD("FT: ret: %d", ret);
    save_result(CASE_TEST_PXYPSOR, test_result);
    return ret;
}
