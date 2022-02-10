#include "common.h"
#include "chnmgr.h"
#include <hardware/sensors.h>
#include <hardware/sensors-base.h>

static int thread_run;
int notify_sensor_data = 0;
static int test_result=0;
static sensors_event_t data;
static int cur_row = 2;
static int col_data = 5;
static int col = 6;
#define SENSOR_TEST_TIMEOUT 60

#define TEXT_SENSOR_FACTORY_CALL "get sensor data"
static int x_row, y_row, z_row, open_row;
static int x_pass, y_pass, z_pass;

static int ENABLE_SENSOR = 1;
static int DISABLE_SENSOR = 0;
static int SENSOR_TYPR = SENSOR_TYPE_ACCELEROMETER;

static pthread_mutex_t msinfo_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t msinfo_cond = PTHREAD_COND_INITIALIZER;

static int asensor_check_ex(float datas)
{
    int ret = -1;

    float start_1 = SPRD_ASENSOR_1G-SPRD_ASENSOR_OFFSET;
    float start_2 = -SPRD_ASENSOR_1G+SPRD_ASENSOR_OFFSET;

    if( ((start_1<datas) || (start_2>datas)) ){
        ret = 0;
    }

    return ret;
}

//enable:1;disable:0
static int enable_sensor(int enable)
{
    int ret = -1;

    char cmd[256] = {0};
    char buff[256] = {0};
    LOGD("FT: enable_sensor enable: %d",enable);
    snprintf(cmd, sizeof(cmd),  "%s=%d,%d", TEST_AT_SENSOR_TEST_FLAG,enable,SENSOR_TYPR);
    LOGD("FT: enable_sensor: close cmd=%s",cmd);
    ret = chnl_send(CHNL_AT,cmd, strlen(cmd), buff, sizeof(buff));
    LOGD("FT: enable_sensor: close ret=%d",ret);

    return ret;
}

static void *test_sensor_show(void *)
{
    int cur_row;
    char msxinfo[64] = "0.0";
    char msyinfo[64] = "0.0";
    char mszinfo[64] = "0.0";
    char disp_flag = 0;

    ui_clear_rows(open_row, 1);
    ui_set_color(CL_GREEN);
    ui_show_text(open_row, 0, TEXT_AS_OPER1);
    gr_flip();

    while(thread_run)
    {
        memset(msxinfo, 0, sizeof(msxinfo));
        memset(msyinfo, 0, sizeof(msyinfo));
        memset(mszinfo, 0, sizeof(mszinfo));
        LOGD("mmi test msensor data: .....0\n");

     pthread_mutex_lock(&msinfo_mtx);
        snprintf(msxinfo,sizeof(msxinfo), "%s  %5.1f", TEXT_GS_X, data.data[0]);
        snprintf(msyinfo,sizeof(msyinfo), "%s  %5.1f", TEXT_GS_Y, data.data[1]);
        snprintf(mszinfo,sizeof(mszinfo), "%s  %5.1f", TEXT_GS_Z, data.data[2]);
     pthread_mutex_unlock(&msinfo_mtx);

        LOGD("mmi test msensor data: <%s,%s,%s>\n",msxinfo, msyinfo, mszinfo);

        if(x_pass == 1) {
            LOGD("show x_pass");
         ui_set_color(CL_GREEN);
            ui_show_text(x_row, col, TEXT_SENSOR_PASS);
            gr_flip();
        }

        if(y_pass == 1) {
            LOGD("show y_pass");
         ui_set_color(CL_GREEN);
            ui_show_text(y_row, col, TEXT_SENSOR_PASS);
            gr_flip();
        }

        if(z_pass == 1) {
            LOGD("show z_pass");
         ui_set_color(CL_GREEN);
            ui_show_text(z_row, col, TEXT_SENSOR_PASS);
            gr_flip();
        }

        LOGD("test_sensor_show x_pass = %d,y_pass = %d,z_pass = %d", x_pass,y_pass,z_pass);
        if(x_pass & y_pass & z_pass){
                notify_sensor_data = 1;
                test_result = RL_PASS;
        }else
            test_result = RL_FAIL;

        if(1 == notify_sensor_data || thread_run == 0)
        {
            ui_push_result(RL_PASS);
         }
    }

    return NULL;
}

