#include "common.h"
#include "chnmgr.h"
#include <hardware/sensors.h>
#include <hardware/sensors-base.h>
#include<math.h>

static int thread_run;
static int notify_sensor_data = 0;
static int test_result=0;
static sensors_event_t data;
static int cur_row = 2;
//static int col = 6;
#define SENSOR_TEST_TIMEOUT 60
#define DROP_CNT 5
#define TEXT_SENSOR_FACTORY_CALL "get sensor data"
static int x_row, y_row, z_row, open_row;
static int x_pass, y_pass, z_pass;

static int ENABLE_SENSOR = 1;
static int DISABLE_SENSOR = 0;
static int SENSOR_TYPR = SENSOR_TYPE_MAGNETIC_FIELD;

static pthread_mutex_t msinfo_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t msinfo_cond = PTHREAD_COND_INITIALIZER;

static char xyz=0;              //if X,Y,Z axis fit the result
static char msxinfo[64] = "0.0";
static char msyinfo[64] = "0.0";
static char mszinfo[64] = "0.0";
static char disp_flag = 0;

static int first_time = 0, msensor_drop_cnt = 0;
static sensors_event_t first_data;

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

static void *test_sensor_show(void *)
{
    int cur_row;
    char msxinfo[64] = "0.0";
    char msyinfo[64] = "0.0";
    char mszinfo[64] = "0.0";
    char disp_flag = 0;

    while(thread_run)
    {
        memset(msxinfo, 0, sizeof(msxinfo));
        memset(msyinfo, 0, sizeof(msyinfo));
        memset(mszinfo, 0, sizeof(mszinfo));
        LOGD("mmi test msensor data: .....0\n");
        pthread_mutex_lock(&msinfo_mtx);    //display info, lock another thread
        //pthread_cond_wait(&msinfo_cond, &msinfo_mtx);//wait another thread produce data

        snprintf(msxinfo,sizeof(msxinfo), "%s  %5.1f", TEXT_MS_X, data.data[0]);
        snprintf(msyinfo,sizeof(msyinfo), "%s  %5.1f", TEXT_MS_Y, data.data[1]);
        snprintf(mszinfo,sizeof(mszinfo), "%s  %5.1f", TEXT_MS_Z, data.data[2]);
        disp_flag = xyz;

        pthread_mutex_unlock(&msinfo_mtx);  //get data success, unlock another thread
        LOGD("mmi test msensor data: <%s,%s,%s>\n",msxinfo, msyinfo, mszinfo);

        cur_row = x_row;
        ui_clear_rows(cur_row, 3);
        if(disp_flag & 0x01)
            ui_set_color(CL_GREEN);
        else
            ui_set_color(CL_WHITE);
        cur_row = ui_show_text(cur_row, 0, msxinfo);

        if(disp_flag & 0x02)
            ui_set_color(CL_GREEN);
        else
            ui_set_color(CL_WHITE);
        cur_row = ui_show_text(cur_row, 0, msyinfo);

        if(disp_flag & 0x04)
            ui_set_color(CL_GREEN);
        else
            ui_set_color(CL_WHITE);
        cur_row = ui_show_text(cur_row, 0, mszinfo);
        LOGD("mmi test msensor data: ....1\n");
        gr_flip();
        LOGD("mmi test msensor data: .....2\n");

        if(1 == notify_sensor_data || thread_run == 0)
        {
            pthread_mutex_lock(&msinfo_mtx);
            snprintf(msxinfo,sizeof(msxinfo), "%s  %5.1f", TEXT_MS_X, data.data[0]);
            snprintf(msyinfo,sizeof(msyinfo), "%s  %5.1f", TEXT_MS_Y, data.data[1]);
            snprintf(mszinfo,sizeof(mszinfo), "%s  %5.1f", TEXT_MS_Z, data.data[2]);
            pthread_mutex_unlock(&msinfo_mtx);

            disp_flag = xyz;
            cur_row = x_row;
            ui_clear_rows(cur_row, 3);
            if(disp_flag & 0x01)
                ui_set_color(CL_GREEN);
            else
                ui_set_color(CL_WHITE);
            cur_row = ui_show_text(cur_row, 0, msxinfo);

            if(disp_flag & 0x02)
                ui_set_color(CL_GREEN);
            else
                ui_set_color(CL_WHITE);
            cur_row = ui_show_text(cur_row, 0, msyinfo);
            
            if(disp_flag & 0x04)
                ui_set_color(CL_GREEN);
            else
                ui_set_color(CL_WHITE);
            cur_row = ui_show_text(cur_row, 0, mszinfo);

            if (notify_sensor_data == 1){
                ui_set_color(CL_GREEN);
                ui_show_text(12, 0, TEXT_TEST_PASS);
            }else if(thread_run == 0){
                ui_set_color(CL_RED);
                ui_show_text(12, 0, TEXT_TEST_FAIL);
            }
            gr_flip();
            LOGD("mmitest msensor test pass");
            sleep(1);

            ui_push_result(RL_PASS);
         }
    }

    return NULL;
}

