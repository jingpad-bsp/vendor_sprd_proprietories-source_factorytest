//
// Spreadtrum Auto Tester
//
// anli   2012-11-10
//

#include <stdlib.h>
#include "common.h"
#include "wifitest.h"
#include "common.h"
#include "wifinew.h"
#include "modem.h"

#include <android/hardware/wifi/1.0/IWifiApIface.h>
#include <android/hardware/wifi/1.0/IWifiStaIface.h>
#include <android/hardware/wifi/1.0/IWifiChip.h>
#include <utils/StrongPointer.h>
using ::android::sp;

static wifi_ap_t sAPs[WIFI_MAX_AP];
static int sStatus = 0;
static int sAPNum = 0;
static int max_ap = 5;      //atmost display max_ap

const char *get_cap_ch = "IFNAME=wlan0 GET_CAPABILITY channels";
const char *remove_network = "IFNAME=wlan0 REMOVE_NETWORK all";
const char *scan = "IFNAME=wlan0 SCAN";
const char *scan_result = "IFNAME=wlan0 SCAN_RESULTS";
const char *terminate = "IFNAME=wlan0 TERMINATE";
const char *results_event = "CTRL-EVENT-SCAN-RESULTS";

const int buffer_len = 4096;
static char wifiinfo[20][200]={0};
static int line = 0;
int wifi5G_flag = 0;

#define  WIFI_DRIVER_MODULE_PATH  "/lib/modules/sprdwl_ng.ko"

//------------------------------------------------------------------------------
int wifiOpen(void) {
FUN_ENTER;
    int ret = 0;
    int flag = -1;
    char cmd[100] = {0};

    sprintf(cmd, "insmod %s", WIFI_DRIVER_MODULE_PATH);
    flag = system(cmd);
    if (!WIFEXITED(flag) || WEXITSTATUS(flag) || -1 == flag) {
        LOGE("%s:insmod /system/lib/modules/sprdwl.ko. flag=%d,err:%s\n",
             __FUNCTION__, flag, strerror(errno));
        return -1;
    }
    LOGE("load wifi driver success\n");
    ret = wifi_start_supplicant(0);
    if (ret < 0) {
        LOGE("wifi_start_supplicant failed\n");
    }
    sleep(2);
    wifi_connect_to_supplicant();
FUN_EXIT;
    return 0;
}

//------------------------------------------------------------------------------
int wifi_ScanAP(void) {
FUN_ENTER;
    int ret = 0;
    static char buffer[buffer_len] = {0};
    unsigned int len = buffer_len;
    char delims1[] = "\n";
    char *result1 = NULL;
    int ii = 0;
    line = 0;

    /* 1. Get channels capability */
    ret = wifi_command(get_cap_ch, buffer, &len);
    if (ret < 0) {
        LOGE("wifi_command failed ret= %d\n", ret);
    } else {
        buffer[len] = '\0';
        LOGD("wifi_command - get_cap_ch, buffer0 = %s,len=%d\n",buffer,len);
        if (NULL != strstr(buffer, "Mode[A]")) {
            wifi5G_flag = 1;
        }
        LOGD("wifi5G_flag=%d\n",wifi5G_flag);
    }

    /* 2. Remove all network */
    memset(buffer, 0, buffer_len);
    len = buffer_len;
    ret = wifi_command(remove_network, buffer, &len);
    if (ret < 0) {
        LOGE("wifi_command failed ret= %d\n", ret);
    } else {
        buffer[len] = '\0';
        LOGD("wifi_command - remove_network, buffer1 = %s,len=%d\n", buffer, len);
    }

    /* 3. Start to scan */
    memset(buffer, 0, buffer_len);
    len = buffer_len;
    ret = wifi_command(scan, buffer, &len);
    if (ret < 0) {
        LOGE("wifi_command failed ret= %d\n", ret);
    } else {
        buffer[len] = '\0';
        LOGD("wifi_command - scan, buffer2 = %s,len=%d\n", buffer, len);
    }

    /* 4. Wait for scan results event */
    ret = wifi_ctrl_recv_event(buffer, buffer_len, &len, results_event, 10000);
    if (ret < 0) {
        LOGE("wifi_ctrl_recv_event fail,  ret=%d\n", ret);
    } else {
        /* 5. Get scan results */
        memset(buffer, 0, buffer_len);
        len = buffer_len;
        ret = wifi_command(scan_result, buffer, &len);
        if (ret < 0) {
            LOGE("wifi_command failed, ret= %d\n", ret);
        } else {
            buffer[len] = '\0';
            LOGD("wifi_command - scan_result, buffer3 = %s,len=%d\n", buffer, len);
            result1 = strtok(buffer, delims1);
            while (result1 != NULL ) {
                LOGD( "line = %d ,result is \"%s\"\n", line, result1 );
                line++;
                if ((line >1) && (line < sizeof(wifiinfo)/sizeof(wifiinfo[0]))) {
                    strcpy(wifiinfo[ii++], result1);
                    LOGD( "wifiinfo[%d]=%s\n", ii - 1, wifiinfo[ii - 1]);
                }
                result1 = strtok(NULL, delims1);
            }
        }
    }

FUN_EXIT;
    return ret;
}

//------------------------------------------------------------------------------
int wifiClose(void) {
 FUN_ENTER;
    int ret = 0;
    char cmd[100] = {0};
    int flag = -1;
    ret=wifi_stop_supplicant(0);
    if (ret < 0) {
        LOGE("wifi_stop_supplicant failed\n");
    }
    wifi_close_supplicant_connection();

    sprintf(cmd, "rmmod %s", WIFI_DRIVER_MODULE_PATH);
    flag = system(cmd);
    if (!WIFEXITED(flag) || WEXITSTATUS(flag) || (-1 == flag)) {
        LOGE("%s:insmod /system/lib/modules/sprdwl.ko. flag=%d,err:%s\n",
              __FUNCTION__, flag, strerror(errno));
        return -1;
    }
    LOGD("wificlose ret=%d\n",ret);
FUN_EXIT;
    return ret;
}

