#include "common.h"
#include "chnmgr.h"
#include "modem.h"

enum call_option
{
    CALL_END,
    CALL_START,
    cal_option_NUMBER,
};

static char* sim_name[] = {
    TEXT_SIM1,
    TEXT_SIM2
};

#define SIM_CARD1 1
#define SIM_CARD2 2

#define CALL_NUMBER_10086 "10086"
#define CALL_NUMBER_112 "112"

static int start_call_ret = -1;

static bool isLteOnly(void){
    char testmode[PROPERTY_VALUE_MAX+1];
    property_get("persist.vendor.radio.ssda.testmode", testmode, "0");
    LOGD("isLteOnly: persist.vendor.radio.ssda.testmode=%s\n",testmode);

    if (!strcmp(testmode, "3")) { // if persist.vendor.radio.ssda.testmode=3, then modem is lte only
        return true;
    }else {
        return false;
    }
}

static int test_call_number(int call_option,char *call_number)
{
    int ret = -1;
    char cmd[256] = {0};
    char buff[256] = {0};
    LOGD("FT: test_call_number call_option : %d,call_number: %s",call_option , call_number);
    snprintf(cmd, sizeof(cmd),  "%s=%d,%d,\"%s\"", TEST_AT_TEL_TEST_FLAG, call_option ,SIM_CARD1, call_number);
    LOGD("FT: test_call_number: cmd=%s",cmd);
    ret = chnl_send(CHNL_AT,cmd, strlen(cmd), buff, sizeof(buff));
    LOGD("FT: test_call_number: ret=%d",ret);
    return ret;
}

static int test_sim_state(int sim_card)
{
    int ret = -1;
    char cmd[256] = {0};
    char buff[256] = {0};
    LOGD("FT: test_sim_state sim_card: %d" , sim_card);
    snprintf(cmd, sizeof(cmd),  "%s=%d", TEST_AT_SIM_TEST_FLAG ,sim_card);
    LOGD("FT: test_call_number: cmd=%s",cmd);
    ret = chnl_send(CHNL_AT,cmd, strlen(cmd), buff, sizeof(buff));
    LOGD("FT: test_sim_state: ret=%d",ret);
    return ret;
 }

int test_sim_start_extern(void)
{
    int ret = -1;
    int ret_sim1 = -1,ret_sim2 = -1;
    int cur_row = 2;

    ui_fill_locked();
    ui_show_title(MENU_TEST_SIMCARD);

    ui_set_color(CL_WHITE);
    cur_row = ui_show_text(cur_row, 0, TEXT_SIM_SCANING);
    gr_flip();

    //SIM1
    ret_sim1 = test_sim_state(SIM_CARD1);
    LOGD("FT: test_sim_start_extern: ret_sim1=%d",ret_sim1);
    usleep(500 * 1000);
    ui_set_color(CL_WHITE);
    cur_row = ui_show_text(cur_row+1, 0, sim_name[0]);
    gr_flip();
    if(ret_sim1 == 0) {
        ui_set_color(CL_GREEN);
        cur_row = ui_show_text(cur_row, 0, TEXT_PASS);
    } else {
        ui_set_color(CL_RED);
        cur_row = ui_show_text(cur_row, 0, TEXT_FAIL);
    }
    gr_flip();
    //SIM2
    ret_sim2 = test_sim_state(SIM_CARD2);
    LOGD("FT: test_sim_start_extern: ret_sim2=%d",ret_sim2);
    usleep(500 * 1000);
    ui_set_color(CL_WHITE);
    cur_row = ui_show_text(cur_row+1, 0, sim_name[1]);
    gr_flip();
    if(ret_sim2 == 0) {
        ui_set_color(CL_GREEN);
        cur_row = ui_show_text(cur_row, 0, TEXT_PASS);
    } else {
        ui_set_color(CL_RED);
        cur_row = ui_show_text(cur_row, 0, TEXT_FAIL);
    }

    if(ret_sim2 == 0 && ret_sim1 == 0){
        ret = RL_PASS;
    }else{
        ret = RL_FAIL;
    }
    gr_flip();
    usleep(500 * 1000);
    LOGD("FT: test_sim_start_extern end: ret=%d",ret);
    save_result(CASE_TEST_SIMCARD,ret);
    if(ret == RL_PASS)
    {
       ui_set_color(CL_GREEN);
       ui_show_text(cur_row, 0, TEXT_TEST_PASS);
    }
    else
    {
       ui_set_color(CL_RED);
       ui_show_text(cur_row, 0, TEXT_TEST_FAIL);
    }
    gr_flip();
    LOGD("FT: test_sim_start_extern end: ret=%d",ret);
    save_result(CASE_TEST_SIMCARD, ret);
    usleep(1000 * 1000);
    return ret;
}