static int getSensorData(sensors_event_t *pEvent)
{
    LOGD("FT: getSensorData................");
    if(test_result == RL_PASS){
        LOGD("FT: getSensorData already pass!");
        return -1;
    }

    if (msensor_drop_cnt) {
        msensor_drop_cnt--;
        return 0;
    }
    pthread_mutex_lock(&msinfo_mtx);		//add lock

    memset(&data, 0, sizeof(sensors_event_t));
    memcpy(&data, pEvent, sizeof(sensors_event_t));

    LOGD("getSensorData mmitest value=<%5.1f,%5.1f,%5.1f>\n",data.data[0], data.data[1], data.data[2]);
    if (SENSOR_TYPE_MAGNETIC_FIELD  == data.type)
    {
        int flag = 0;

        if (data.data[0] == 0 && data.data[1] == 0 && data.data[2] == 0){
            flag = 1;
        }

        if (first_time) {
            memset(&first_data, 0, sizeof(sensors_event_t));
            memcpy(&first_data, pEvent, sizeof(sensors_event_t));
            first_time = 0;

            pthread_mutex_unlock(&msinfo_mtx);
            //pthread_cond_signal(&msinfo_cond);

            return 0;
        }

        if(fabsf(first_data.data[0]-data.data[0]) > 20 && flag != 1)     // X axis fit the result
            xyz = xyz | 0x01;

        if(fabsf(first_data.data[1]-data.data[1]) > 20 && flag != 1)     // Y axis fit the result
            xyz = xyz | 0x02;

        if(fabsf(first_data.data[2]-data.data[2]) > 20 && flag != 1)     // Z axis fit the result
            xyz = xyz | 0x04;
    }

    LOGD("getSensorData xyz = %d", xyz);

    pthread_mutex_unlock(&msinfo_mtx);
    //pthread_cond_signal(&msinfo_cond);

    if(xyz == 7){
        notify_sensor_data = 1;
        test_result = RL_PASS;
        enable_sensor(DISABLE_SENSOR,SENSOR_TYPE_MAGNETIC_FIELD);
    }else
        test_result = RL_FAIL;

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
            LOGD("test_asensor_thread SENSOR_TEST_TIMEOUT!");
            break;
        }
     usleep(20 * 1000);
    } while (ret != 1);
    enable_sensor(DISABLE_SENSOR,SENSOR_TYPE_MAGNETIC_FIELD);
    LOGD("test_sensor_thread exit!");
    return NULL;
}

int test_msensor_start_extern(void)
{
    //char buff[256] = {0};
    int ret = -1;
    cur_row = 2;
    LOGD("FT: test_asensor_start_extern start.....");

    pthread_t thread;
    pthread_t thread_show;
    LOGD("enter");
    //char cmd[56] = {0};
    test_result = RL_FAIL;
    first_time = 1;
    msensor_drop_cnt = DROP_CNT;
    test_result = 0;
    notify_sensor_data = 0;
    xyz = 0;

    ui_fill_locked();
    ui_show_title(MENU_TEST_MSENSOR);
    ui_set_color(CL_WHITE);
    open_row = cur_row;
    cur_row = ui_show_text(cur_row, 0, TEXT_SENSOR_OPEN);
    gr_flip();

    cur_row = ui_show_text(cur_row, 0, TEXT_MS_OPER1);
    cur_row = ui_show_text(cur_row, 0, TEXT_MS_OPER2);
    x_row=cur_row;
    gr_flip();

    thread_run = 1;
    //ret = ui_handle_button(NULL, NULL, TEXT_FAIL);
    pthread_create(&thread, NULL, test_sensor_thread, NULL);
    pthread_create(&thread_show, NULL, test_sensor_show, NULL);
    usleep(10 * 1000);

    //Register callback
    LOGD("FT: add callback: %x", getSensorData);
    chnl_fw_ptrFunc_add(TEXT_SENSOR_FACTORY_CALL, (void **)(&getSensorData));

    enable_sensor(ENABLE_SENSOR,SENSOR_TYPE_MAGNETIC_FIELD);

    pthread_join(thread, NULL); /* wait "handle key" thread exit. */
    thread_run = 0;
    pthread_cond_signal(&msinfo_cond);
    pthread_join(thread_show, NULL);

    //enable_sensor(0);
    chnl_fw_ptrFunc_remove(TEXT_SENSOR_FACTORY_CALL);
    save_result(CASE_TEST_MAGSOR,test_result);
    LOGD("exit");
    return ret;
}