//------------------------------------------------------------------------------
static void wifi_show_result() {
FUN_ENTER;
    int i = 0;
    int row = 2, col = 0;
    char str[1024] = {0};

    ui_clear_rows(row, 2);
    ui_set_color(CL_YELLOW);
    set_render_mode(Render_BOLD);
    row = ui_show_text(row, col, "bssid  freq  signal  ssid");
    set_render_mode(Render_DEFAULT);
    gr_flip();

    for (i = 0; (i < sAPNum) && (i < max_ap); i++) {
        ui_set_color(CL_RED);
        memset(str, 0, sizeof(str));
        sprintf(str, "%s %d", sAPs[i].smac, sAPs[i].frequency);
        LOGD("str=%s\n", str);
        row = ui_show_text(row, col, str);

        ui_set_color(CL_BLUE);
        memset(str, 0, sizeof(str));
        sprintf(str,"%d %s",sAPs[i].sig_level, sAPs[i].name);;
        LOGD("str=%s\n",str);
        row = ui_show_text(row, col, str);

        if ((row + 1) > ui_getMaxRows()) {
            break;
        }
    }
    gr_flip();
FUN_EXIT;
}

//------------------------------------------------------------------------------
void wifi_scan_results_parse() {
    int tbline = 0;
    int i = 0, j = 0;
    char delims2[] = "\t";
    char *result2 = NULL;

    sAPNum = WIFI_MAX_AP > line - 1 ? line - 1 : WIFI_MAX_AP;
    LOGD("sAPNum = %d, line =%d ", sAPNum, line - 1);
    for ( i = 0; i < sAPNum; i++) {
        result2 = strtok(wifiinfo[i], delims2);
        while (result2 != NULL ) {
            tbline++;
            if(tbline == 1) {
                strcpy(sAPs[i].smac, result2);
            } else if(tbline == 2) {
                sAPs[i].frequency = atoi(result2);
            } else if(tbline == 3) {
                sAPs[i].sig_level = atoi(result2);
            } else if(tbline == 5) {
                strcpy(sAPs[i].name, result2);
            }
            result2 = strtok( NULL, delims2);
        }
        tbline = 0;
    }
}

//------------------------------------------------------------------------------
int wifi_ap_type_check() {
    int ret = 0;
    int i = 0, j = 0;
    int wifi5g_num =0;
    int wifi2g_num = 0;
    int num = sAPNum;

    LOGD("enter");
    if (wifi5G_flag ==1) {
        for(int j=0; j<num; j++) {
            if(sAPs[j].frequency >4000) {
                wifi5g_num++;
            } else {
                wifi2g_num++;
            }
        }
        LOGD("wifi5g_num=%d, wifi2g_num=%d",wifi5g_num, wifi2g_num);
        if((wifi5g_num <1) ||(wifi2g_num<1)) {
            ret = -1;
            LOGD("wifi5G test failed\n");
        }
    }
    return ret;
}

//------------------------------------------------------------------------------
int eng_wifi_scan_start(void) {
FUN_ENTER;
    if (wifiOpen() < 0) {
        return -1;
    }

    if ( wifi_ScanAP() < 0) {
        return -1;
    }

    LOGD("eng_wifi_scan_start exit");
FUN_EXIT;
    return 0;
}

//------------------------------------------------------------------------------
int test_wifi_start_common(void)
{
FUN_ENTER;
    LOGD("enter");
    int ret = 0;
    int midrow = ui_getMaxRows() / 2;
    int midcol = gr_fb_width() / ui_getCharSize() / 2;

    ui_fill_locked();
    ui_show_title(MENU_TEST_WIFI);
    ui_set_color(CL_WHITE);
    ui_show_text(2, 0, TEXT_BT_SCANING);
    gr_flip();

    if (eng_wifi_scan_start() < 0) {
        ret = RL_FAIL;
        goto out;
    }

    wifi_scan_results_parse();

    if (wifi_ap_type_check() < 0) {
        ret = RL_FAIL;
        goto out;
    }

    if (sAPNum > 0) {
        wifi_show_result();

        ui_set_color(CL_GREEN);
        set_render_mode(Render_BOLD);
        ui_show_text(midrow, midcol - strlen(TEXT_TEST_PASS) / 2, TEXT_TEST_PASS);
        set_render_mode(Render_DEFAULT);
        gr_flip();
        ret = RL_PASS;
    } else {
        ret = RL_FAIL;
    }

    usleep(100);
out:
    if (RL_FAIL == ret) {
        ui_set_color(CL_RED);
        set_render_mode(Render_BOLD);
        ui_show_text(midrow, midcol - strlen(TEXT_TEST_FAIL) / 2, TEXT_TEST_FAIL);
        set_render_mode(Render_DEFAULT);
        gr_flip();
    }

    flush_wcn_log();

    wifiClose();
    save_result(CASE_TEST_WIFI, ret);
FUN_EXIT;
    return ret;
}

//------------------------------------------------------------------------------
int test_wifi_pretest_common(void) {
    int ret = 0;

    if (sAPNum > 0) {
        ret= RL_PASS;
    } else {
        ret= RL_FAIL;
    }

    save_result(CASE_TEST_WIFI, ret);
    return ret;
}
