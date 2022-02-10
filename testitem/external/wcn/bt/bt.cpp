#include "common.h"
#include "chnmgr.h"
#include "modem.h"
#include <stdlib.h>

#define MAX_SUPPORT_RMTDEV_NUM 12
#define MAX_REMOTE_DEVICE_NAME_LEN (18)
#define BD_ADDR_LEN_HEX (6)
#define BD_ADDR_LEN_STRING (18)

typedef struct bdremote_t{
    unsigned char addr_u8[BD_ADDR_LEN_HEX];
    char addr_str[BD_ADDR_LEN_STRING];
    char name[MAX_REMOTE_DEVICE_NAME_LEN];
    int rssi_val;
}bdremote_t;

int notify_bt_data = 0;
static bdremote_t data;
static int rest_result=0;

static int stop_test_flag = 0;

#define TEXT_BT_FACTORY_CALL "BT scan devices"
#define TEXT_BTFACTORY "BTFACTORY"
static enum BT_FACTORY_CMD_INDEX {
    BT_FACTORY_TEST_NONE = 0,
    BT_FACTORY_TEST_START,
    BT_FACTORY_TEST_STOP,
};
#define BT_TEST_TIMEOUT 30

#define  MAX_SUPPORT_RMTDEV_NUM 12

int getBTData(bdremote_t *pEvent)
{
    LOGD("FT: getBTData................");
    if(rest_result == RL_PASS){
        LOGD("FT: getBTDat already pass!");
        return 1;
    }
    memset(&data, 0, sizeof(bdremote_t));
    memcpy(&data, pEvent, sizeof(bdremote_t));
    LOGD("FT: getBTData addr_u8=%s",data.addr_u8);
    LOGD("FT: getBTData addr_str=%s",data.addr_str);
    LOGD("FT: getBTData name=%s",data.name);
    LOGD("FT: getBTData rssi_val=%d",data.rssi_val);
    LOGD("FT: getBTData found BT device!!");
    notify_bt_data = 1;
    rest_result = RL_PASS;
    return 0;
}

static void *test_bt_start_thread (void *)
{
    //char buff[256] = {0};
    char *buff = (char *)malloc (2048);
    char cmd[256] = {0};
    int ret = -1;
    //Register callback
    LOGD("FT: add callback: %x", getBTData);
    chnl_fw_ptrFunc_add(TEXT_BT_FACTORY_CALL, (void **)(&getBTData));

    //Start test
    LOGD("FT: start_bt_test: open");
    snprintf(cmd, sizeof(cmd), "%s=%s,%d", TEST_AT_BT_TEST_FLAG,TEXT_BTFACTORY,BT_FACTORY_TEST_START);
    LOGD("FT: start_bt_test: open cmd=%s",cmd);
    ret = chnl_send(CHNL_AT,cmd, strlen(cmd), buff, 2048);
    LOGD("FT: start_bt_test: open ret=%d",ret);
    free(buff);
    return NULL;
}

static void *test_bt_stop_thread (void *)
{
    char buff[256] = {0};
    char cmd[256] = {0};
    int ret = -1;
    //Stop test
    LOGD("FT: stop_bt_test: open");
    snprintf(cmd, sizeof(cmd),  "%s=%s,%d", TEST_AT_BT_TEST_FLAG,TEXT_BTFACTORY,BT_FACTORY_TEST_STOP);
    LOGD("FT: stop_bt_test: open cmd=%s",cmd);
    ret = chnl_send(CHNL_AT,cmd, strlen(cmd), buff, sizeof(buff));
    LOGD("FT: stop_bt_test: open ret=%d",ret);

    flush_wcn_log();
    return NULL;
}

static void *test_bt_thread (void *)
{
    int n = 0;
    //char buff[256] = {0};
    int ret = -1;
    //char cmd[256] = {0};
    notify_bt_data = 0;

    time_t start_time,now_time;
    start_time=time(NULL);
    LOGD("test_bt_thread mmitest here while notify_bt_data=%d",notify_bt_data);
    do {
        now_time=time(NULL);
        if (notify_bt_data){
            ret = 1;
        }
        usleep(10*1000);
        if((now_time-start_time) >= BT_TEST_TIMEOUT) {
            LOGD("test_bt_thread BT_TEST_TIMEOUT!");
            //Stop test
            if(stop_test_flag == 0){
                rest_result = RL_FAIL;
                pthread_t stop_thread;
                pthread_create(&stop_thread, NULL, test_bt_stop_thread, NULL);
                stop_test_flag = 1;
            }
            break;
        }
    } while (ret != 1);
    return NULL;
}

int test_bt_pretest_extern(void)
{
    int ret;
    rest_result = RL_FAIL;
    save_result(CASE_TEST_BT,rest_result);
    stop_test_flag = 0;
    pthread_t thread;
    pthread_t start_thread;
    pthread_t stop_thread;
    LOGD("enter test_bt_pretest_extern");
    pthread_create(&thread, NULL, test_bt_thread, NULL);
    pthread_create(&start_thread, NULL, test_bt_start_thread, NULL);
    pthread_join(thread, NULL); /* wait "handle key" thread exit. */
    ret= rest_result;
    LOGD("FT: test_bt_pretest_extern  ret =%d",ret);
    sleep(1);
    //pthread_create(&stop_thread, NULL, test_bt_stop_thread, NULL);
    if(stop_test_flag == 0){
       rest_result = RL_FAIL;
       pthread_create(&stop_thread, NULL, test_bt_stop_thread, NULL);
       stop_test_flag = 1;
    }
    sleep(2);
    save_result(CASE_TEST_BT,ret);
    return ret;
}

int test_bt_start_extern(void)
{
    int ret;
    stop_test_flag = 0;
    pthread_t thread;
    pthread_t start_thread;
    pthread_t stop_thread;
    LOGD("enter");
    char cmd[256] = {0};
    rest_result = RL_FAIL;
    ui_fill_locked();
    ui_show_title(MENU_TEST_BT);
    ui_set_color(CL_WHITE);
    ui_show_text(2, 0, TEXT_BT_SCANING);
    gr_flip();

    pthread_create(&thread, NULL, test_bt_thread, NULL);
    pthread_create(&start_thread, NULL, test_bt_start_thread, NULL);
    pthread_join(thread, NULL); /* wait "handle key" thread exit. */

    if(rest_result == RL_PASS)
    {
        snprintf(cmd, sizeof(cmd), "%02X:%02X:%02X:%02X:%02X:%02X",data.addr_u8[0],data.addr_u8[1],data.addr_u8[2],data.addr_u8[3],data.addr_u8[4],data.addr_u8[5]);
        LOGD("FT: getBTDatabt  mac =%s",cmd);
        ui_set_color(CL_GREEN);
        ui_show_text(3, 0, cmd);
        ui_set_color(CL_WHITE);
        ui_show_text(4, 0, data.name);
        gr_flip();
        ui_set_color(CL_GREEN);
        ret = 0;
        ui_show_text(5, 0, TEXT_TEST_PASS);
    }
    else
    {
        ret = -1;
        ui_set_color(CL_RED);
        ui_show_text(5, 0, TEXT_TEST_FAIL);
    }
    gr_flip();
    sleep(2);
    save_result(CASE_TEST_BT, rest_result);
    if(stop_test_flag == 0){
       rest_result = RL_FAIL;
       pthread_create(&stop_thread, NULL, test_bt_stop_thread, NULL);
       stop_test_flag = 1;
    }
    LOGD("exit");
    return ret;
}