int test_sim_pretest_extern()
{
    int ret = RL_FAIL;
    int ret_sim1 = -1,ret_sim2 = -1;
    //SIM1
    ret_sim1 = test_sim_state(SIM_CARD1);
    LOGD("FT: test_sim_pretest_extern: ret_sim1=%d",ret_sim1);
    //SIM2
    ret_sim2 = test_sim_state(SIM_CARD2);
    LOGD("FT: test_sim_pretest_extern: ret_sim2=%d",ret_sim2);
    if(ret_sim2 == 0 && ret_sim1 == 0){
        ret = RL_PASS;
    }
    LOGD("FT: test_sim_pretest_extern end: ret=%d",ret);
    save_result(CASE_TEST_SIMCARD, ret);
    return ret;
}

static void *test_starttelcall_thread (void *arg)
{
    int ret = -1;
    if (isLteOnly()) {
        ret = test_call_number(CALL_START, CALL_NUMBER_10086);
    }else{
        ret = test_call_number(CALL_START, CALL_NUMBER_112);
    }
    LOGD("FT: test_starttelcall_thread teling: ret=%d",ret);
    start_call_ret = ret;
    return NULL;
}

static void *test_endtelcall_thread (void *arg)
{
    int ret = -1;
    if (isLteOnly()) {
        ret = test_call_number(CALL_END, CALL_NUMBER_10086);
    }else{
        ret = test_call_number(CALL_END, CALL_NUMBER_112);
    }
    LOGD("FT: test_endtelcall_thread ret=%d",ret);
    return NULL;
}

int test_tel_start_extern(void)
{
    int cur_row = 2;
    int ret = -1;
    start_call_ret = -1;

    ui_fill_locked();
    ui_show_title(MENU_TEST_TEL);
    ui_set_color(CL_GREEN);
    if (isLteOnly()) {
        cur_row = ui_show_text(cur_row, 0, TEL_TEST_START_LTE);
    }else{
        cur_row = ui_show_text(cur_row, 0, TEL_TEST_START);
    }
    cur_row = ui_show_text(cur_row, 0, TEL_TEST_TIPS);
    gr_flip();

    //start call
    /*if (isLteOnly()) {
        ret = test_call_number(CALL_START, CALL_NUMBER_10086);
    }else{
        ret = test_call_number(CALL_START, CALL_NUMBER_112);
    }*/
    pthread_t thread;
    pthread_create(&thread, NULL, test_starttelcall_thread, NULL);
    pthread_join(thread, NULL); /* wait "handle key" thread exit. */

    LOGD("FT: test_tel_start_extern teling: ret=%d",ret);
    ret = start_call_ret;
    if(ret == 0){
        cur_row = ui_show_text(cur_row, 0, TEL_DIAL_OVER);
        gr_flip();
        ret = ui_handle_button(TEXT_PASS, NULL, TEXT_FAIL);
    }
    //end call
    /*if (isLteOnly()) {
        test_call_number(CALL_END, CALL_NUMBER_10086);
    }else{
        test_call_number(CALL_END, CALL_NUMBER_112);
    }*/
    pthread_t thread_endcall;
    pthread_create(&thread_endcall, NULL, test_endtelcall_thread, NULL);
    pthread_join(thread_endcall, NULL); /* wait "handle key" thread exit. */
    if(ret == RL_PASS)
    {
       ui_set_color(CL_GREEN);
       ui_show_text(cur_row, 0, TEXT_TEST_PASS);
    }
    else
    {
       ui_set_color(CL_RED);
       ui_show_text(cur_row, 0, TEXT_TEST_FAIL);
    }
    gr_flip();
    LOGD("FT: test_tel_start_extern end: ret=%d",ret);
    sleep(1);
    save_result(CASE_TEST_TEL, ret);
    return ret;
}