int getSensorData(sensors_event_t *pEvent)
{
    LOGD("FT: getSensorData................");
    if(test_result == RL_PASS){
        LOGD("FT: getSensorData already pass!");
        return -1;
    }

    pthread_mutex_lock(&msinfo_mtx);        //add lock

    memset(&data, 0, sizeof(sensors_event_t));
    memcpy(&data, pEvent, sizeof(sensors_event_t));

    pthread_mutex_unlock(&msinfo_mtx);

    LOGD("getSensorData mmitest value=<%5.1f,%5.1f,%5.1f>\n",data.data[0], data.data[1], data.data[2]);
    if(x_pass == 0 && asensor_check_ex(data.data[0]) == 0) {
        x_pass = 1;
        LOGD("x_pass");
    }

    if(y_pass == 0 && asensor_check_ex(data.data[1]) == 0) {
        y_pass = 1;
        LOGD("y_pass");
    }

    if(z_pass == 0 && asensor_check_ex(data.data[2]) == 0) {
        z_pass = 1;
        LOGD("z_pass");
    }
    return 0;
}

static void *test_asensor_thread (void *)
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
        LOGD("test_asensor_thread mmitest here while notify_sensor_data=%d",notify_sensor_data);
        LOGD("test_asensor_thread mmitest here while now_time=%d,start_time=%d",now_time,start_time);
        if (notify_sensor_data){
            ret = 1;
        }
        if((now_time-start_time) >= SENSOR_TEST_TIMEOUT) {
            LOGD("test_asensor_thread SENSOR_TEST_TIMEOUT!");
            break;
        }
     usleep(20 * 1000);
    } while (ret != 1);
    enable_sensor(0);
    LOGD("test_asensor_thread exit!");
    return NULL;
}

int test_asensor_start_extern(void)
{
    char buff[256] = {0};
    int ret = -1;
    cur_row = 2;
    LOGD("FT: test_asensor_start_extern start.....");

    pthread_t thread;
    pthread_t thread_show;

    LOGD("enter");
    char cmd[56] = {0};
    test_result = RL_FAIL;
    x_pass = 0;
    y_pass = 0;
    z_pass = 0;
    thread_run = 1;

    ui_fill_locked();
    ui_show_title(MENU_TEST_ASENSOR);
    ui_set_color(CL_WHITE);
    open_row = cur_row;
    cur_row = ui_show_text(cur_row, 0, TEXT_SENSOR_OPEN);
    gr_flip();

    LOGD("cur_row = %d", cur_row);
    cur_row = ui_show_text(cur_row, 0, TEXT_AS_OPER2);
    ui_set_color(CL_WHITE);
    x_row = cur_row;
    cur_row = ui_show_text(cur_row, 0, TEXT_GS_X);
    y_row = cur_row;
    cur_row = ui_show_text(cur_row, 0, TEXT_GS_Y);
    z_row = cur_row;
    cur_row = ui_show_text(cur_row, 0, TEXT_GS_Z);
    gr_flip();

    //ret = ui_handle_button(NULL, NULL, TEXT_FAIL);
    pthread_create(&thread, NULL, test_asensor_thread, NULL);
    pthread_create(&thread_show, NULL, test_sensor_show, NULL);
    usleep(10 * 1000);

    //Register callback
    LOGD("FT: add callback: %x", getSensorData);
    chnl_fw_ptrFunc_add(TEXT_SENSOR_FACTORY_CALL, (void **)(&getSensorData));

    enable_sensor(1);

    pthread_join(thread, NULL); /* wait "handle key" thread exit. */
    thread_run = 0;
    pthread_cond_signal(&msinfo_cond);
    pthread_join(thread_show, NULL);

    if(test_result == RL_PASS)
    {
        ui_set_color(CL_GREEN);
        ret = 0;
        ui_show_text(cur_row, 0, TEXT_TEST_PASS);
    }
    else
    {
        ret = -1;
        ui_set_color(CL_RED);
        ui_show_text(cur_row, 0, TEXT_TEST_FAIL);
    }
    gr_flip();
    sleep(1);

    //enable_sensor(0);
    chnl_fw_ptrFunc_remove(TEXT_SENSOR_FACTORY_CALL);
    save_result(CASE_TEST_ACCSOR,test_result);
    LOGD("exit");
    return ret;
}

