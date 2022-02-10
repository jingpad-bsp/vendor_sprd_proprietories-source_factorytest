#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <cutils/android_reboot.h>
#include <cutils/properties.h>
#include <cutils/sockets.h>
#include <wait.h>

#include "minui.h"
#include "common.h"
#include "testitem.h"
#include "factorytest.h"
#include "menu_testitem.h"
#include "dev_header.h"
#include "chnmgr.h"
#include "parse_config.h"
#include "events.h"
#include "modem.h"
#include <bootloader_message/bootloader_message.h>

/**************Factorytest log path************/
#define FACTORYTEST_LOG_COMMON_DIR "/data/ylog/factorytest_log"
#define FACTORYTEST_LOG_ENCRYPTED_DIR "/cache/factorytest_log"


mmi_result phone_result[TOTAL_NUM];
mmi_result pcba_result[TOTAL_NUM];
hardware_result support_result[TOTAL_NUM];

menu_info menu_phone_result_menu[64];
menu_info menu_pcba_result_menu[64];
menu_info menu_not_suggestion_result_menu[64];

static int auto_all_test(void);
static int PCBA_auto_all_test(void);
static int show_phone_test_menu(void);
static int show_pcba_test_menu(void);
static int show_suggestion_test_menu(void);
static int show_phone_info_menu(void);

static int show_phone_test_result(void);
static int show_pcba_test_result(void);
static int show_suggestion_test_result(void);
static int phone_shutdown(void);
static int phone_shutdown_new_API(void);
static int phone_reboot(void);

static pthread_mutex_t result_mutex = PTHREAD_MUTEX_INITIALIZER;

char pcba_phone=0;
char autotest_flag = 0;

#define MULTI_TEST_CNT  5

#define SOCKET_NAME_MODEM_CTL "control_modem"
#define CMDLINE_SIZE    (0x1000)
menu_list *current_menu = NULL;

menu_info menu_root[] = {
#define MENUTITLE(num,title, func) \
    [ K_ROOT_##title ] = {num,MENU_##title, func, },
#include "menu_root.h"
#undef MENUTITLE
     [K_MENU_ROOT_CNT]={0,MENU_FACTORY_RESET, phone_shutdown_new_API,},
};

menu_info menu_auto_test[] = {
#define MENUTITLE(num,title, func) \
    [ A_PHONE_##title ] = {num,MENU_##title, func, },
#include "menu_auto_test.h"
#undef MENUTITLE
    //[K_MENU_AUTO_TEST_CNT] = {0,0,MENU_BACK, 0,},
};

menu_info menu_phone_test[] = {
#define MENUTITLE(num,title, func) \
    [ K_PHONE_##title ] = {num,MENU_##title, func, },
#include "menu_phone_test.h"
#undef MENUTITLE
    //[K_MENU_PHONE_TEST_CNT] = {0,0,MENU_BACK, 0,},
};

menu_info menu_not_auto_test[] = {
#define MENUTITLE(num,title, func) \
    [ D_PHONE_##title ] = {num,MENU_##title, func, },
#include "menu_not_autotest.h"
#undef MENUTITLE
    //[K_MENU_AUTO_TEST_CNT] = {0,0,MENU_BACK, 0,},
};

menu_info menu_phone_info_testmenu[] = {
#define MENUTITLE(num,title, func) \
    [ B_PHONE_##title ] = {num,MENU_##title, func, },
#include "menu_phone_info.h"
#undef MENUTITLE
    //[K_MENU_INFO_CNT] = {0,0,MENU_BACK, 0,},
};
menu_info phone_test_menu_supported[TOTAL_NUM] = {{0,NULL,NULL}};
menu_info pcba_test_menu_supported[TOTAL_NUM] = {{0,NULL,NULL}};
menu_info suggest_test_menu_supported[TOTAL_NUM] = {{0,NULL,NULL}};

static mmi_show_data mmi_data_table[] = {
    {CASE_TEST_LCD,            MENU_TEST_LCD,                test_lcd_start,            NULL,    ""},
    {CASE_TEST_TP,            MENU_TEST_TP,                test_tp_start,            NULL,    ""},
    {CASE_TEST_MULTITOUCH,    MENU_TEST_MULTI_TOUCH,        test_multi_touch_start,    NULL,    ""},
    {CASE_TEST_VIBRATOR,        MENU_TEST_VIBRATOR,                 test_vb_bl_start,            NULL,    ""},
    {CASE_TEST_BACKLIGHT,        MENU_TEST_BACKLIGHT,        test_vb_bl_start,            NULL,    ""},
    {CASE_TEST_FLASHLIGHT,    MENU_TEST_FLASHLIGHT,            test_flashlight_start,        NULL,    ""},
    {CASE_TEST_LED,            MENU_TEST_LED,                test_led_start,            NULL,    ""},
    {CASE_TEST_KEY,            MENU_TEST_KEY,                test_key_start,            NULL,    ""},
    {CASE_TEST_FCAMERA,        MENU_TEST_FCAMERA,            test_fcamera_start,        NULL,    ""},
    {CASE_TEST_FACAMERA,        MENU_TEST_FACAMERA,        test_facamera_start,        NULL,    ""},
    {CASE_TEST_BCAMERA,        MENU_TEST_BCAMERA,            test_bcamera_start,        NULL,    ""},
    {CASE_TEST_ACAMERA,        MENU_TEST_ACAMERA,            test_acamera_start,        NULL,    ""},
    {CASE_TEST_THIRD_CAMERA,        MENU_TEST_THIRD_CAMERA,            test_third_camera_start,        NULL,    ""},
    {CASE_TEST_FLASH,            MENU_TEST_FLASH,             test_bcamera_start,        NULL,    ""},
    {CASE_TEST_MAINLOOP,        MENU_TEST_MAINLOOP,        test_mainloopback_start,    NULL,    ""},
    {CASE_TEST_ASSISLOOP,        MENU_TEST_ASSISLOOP,        test_assisloopback_start,    NULL,    ""},
    {CASE_TEST_RECEIVER,        MENU_TEST_RECEIVER,            test_receiver_start,        NULL,    ""},
    {CASE_TEST_CHARGE,        MENU_TEST_CHARGE,            test_charge_start,        NULL,    ""},
    {CASE_TEST_SDCARD,        MENU_TEST_SDCARD,            test_sdcard_start,        NULL,    ""},
    {CASE_TEST_EMMC,            MENU_TEST_EMMC,            test_emmc_start,            NULL,    ""},
    {CASE_TEST_SIMCARD,        MENU_TEST_SIMCARD,            test_sim_start,            NULL,    ""},
    {CASE_TEST_RTC,            MENU_TEST_RTC,                test_rtc_start,            NULL,    ""},
    {CASE_TEST_HEADSET,        MENU_TEST_HEADSET,            test_headset_start,        NULL,    ""},
    {CASE_TEST_FM,            MENU_TEST_FM,                test_fm_start,            NULL,    ""},
    {CASE_TEST_LPSOR,            MENU_TEST_LPSENSOR,            test_lsensor_start,        NULL,    ""},
    {CASE_TEST_LSENSOR,            MENU_TEST_LSENSOR,            test_lsensor_start,        NULL,    ""},
    {CASE_TEST_PXYPSOR,            MENU_TEST_PXYSENSOR,            test_pxysensor_start,        NULL,    ""},
    {CASE_TEST_GYRSOR,        MENU_TEST_GSENSOR,            test_gsensor_start,        NULL,    ""},
    {CASE_TEST_ACCSOR,        MENU_TEST_ASENSOR,             test_asensor_start,        NULL,    ""},
    {CASE_TEST_MAGSOR,        MENU_TEST_MSENSOR,            test_msensor_start,        NULL,    ""},
    {CASE_TEST_PRESSOR,        MENU_TEST_PSENSOR,             test_psensor_start,        NULL,    ""},
    {CASE_TEST_BT,            MENU_TEST_BT,                test_bt_start,            NULL,    ""},
    {CASE_TEST_WIFI,            MENU_TEST_WIFI,                test_wifi_start,            NULL,    ""},
    {CASE_TEST_GPS,            MENU_TEST_GPS,                test_gps_start,            NULL,    ""},
    {CASE_TEST_SOUNDTRIGGER,    MENU_TEST_SOUNDTRIGGER,    test_soundtrigger_start,    NULL,    ""},
    {CASE_TEST_FINGERSOR,        MENU_TEST_FINGERSOR,        test_fingersor_start,        NULL,    ""},
    {CASE_TEST_TEL,            MENU_TEST_TEL,                test_tel_start,            NULL,    ""},
    {CASE_TEST_OTG,            MENU_TEST_OTG,                test_otg_start,            NULL,    ""},
    {CASE_TEST_NFC,             MENU_TEST_NFC,                test_nfc_start,             NULL,    ""},
    {CASE_CALI_ACCSOR,        MENU_CALI_ACCSOR,            cali_asensor_start,        NULL,    ""},
    {CASE_CALI_GYRSOR,        MENU_CALI_CYRSOR,            cali_gsensor_start,        NULL,    ""},
    {CASE_CALI_MAGSOR,        MENU_CALI_MAGSOR,            cali_msensor_start,        NULL,    ""},
    {CASE_CALI_LSOR,        MENU_CALI_LSOR,            cali_lsensor_start,        NULL,    ""},
    {CASE_CALI_PROSOR,        MENU_CALI_PROSOR,            cali_prosensor_start,        NULL,    ""},
    {CASE_CALI_PROSOR,        MENU_CALI_AUTOPROSOR,        cali_auto_prosensor_start,    NULL,    ""}
};

menu_info multi_test_item[MULTI_TEST_CNT] = {
    {CASE_TEST_SDCARD,MENU_TEST_SDCARD, &test_sdcard_pretest},
    {CASE_TEST_SIMCARD,MENU_TEST_SIMCARD, test_sim_pretest},
    {CASE_TEST_WIFI,MENU_TEST_WIFI, test_wifi_pretest},
    {CASE_TEST_BT,MENU_TEST_BT, test_bt_pretest},
    {CASE_TEST_GPS,MENU_TEST_GPS, test_gps_pretest}
};

hardware_result* getHardwareRes(){
    return support_result;
}

mmi_result* get_result_ptr(unsigned char id)
{
    mmi_result*ptr = NULL;

    if(id < TOTAL_NUM)
    {
        if(1 == pcba_phone)
            ptr = &pcba_result[id];
        else
            ptr = &phone_result[id];
    }
    return ptr;
}

unsigned char get_result(unsigned char id)
{
    mmi_result*ptr = NULL;

    if(id < TOTAL_NUM)
    {
        if(1 == pcba_phone)
            ptr = &pcba_result[id];
        else
            ptr = &phone_result[id];

        return ptr->pass_faild;
    }else {
      return  0;
    }
}

int isSuggestionCase(unsigned char id)
{
    int i = 0, menu_cnt = 0;

    for(i = 0, menu_cnt = 0; i < K_MENU_NOT_AUTO_TEST_CNT; i++)
    {
        if(menu_not_auto_test[i].num == id){
            return true;
        }
    }

    return false;
}

void save_result(unsigned char id,char key_result)
{
    mmi_result*ptr ;

    if( (RESULT_PASS == key_result) || (RESULT_FAIL == key_result))
    {
        ptr = get_result_ptr(id);
        ptr->pass_faild = key_result;

        if(isSuggestionCase(id))
        {
            int pcba_phone_temp = pcba_phone;
            pcba_phone = 1;
            ptr = get_result_ptr(id);
            ptr->pass_faild = key_result;
            pcba_phone = pcba_phone_temp;
        }
    }
}

int test_case_support(unsigned char id)
{
    hardware_result* ptr ;
    ptr = &support_result[id];

    return ptr->support;
}

static void read_bin(void)
{
    int fd_pcba = -1, fd_whole = -1;
    int len;

    fd_whole = open(PHONETXTPATH, O_RDWR);
    if (fd_whole < 0)
    {
        LOGE("mmitest open %s failed", PHONETXTPATH);
        goto out;
    }

    pthread_mutex_lock(&result_mutex);
    len = read(fd_whole, phone_result, sizeof(phone_result));
    if(len < 0)
    {
        LOGE("read %s failed,len = %d", PHONETXTPATH, len);
        goto out;
    }

    fd_pcba = open(PCBATXTPATH, O_RDWR);
    if (fd_pcba < 0 )
    {
        LOGE("mmitest open %s failed.", PHONETXTPATH);
        goto out;
    }
    len = read(fd_pcba, pcba_result, sizeof(pcba_result));
    if(len < 0)
    {
        LOGE("read %s failed, len = %d", PCBATXTPATH, len);
        goto out;
    }
    pthread_mutex_unlock(&result_mutex);

out:
    if(fd_whole >=  0)
        close(fd_whole);
    if(fd_pcba >= 0)
        close(fd_pcba);
}

static void write_bin(char * pathname)
{
    int i = 0, nums = 0, nump = 0;
    int len;
    int fd;

    fd = open(pathname, O_RDWR);
    if (fd < 0 )
    {
        LOGE("mmitest open %s failed.", pathname);
        goto out;
    }

    pthread_mutex_lock(&result_mutex);
    if(1 == pcba_phone)
    {
        for(i = 0; i < TOTAL_NUM - 1; i++)
        {
            if((1 == pcba_result[i].eng_support) && !isSuggestionCase(pcba_result[i].function_id))
            {
                nums++;
                if( RESULT_FAIL == pcba_result[i].pass_faild)
                {
                    pcba_result[TOTAL_NUM - 1].pass_faild = RESULT_FAIL;
                    break;
                }
                else if(RESULT_PASS == pcba_result[i].pass_faild)
                {
                    nump++;
                }
            }
        }
        if(nump == nums)
            pcba_result[TOTAL_NUM - 1].pass_faild = RESULT_PASS;
        len = write(fd, pcba_result, sizeof(pcba_result));
        if(len < 0)
        {
            LOGE("write %s failed,len = %d", pathname, len);
            goto out;
        }
    }
    else
    {
        for(i = 0; i < TOTAL_NUM - 1; i++)
        {
            if((1 == phone_result[i].eng_support) && !isSuggestionCase(phone_result[i].function_id))
            {
                nums++;
                if( RESULT_FAIL == phone_result[i].pass_faild)
                {
                    phone_result[TOTAL_NUM - 1].pass_faild = RESULT_FAIL;
                    break;
                }
                else if(RESULT_PASS == phone_result[i].pass_faild)
                {
                    nump++;
                }
            }
        }
        if(nump == nums)
            phone_result[TOTAL_NUM - 1].pass_faild = RESULT_PASS;
        len = write(fd, phone_result, sizeof(phone_result));
        if(len < 0)
        {
            LOGE("write %s failed,len = %d", pathname, len);
            goto out;
        }
    }
    sync();
    pthread_mutex_unlock(&result_mutex);

    LOGD("%s pcba_phone = %d,len = %d", pathname, pcba_phone, len);

out:
    if(fd >= 0 )
        close(fd);
    return ;
}

int set_menu(menu_list *menu, unsigned char menu_id)
{
    int cnt = 0;

    LOGD("set menu function");

    switch(menu_id)
    {
    case ROOT_MENU:
        menu->menu = menu_root;
        menu->item_total = K_MENU_ROOT_CNT + 1;
        menu->page_current = 1;
        menu->page_total = (ui_getMaxItems() - 1 + menu->item_total) / ui_getMaxItems();
        menu->item_select = menu->father_item_select;
        menu->title = MENU_TITLE_ROOT;
        menu->father = menu->self;
        menu->self = ROOT_MENU;
        menu->item_top_on_page = 0;
        menu->item_end_on_page = ui_getMaxItems();
        if(menu->item_end_on_page > menu->item_total)
            menu->item_end_on_page = menu->item_total;

        LOGD("ROOT_MENU: item_total = %d, page_total = %d", menu->item_total, menu->page_total);
        break;
    case PCBA_TEST_MENU:
        menu->menu = pcba_test_menu_supported;
        while(pcba_test_menu_supported[cnt].func != NULL)
            cnt++;
        menu->item_total = cnt;
        menu->page_current = 1;
        menu->page_total = (ui_getMaxItems() - 1 + menu->item_total) / ui_getMaxItems();
        menu->item_select = 0;
        menu->title = MENU_BOARD_SINGLETEST;
        menu->father = menu->self;
        menu->self = PCBA_TEST_MENU;
        menu->item_top_on_page = 0;
        menu->item_end_on_page = ui_getMaxItems();
        if(menu->item_end_on_page > menu->item_total)
            menu->item_end_on_page = menu->item_total;

        LOGD("PCBA_TEST_MENU: item_total = %d, page_total = %d", menu->item_total, menu->page_total);
        break;
    case PHONE_TEST_MENU:
        menu->menu = phone_test_menu_supported;
        while(phone_test_menu_supported[cnt].func != NULL)
            cnt++;
        menu->item_total = cnt;
        menu->page_current = 1;
        menu->page_total = (ui_getMaxItems() - 1 + menu->item_total) / ui_getMaxItems();
        menu->item_select = 0;
        menu->title = MENU_PHONE_SINGLETEST;
        menu->father = menu->self;
        menu->self = PHONE_TEST_MENU;
        menu->item_top_on_page = 0;
        menu->item_end_on_page = ui_getMaxItems();
        if(menu->item_end_on_page > menu->item_total)
            menu->item_end_on_page = menu->item_total;

        LOGD("PHONE_TEST_MENU: item_total = %d, page_total = %d", menu->item_total, menu->page_total);
        break;
    case SUGGEST_TEST_MENU:
        menu->menu = suggest_test_menu_supported;
        while(suggest_test_menu_supported[cnt].func != NULL)
            cnt++;
        menu->item_total = cnt;
        menu->page_current = 1;
        menu->page_total = (ui_getMaxItems() - 1 + menu->item_total) / ui_getMaxItems();
        menu->item_select = 0;
        menu->title = MENU_NOT_AUTO_TEST;
        menu->father = menu->self;
        menu->self = SUGGEST_TEST_MENU;
        menu->item_top_on_page = 0;
        menu->item_end_on_page = ui_getMaxItems();
        if(menu->item_end_on_page > menu->item_total)
            menu->item_end_on_page = menu->item_total;

        LOGD("SUGGEST_TEST_MENU: item_total = %d, page_total = %d", menu->item_total, menu->page_total);
        break;
    case PHONE_VERSION_MENU:
        menu->menu = menu_phone_info_testmenu;
        menu->item_total = K_MENU_INFO_CNT;
        menu->page_current = 1;
        menu->page_total = (ui_getMaxItems() - 1 + menu->item_total) / ui_getMaxItems();
        menu->item_select = 0;
        menu->title = MENU_PHONE_INFO;
        menu->father = menu->self;
        menu->self = PHONE_VERSION_MENU;
        menu->item_top_on_page = 0;
        menu->item_end_on_page = ui_getMaxItems();
        if(menu->item_end_on_page > menu->item_total)
            menu->item_end_on_page = menu->item_total;

        LOGD("PHONE_VERSION_MENU: item_total = %d, page_total = %d", menu->item_total, menu->page_total);
        break;
    case PCBA_RESULT_MENU:
        menu->menu = menu_pcba_result_menu;
        while(menu_pcba_result_menu[cnt].func != NULL)
            cnt++;
        menu->item_total = cnt;
        menu->page_current = 1;
        menu->page_total = (ui_getMaxItems() - 1 + menu->item_total) / ui_getMaxItems();
        menu->item_select = 0;
        menu->title = MENU_BOARD_REPORT;
        menu->father = menu->self;
        menu->self = PCBA_RESULT_MENU;
        menu->item_top_on_page = 0;
        menu->item_end_on_page = ui_getMaxItems();
        if(menu->item_end_on_page > menu->item_total)
            menu->item_end_on_page = menu->item_total;

        LOGD("PCBA_RESULT_MENU: item_total = %d, page_total = %d", menu->item_total, menu->page_total);
        break;
    case PHONE_RESULT_MENU:
        menu->menu = menu_phone_result_menu;
        while(menu_phone_result_menu[cnt].func != NULL)
            cnt++;
        menu->item_total = cnt;
        menu->page_current = 1;
        menu->page_total = (ui_getMaxItems() - 1 + menu->item_total) / ui_getMaxItems();
        menu->item_select = 0;
        menu->title = MENU_PHONE_REPORT;
        menu->father = menu->self;
        menu->self = PHONE_RESULT_MENU;
        menu->item_top_on_page = 0;
        menu->item_end_on_page = ui_getMaxItems();
        if(menu->item_end_on_page > menu->item_total)
            menu->item_end_on_page = menu->item_total;

        LOGD("PHONE_RESULT_MENU: item_total = %d, page_total = %d", menu->item_total, menu->page_total);
        break;
    case SUGGEST_RESULT_MENU:
        menu->menu = menu_not_suggestion_result_menu;
        while(menu_not_suggestion_result_menu[cnt].func != NULL)
            cnt++;
        menu->item_total = cnt;
        menu->page_current = 1;
        menu->page_total = (ui_getMaxItems() - 1 + menu->item_total) / ui_getMaxItems();
        menu->item_select = 0;
        menu->title = MENU_NOT_AUTO_REPORT;
        menu->father = menu->self;
        menu->self = SUGGEST_RESULT_MENU;
        menu->item_top_on_page = 0;
        menu->item_end_on_page = ui_getMaxItems();
        if(menu->item_end_on_page > menu->item_total)
            menu->item_end_on_page = menu->item_total;

        LOGD("SUGGEST_RESULT_MENU: item_total = %d, page_total = %d", menu->item_total, menu->page_total);
        break;
    default:
        LOGD("unknow MENU");
        break;
    }

    return 0;
}

void config_supported_item()
{
    int menu_cnt = 0;
    int i = 0;

    LOGD("config supported test items");

    for(i = 0, menu_cnt = 0; i < K_MENU_PHONE_TEST_CNT; i++)
    {
        if(test_case_support(menu_phone_test[i].num))
        {
            phone_test_menu_supported[menu_cnt].title= menu_phone_test[i].title;
            phone_test_menu_supported[menu_cnt].func= menu_phone_test[i].func;
            phone_test_menu_supported[menu_cnt].num= menu_phone_test[i].num;
            LOGD("phone test supported: [%d]:%s",menu_cnt, phone_test_menu_supported[menu_cnt].title);
            menu_cnt++;
        }
    }

    for(i = 1, menu_cnt = 0; i < K_MENU_PHONE_TEST_CNT; i++)
    {
        if(test_case_support(menu_phone_test[i].num))
        {
            pcba_test_menu_supported[menu_cnt].title= menu_phone_test[i].title;
            pcba_test_menu_supported[menu_cnt].func= menu_phone_test[i].func;
            pcba_test_menu_supported[menu_cnt].num= menu_phone_test[i].num;
            LOGD("PCBA test supported: [%d]:%s",menu_cnt, pcba_test_menu_supported[menu_cnt].title);
            menu_cnt++;
        }
    }

    for(i = 0, menu_cnt = 0; i < K_MENU_NOT_AUTO_TEST_CNT; i++)
    {
        if(test_case_support(menu_not_auto_test[i].num))
        {
            suggest_test_menu_supported[menu_cnt].title= menu_not_auto_test[i].title;
            suggest_test_menu_supported[menu_cnt].func= menu_not_auto_test[i].func;
            suggest_test_menu_supported[menu_cnt].num= menu_not_auto_test[i].num;
            LOGD("suggest test supported: [%d]:%s",menu_cnt, suggest_test_menu_supported[menu_cnt].title);
            menu_cnt++;
        }
    }
}

static int show_root_menu(void)
{
    int action;
    int key;
    int ret;

    config_supported_item();
    current_menu = (menu_list*)malloc(sizeof(menu_list));
    current_menu->self = 0;
    current_menu->father_item_select = 0;
    set_menu(current_menu, ROOT_MENU);
    LOGD("native mmi: show root menu");

    while(1)
    {
        draw_menu(current_menu);

        key = ui_wait_key();
        LOGD("the pressed key = %d", key);

        action = device_handle_key(key);
        ret = menu_change(current_menu, action);
        if(ret >= 0)
        {
            draw_menu(current_menu);
            current_menu->father_item_select = current_menu->item_select;
            current_menu->menu[ret].func();
            ui_clear_key_queue();
        }
    }

    return 0;
}

static void show_multi_test_result(void)
{
    int ret = RL_NA;
    int row = 3;
    int menu_cnt = 0;
    char tmp[128];
    char* rl_str;
    int i,j;
    menu_info pmenu[MULTI_TEST_CNT] = {{0,NULL,NULL}};
    menu_info ptest[MULTI_TEST_CNT] = {{0,NULL,NULL}};
    ui_fill_locked();
    ui_show_title(MENU_MULTI_TEST);
    gr_flip();

    for(i = 0; i < MULTI_TEST_CNT; i++) {
        if(test_case_support(multi_test_item[i].num)){
            ptest[menu_cnt]=multi_test_item[i];
            LOGD("mmitest pmenu[%d].title=%s",menu_cnt,pmenu[menu_cnt].title);
            menu_cnt++;
        }
    }

    for(i = 0; i < menu_cnt; i++) {
        //ret = ptest[i].func();
        //Just get result from tested info
        ret = get_result(multi_test_item[i].num);
	 LOGD("multi mmitest ret[%d].title=%s",ret,multi_test_item[i].title);
        if(ret == RL_PASS) {
            ui_set_color(CL_GREEN);
            rl_str = TEXT_PASS;
        } else {
            ui_set_color(CL_RED);
            rl_str = TEXT_FAIL;
        }
        memset(tmp, 0, sizeof(tmp));
        for(j=0;i<K_MENU_AUTO_TEST_CNT;j++){
            if(ptest[i].num==menu_auto_test[j].num)
                break;
        }
        snprintf(tmp, sizeof(tmp), "%s: %s", (menu_auto_test[j].title+1), rl_str);
        row = ui_show_text(row, 0, tmp);
        gr_flip();
    }

    sleep(1);
}

static void fail_retest(menu_info* auto_test)
{
    int i;
    menu_info* pmenu = auto_test;
    mmi_result*ptr;
    int result = RL_NA;
    autotest_flag = 1;

    for(i = pcba_phone; i < K_MENU_AUTO_TEST_CNT; i++){
        ptr = get_result_ptr(pmenu[i].num);
        if(RL_FAIL == ptr->pass_faild){
            result = pmenu[i].func();
        }
        if((RL_NEXT_PAGE == result)||(RL_BACK == result)){
            break;
        }
    }
    autotest_flag = 0;

}

typedef int (*ft_item_work)(void);

void* work_thread(void *arg)
{
    ft_item_work work = NULL;
    if (arg != NULL) {
        work = *((ft_item_work *)arg);

        pthread_detach(pthread_self());
        if(work != NULL) {
            work();
        }
    }

    return NULL;
}

void thread_run(ft_item_work it_thread){
    pthread_t t_thread;
    pthread_create(&t_thread, NULL, work_thread, (void *)(&it_thread));
}

void *test_back_groud(void *)
{
    //Do pretest work.
    LOGD("test_back_groud");
    int pre_menu_cnt = 0;
    int ret = -1;
    int i = 0;
    menu_info ptest[MULTI_TEST_CNT] = {{0,NULL,NULL}};
    for(i = 0; i < MULTI_TEST_CNT; i++) {
        if(test_case_support(multi_test_item[i].num)){
            ptest[pre_menu_cnt]=multi_test_item[i];
            LOGD("test_back_groud pretest pmenu[%d].title=%s",pre_menu_cnt,ptest[pre_menu_cnt].title);
            pre_menu_cnt++;
        }
    }

    for(i = 0; i < pre_menu_cnt; i++) {
        if (i == CASE_TEST_BT || i == CASE_TEST_GPS || i == CASE_TEST_WIFI){
            thread_run(ptest[i].func);
        }else{
            ret = ptest[i].func();
        }
    }
    //end
    return NULL;
}

static int test_backgroud_init(void)
{
    pthread_t backgroud_init_thread;
    pthread_create(&backgroud_init_thread, NULL, test_back_groud, NULL);
    return 0;
}

static int auto_all_test(void)
{
    int i = 0, j = 0;
    int menu_cnt = 0;
    int result = 0,time_consume = 0;
    int key;
    char* rl_str;
    time_t start_time,end_time;

    //test_gps_init();
    //test_bt_wifi_init(NULL);
    test_backgroud_init();
    menu_info pmenu[K_MENU_PHONE_TEST_CNT] = {{0,NULL,NULL}};
    pcba_phone=0;
    autotest_flag = 1;

    for(i = 0; i < K_MENU_AUTO_TEST_CNT; i++) {
        if(test_case_support(menu_auto_test[i].num)){
            pmenu[menu_cnt]=menu_auto_test[i];
            LOGD("mmitest pmenu[%d].title=%s",menu_cnt,pmenu[menu_cnt].title);
            menu_cnt++;
        }
    }

    for(i = 0; i < menu_cnt; i++){
        for(j = 0; j < MULTI_TEST_CNT; j++) {
            if(pmenu[i].num== multi_test_item[j].num) {
                LOGD("mmitest break, id=%d", i);
                break;
            }
        }
        if(j < MULTI_TEST_CNT) {
            continue;
        }
        LOGD("mmitest Do, id=%d", i);
        if(pmenu[i].func != NULL) {
            start_time = time(NULL);
            result = pmenu[i].func();

            //clean screen, after eatch auto test item. by shengyang.liu
            gr_color(0,0,0,255);
            gr_clear();

            end_time = time(NULL);
            time_consume = end_time -start_time;
            LOGD("********* %s Test Time: %ds *********", pmenu[i].title, time_consume);
            if((RL_NEXT_PAGE == result)||(RL_BACK == result)){
                autotest_flag = 0;
                goto end;
            }
        }

    }
    show_multi_test_result();
    autotest_flag = 0;
    //gpsClose();
    //wifiClose();
    key = ui_handle_button(NULL,TEXT_TEST_FAIL_CASE,TEXT_GOBACK);
    if(RL_NEXT_PAGE == key){
        LOGD("mmitest retest fail cases!");
        fail_retest(pmenu);
    }
end:
    write_bin(PHONETXTPATH);
    return 0;
}


static int PCBA_auto_all_test(void)
{
    int i = 0, j = 0;
    int menu_cnt = 0;
    int result = 0,time_consume = 0;
    int key;
    char* rl_str;
    time_t start_time,end_time;

    //test_gps_init();
    //test_bt_wifi_init(NULL);
    test_backgroud_init();
    menu_info pmenu[K_MENU_PHONE_TEST_CNT] = {{0,NULL,NULL}};
    pcba_phone=1;
    autotest_flag = 1;

    for(i = 1; i < K_MENU_AUTO_TEST_CNT; i++) {
        if(test_case_support(menu_auto_test[i].num)){
            pmenu[menu_cnt]=menu_auto_test[i];
            LOGD("mmitest pmenu[%d].title=%s",menu_cnt,pmenu[menu_cnt].title);
            menu_cnt++;
        }
    }

    for(i = 0; i < menu_cnt; i++)
    {
        for(j = 0; j < MULTI_TEST_CNT; j++)
        {
            if(pmenu[i].num == multi_test_item[j].num)
            {
                LOGD("mmitest break, id=%d", i);
                break;
            }
        }

        if(j < MULTI_TEST_CNT)
            continue;

        if(pmenu[i].func != NULL)
        {
            LOGD("mmitest Do id=%d", i);
            start_time = time(NULL);
            result = pmenu[i].func();

            //clean screen, after eatch auto test item. by shengyang.liu
            gr_color(0,0,0,255);
            gr_clear();

            end_time = time(NULL);
            time_consume = end_time -start_time;
            LOGD("********* %s Test Time: %ds *********", pmenu[i].title, time_consume);
            if((RL_NEXT_PAGE == result)||(RL_BACK == result))
            {
                autotest_flag = 0;
                goto end;
            }
        }

    }
    show_multi_test_result();
    autotest_flag = 0;
    //gpsClose();
    //wifiClose();
    key = ui_handle_button(NULL,TEXT_TEST_FAIL_CASE,TEXT_GOBACK);
    if(RL_NEXT_PAGE == key)
    {
        LOGD("mmitest retest fail cases!");
        fail_retest(pmenu);
    }
end:

    write_bin(PCBATXTPATH);
    return 0;
}

static int show_phone_test_menu(void)
{
    int action;
    int key;
    int ret;
    time_t start_time, end_time;
    int time_consume;

    pcba_phone = 0;
    set_menu(current_menu, PHONE_TEST_MENU);
    LOGD("show phone test menu");

    while(1)
    {
        draw_menu(current_menu);

        key = ui_wait_key();
        LOGD("the pressed key = %d", key);

        action = device_handle_key(key);
        ret = menu_change(current_menu, action);
        if(ret >= 0)
        {
            draw_menu(current_menu);
            if(current_menu->menu[ret].func != NULL)
            {
                LOGD("enter function:%s", current_menu->menu[ret].title);
                start_time = time(NULL);
                current_menu->menu[ret].func();
                end_time = time(NULL);
                time_consume = end_time - start_time;
                LOGD("********* %s Test Time: %ds *********", current_menu->menu[ret].title, time_consume);
            }
            ui_clear_key_queue();
        }
        else if(ret == -2)
        {
            LOGD("back to father Menu");
            set_menu(current_menu, current_menu->father);
            break;
        }
    }

    write_bin(PHONETXTPATH);
    return 0;
}

static int show_pcba_test_menu(void)
{
    int action;
    int key;
    int ret;
    time_t start_time, end_time;
    int time_consume;

    pcba_phone = 1;
    set_menu(current_menu, PCBA_TEST_MENU);
    LOGD("show phone test menu");

    while(1)
    {
        draw_menu(current_menu);

        key = ui_wait_key();
        LOGD("the pressed key = %d", key);

        action = device_handle_key(key);
        ret = menu_change(current_menu, action);
        if(ret >= 0)
        {
            draw_menu(current_menu);
            if(current_menu->menu[ret].func != NULL)
            {
                LOGD("enter function:%s", current_menu->menu[ret].title);
                start_time = time(NULL);
                current_menu->menu[ret].func();
                end_time = time(NULL);
                time_consume = end_time - start_time;
                LOGD("********* %s Test Time: %ds *********", current_menu->menu[ret].title, time_consume);
            }
            ui_clear_key_queue();
        }
        else if(ret == -2)
        {
            LOGD("back to father Menu");
            set_menu(current_menu, current_menu->father);
            break;
        }
    }

    write_bin(PCBATXTPATH);
    return 0;
}

void updata_test_result(int test, bool bWrite)
{
    int i = 0;
    int flag = 0;
    unsigned char result;
    int menu_cnt = 0;
    int num = sizeof(mmi_data_table)/sizeof(mmi_data_table[0])-K_MENU_NOT_AUTO_TEST_CNT;

    LOGD("updata test result");
    switch(test)
    {
    case PHONE_RESULT_MENU:
        for(i = 0; i < num; i++)
        {
            if(test_case_support(mmi_data_table[i].id))
            {
                result = get_result(mmi_data_table[i].id);
                memset(mmi_data_table[i].title, 0, sizeof(mmi_data_table[i].title));
                switch(result)
                {
                case RL_NA:
                    snprintf(mmi_data_table[i].title,sizeof(mmi_data_table[i].title),"-%s:%s",(mmi_data_table[i].name+1),TEXT_NA);
                    break;
                case RL_FAIL:
                    snprintf(mmi_data_table[i].title,sizeof(mmi_data_table[i].title),"-%s:%s",(mmi_data_table[i].name+1),TEXT_FAIL);
                    break;
                case RL_PASS:
                    snprintf(mmi_data_table[i].title,sizeof(mmi_data_table[i].title),"-%s:%s",(mmi_data_table[i].name+1),TEXT_PASS);
                    break;
                case RL_NS:
                    snprintf(mmi_data_table[i].title,sizeof(mmi_data_table[i].title),"-%s:%s",(mmi_data_table[i].name+1),TEXT_NS);
                    break;
                default:
                    snprintf(mmi_data_table[i].title,sizeof(mmi_data_table[i].title),"-%s:%s",(mmi_data_table[i].name+1),TEXT_NA);
                    break;
                }
                menu_phone_result_menu[menu_cnt].title = mmi_data_table[i].title;
                menu_phone_result_menu[menu_cnt].func = mmi_data_table[i].func;

                LOGD("PHONE_RESULT_MENU: %s", menu_phone_result_menu[menu_cnt].title);
                menu_cnt++;
            }
        }
        break;
    case PCBA_RESULT_MENU:
        for(i = 1; i < num; i++)
        {
            if(test_case_support(mmi_data_table[i].id))
            {
                result = get_result(mmi_data_table[i].id);
                memset(mmi_data_table[i].title, 0, sizeof(mmi_data_table[i].title));
                switch(result)
                {
                case RL_NA:
                    snprintf(mmi_data_table[i].title,sizeof(mmi_data_table[i].title),"-%s:%s",(mmi_data_table[i].name+1),TEXT_NA);
                    break;
                case RL_FAIL:
                    snprintf(mmi_data_table[i].title,sizeof(mmi_data_table[i].title),"-%s:%s",(mmi_data_table[i].name+1),TEXT_FAIL);
                    break;
                case RL_PASS:
                    snprintf(mmi_data_table[i].title,sizeof(mmi_data_table[i].title),"-%s:%s",(mmi_data_table[i].name+1),TEXT_PASS);
                    break;
                case RL_NS:
                    snprintf(mmi_data_table[i].title,sizeof(mmi_data_table[i].title),"-%s:%s",(mmi_data_table[i].name+1),TEXT_NS);
                    break;
                default:
                    snprintf(mmi_data_table[i].title,sizeof(mmi_data_table[i].title),"-%s:%s",(mmi_data_table[i].name+1),TEXT_NA);
                    break;
                }
                menu_pcba_result_menu[menu_cnt].title = mmi_data_table[i].title;
                menu_pcba_result_menu[menu_cnt].func = mmi_data_table[i].func;

                LOGD("PCBA_RESULT_MENU: %s", menu_pcba_result_menu[menu_cnt].title);
                menu_cnt++;
            }
        }
        break;
    case SUGGEST_RESULT_MENU:
        for(i = num; i < num + K_MENU_NOT_AUTO_TEST_CNT; i++)
        {
            if(test_case_support(mmi_data_table[i].id))
            {
                result = get_result(mmi_data_table[i].id);
                memset(mmi_data_table[i].title, 0, sizeof(mmi_data_table[i].title));
                switch(result)
                {
                case RL_NA:
                    snprintf(mmi_data_table[i].title,sizeof(mmi_data_table[i].title),"-%s:%s",(mmi_data_table[i].name+1),TEXT_NA);
                    break;
                case RL_FAIL:
                    snprintf(mmi_data_table[i].title,sizeof(mmi_data_table[i].title),"-%s:%s",(mmi_data_table[i].name+1),TEXT_FAIL);
                    break;
                case RL_PASS:
                    snprintf(mmi_data_table[i].title,sizeof(mmi_data_table[i].title),"-%s:%s",(mmi_data_table[i].name+1),TEXT_PASS);
                    break;
                case RL_NS:
                    snprintf(mmi_data_table[i].title,sizeof(mmi_data_table[i].title),"-%s:%s",(mmi_data_table[i].name+1),TEXT_NS);
                    break;
                default:
                    snprintf(mmi_data_table[i].title,sizeof(mmi_data_table[i].title),"-%s:%s",(mmi_data_table[i].name+1),TEXT_NA);
                    break;
                }
                menu_not_suggestion_result_menu[menu_cnt].title = mmi_data_table[i].title;
                menu_not_suggestion_result_menu[menu_cnt].func = mmi_data_table[i].func;

                LOGD("SUGGEST_RESULT_MENU: %s", menu_pcba_result_menu[menu_cnt].title);
                menu_cnt++;
            }
        }
        break;
    }

    if (bWrite){
        flag = pcba_phone;
        pcba_phone = 0;
        write_bin(PHONETXTPATH);
        pcba_phone = 1;
        write_bin(PCBATXTPATH);
        pcba_phone = flag;
    }

}

static int show_suggestion_test_menu(void)
{
    int action;
    int key;
    int ret;
    int flag;
    time_t start_time, end_time;
    int time_consume;

    pcba_phone=0;
    set_menu(current_menu, SUGGEST_TEST_MENU);
    LOGD("show phone test menu");

    while(1)
    {
        draw_menu(current_menu);

        key = ui_wait_key();
        LOGD("the pressed key = %d", key);

        action = device_handle_key(key);
        ret = menu_change(current_menu, action);
        draw_menu(current_menu);
        if(current_menu->menu[ret].func != NULL)
        {
            LOGD("enter function:%s", current_menu->menu[ret].title);
            start_time = time(NULL);
            current_menu->menu[ret].func();
            end_time = time(NULL);
            time_consume = end_time -start_time;
            LOGD("********* %s Test Time: %ds *********",current_menu->menu[ret].title, time_consume);
        }
        else if(ret == -2)
        {
            LOGD("back to father Menu");
            set_menu(current_menu, current_menu->father);
            break;
        }
    }

    flag = pcba_phone;
    pcba_phone = 0;
    write_bin(PHONETXTPATH);
    pcba_phone = 1;
    write_bin(PCBATXTPATH);
    pcba_phone = flag;

    return 0;
}

static int show_phone_info_menu(void)
{
    int action;
    int key;
    int ret;

    set_menu(current_menu, PHONE_VERSION_MENU);
    LOGD("show phone test menu");

    while(1)
    {
        draw_menu(current_menu);

        key = ui_wait_key();
        LOGD("the pressed key = %d", key);

        action = device_handle_key(key);

        ret = menu_change(current_menu, action);
        if(ret >= 0)
        {
            draw_menu(current_menu);
            LOGD("enter function:%s", current_menu->menu[ret].title);
            current_menu->menu[ret].func();
            ui_clear_key_queue();
        }
        else if(ret == -2)
        {
            LOGD("back to father Menu");
            set_menu(current_menu, current_menu->father);
            return 0;
        }
    }

    return 0;
}

static int show_phone_test_result(void)
{
    int action;
    int key;
    int ret;

    LOGD("show phone test result");

    pcba_phone=0;
    read_bin();
    updata_test_result(PHONE_RESULT_MENU, false);
    set_menu(current_menu, PHONE_RESULT_MENU);

    while(1)
    {
        draw_menu(current_menu);

        key = ui_wait_key();
        LOGD("the pressed key = %d", key);

        action = device_handle_key(key);
        ret = menu_change(current_menu, action);
        if(ret >= 0)
        {
            draw_menu(current_menu);
            LOGD("enter function:%s", current_menu->menu[ret].title);
            current_menu->menu[ret].func();
            ui_clear_key_queue();
            updata_test_result(PHONE_RESULT_MENU, true);
        }
        else if(ret == -2)
        {
            LOGD("back to father Menu");
            set_menu(current_menu, current_menu->father);
            return 0;
        }
    }

    return 0;
}

static int show_pcba_test_result(void)
{
    int action;
    int key;
    int ret;

    LOGD("show PCBA test result");

    pcba_phone=1;
    read_bin();
    updata_test_result(PCBA_RESULT_MENU, false);
    set_menu(current_menu, PCBA_RESULT_MENU);

    while(1)
    {
        draw_menu(current_menu);

        key = ui_wait_key();
        LOGD("the pressed key = %d", key);

        action = device_handle_key(key);
        ret = menu_change(current_menu, action);
        if(ret >= 0)
        {
            draw_menu(current_menu);
            LOGD("enter function:%s", current_menu->menu[ret].title);
            current_menu->menu[ret].func();
            ui_clear_key_queue();
            updata_test_result(PCBA_RESULT_MENU, true);
        }
        else if(ret == -2)
        {
            LOGD("back to father Menu");
            set_menu(current_menu, current_menu->father);
            return 0;
        }
    }

    return 0;
}

static int show_suggestion_test_result(void)
{
    int action;
    int key;
    int ret;

    LOGD("show suggestion test result");
    pcba_phone=0;
    read_bin();
    updata_test_result(SUGGEST_RESULT_MENU, false);
    set_menu(current_menu, SUGGEST_RESULT_MENU);

    while(1)
    {
        draw_menu(current_menu);

        key = ui_wait_key();
        LOGD("the pressed key = %d", key);

        action = device_handle_key(key);
        ret = menu_change(current_menu, action);
        if(ret >= 0)
        {
            draw_menu(current_menu);
            LOGD("enter function:%s", current_menu->menu[ret].title);
            current_menu->menu[ret].func();
            ui_clear_key_queue();
            updata_test_result(SUGGEST_RESULT_MENU, true);
        }
        else if(ret == -2)
        {
            LOGD("back to father Menu");
            set_menu(current_menu, current_menu->father);
            return 0;
        }
    }

    return 0;
}

static int phone_reboot(void)
{
    LOGD("==== phone_reboot enter ====\n");
    sync();
    android_reboot(ANDROID_RB_RESTART2, 0, "normal");
    return 0;
}

#define REBOOT_NEW_API 1
static int reboot_into_recovery(const std::vector<std::string>& options) {
    LOGD("Rebooting into recovery");
    std::string err;
    if (!write_bootloader_message(options, &err)) {
	 LOGE("Rebooting into recovery fail!");
        return -1;
    }
    property_set("sys.powerctl", "reboot,recovery");
    return 0;
}
static int phone_shutdown_new_API(void)
{
#ifdef LANGUAGE_CN
    const char Cmd1[] = "--wipe_data\n--locale=zh_CN";
#else
    const char Cmd1[] = "--wipe_data\n--locale=en_US";
#endif
    const char Cmd2[] = "--reason=wipe_data_via_recovery\n";
   LOGD("phone_shutdown_new_API Cmd1=%s,Cmd2=%s",Cmd1,Cmd2);
   return reboot_into_recovery({Cmd1, Cmd2});
}

static int phone_shutdown(void)
{
    int fd;
    int ret = -1;
    int time_consume = 0;
    const char RecoveryDir[] = "/cache/recovery/";
    const char CmdPath[] = "/cache/recovery/command";
#ifdef LANGUAGE_CN
    const char Cmd1[] = "--wipe_data\n--locale=zh_CN";
#else
    const char Cmd1[] = "--wipe_data\n--locale=en_US";
#endif
    const char Cmd2[] = "--reason=wipe_data_via_recovery\n";
    time_t start_time, end_time;


    start_time = time(NULL);
    ret = mkdir(RecoveryDir, S_IRWXU | S_IRWXG | S_IRWXO);
    if (-1 == ret && (errno != EEXIST))
    {
        LOGE("mkdir %s failed.",RecoveryDir);
    }

    fd = open(CmdPath, O_WRONLY | O_CREAT, 0777);
    if (fd >= 0)
    {
        write(fd, Cmd1, strlen(Cmd1) + 1);
        write(fd, Cmd2, strlen(Cmd2) + 1);
        sync();
        close(fd);
    }
    else
    {
        LOGE("open %s failed.",CmdPath);
        return -1;
    }

    end_time = time(NULL);
    time_consume = end_time - start_time;
    LOGD("mmitest select menu = %s consume time = %d", MENU_FACTORY_RESET, time_consume);
    usleep(200 * 1000);
    android_reboot(ANDROID_RB_RESTART2, 0, "recovery");

    return 0;
}

void *test_printlog_thread(void *)
{
    int ret = -1;
    int fd = -1;
      char *lpLogDir = NULL;
      char LogPath[128] = {0};
      char buff[128] = {0};
      char prop_encrypted[PROPERTY_VALUE_MAX+1] = {0};

    LOGD("test_printlog_thread start");
    pthread_detach(pthread_self());     //free by itself

       property_get("ro.crypto.state", prop_encrypted, "NONE");
       LOGD("in %s: ro.crypto.state=%s\n",__func__,prop_encrypted);

       if (strcmp(prop_encrypted, "encrypted") == 0) {
            lpLogDir = FACTORYTEST_LOG_ENCRYPTED_DIR;
        }else {
            lpLogDir = FACTORYTEST_LOG_COMMON_DIR;
        }

    if (0 != access(lpLogDir, F_OK) )
    {
        ret = mkdir(lpLogDir, S_IRWXU | S_IRWXG | S_IRWXO);
        if (-1 == ret && (errno != EEXIST))
        {
            LOGE("mkdir %s failed.", lpLogDir);
            return NULL;
        }
    }
    ret = chmod(lpLogDir, S_IRWXU | S_IRWXG | S_IRWXO);
    if (-1 == ret)
    {
        LOGE("chmod %s failed.", lpLogDir);
        return NULL;
    }

       memset(LogPath, 0, sizeof(LogPath));
       sprintf(LogPath, "%s/last_factorytest.log", lpLogDir);
    if (0 == access(LogPath, F_OK))
    {
        ret = remove(LogPath);
        if (-1 == ret)
        {
            LOGE("remove failed.");
            return NULL;
        }
    }

       memset(LogPath, 0, sizeof(LogPath));
       sprintf(LogPath, "%s/factorytest.log", lpLogDir);
    if (0 == access(LogPath, F_OK))
    {
            memset(buff, 0, sizeof(buff));
            sprintf(buff, "%s/last_factorytest.log", lpLogDir);
            ret =  rename(LogPath, buff);
            if (-1 == ret)
            {
                LOGE("rename failed.");
                return NULL;
            }
    }

       memset(LogPath, 0, sizeof(LogPath));
       sprintf(LogPath, "%s/factorytest.log", lpLogDir);
    fd = open(LogPath, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd == -1 && (errno != EEXIST))
    {
        LOGE("creat %s failed.", lpLogDir);
        return NULL;
    }
    if (fd >= 0 )
        close(fd);

    ret = chmod(LogPath, 0777);
    if (-1 == ret)
    {
        LOGE("chmod %s failed.", lpLogDir);
        return NULL;
    }

       memset(buff, 0, sizeof(buff));
       sprintf(buff, "logcat -v threadtime -f %s & ", LogPath);
       LOGD("logcat cmdline = %s", buff);
    ret = system(buff);
    if(!WIFEXITED(ret) || WEXITSTATUS(ret) || -1 ==  ret)
    {
        LOGE("system failed.");
        return NULL;
    }

    system("sync");
    return NULL;
}

static int test_result_mkdir(void)
{
    int i, ret, len = 0;
    int fd_pcba = -1, fd_whole = -1;
    mmi_result result[TOTAL_NUM];

    ret = chmod("/vendor/productinfo", 0777);
    if (-1 == ret)
    {
        LOGE("mmitest chmod /productinfo failed.");
        //goto out;
    }

    if (0 != access(PCBATXTPATH, F_OK))
    {
        fd_pcba = open(PCBATXTPATH, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        if (fd_pcba < 0)
        {
            LOGE("mmitest,create %s failed.", PCBATXTPATH);
            goto out;
        }

        //init /productinfo/PCBAtest.txt
        for(i = 0; i < TOTAL_NUM; i++)
        {
            result[i].type_id = 1;
            result[i].function_id = i;
            result[i].eng_support = support_result[i].support;
            result[i].pass_faild = RESULT_NOT_TEST;
        }
        result[0].eng_support = 0; //lcd not support
        len = write(fd_pcba, result, sizeof(result));
        if(len < 0)
        {
            LOGE("mmitest %s write_len = %d.", PCBATXTPATH, len);
            goto out;
        }
        fsync(fd_pcba);
    }

    if (0 != access(PHONETXTPATH, F_OK))
    {
        fd_whole = open(PHONETXTPATH, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        if (fd_whole < 0)
        {
            LOGE("mmitest create %s failed.", PHONETXTPATH);
            goto out;
        }
        //init /productinfo/wholetest.txt
        for(i = 0; i < TOTAL_NUM; i++)
        {
            result[i].type_id = 0;
            result[i].function_id = i;
            result[i].eng_support = support_result[i].support;
            result[i].pass_faild = RESULT_NOT_TEST;
        }
        len = write(fd_whole, result, sizeof(result));
        if(len < 0)
        {
            LOGE("mmitest write %s failed! write_len = %d.", PHONETXTPATH, len);
            goto out;
        }
        fsync(fd_whole);
    }

out:
    if(fd_pcba >= 0)
        close(fd_pcba);
    if(fd_whole >= 0)
        close(fd_whole);

    return 0;
}

static void detect_modem_control()
{
    int control_fd = -1;
    int  numRead;
    char buf[128];

reconnect:
    LOGD("try to connect socket %s...", SOCKET_NAME_MODEM_CTL);

    do
    {
        usleep(10 * 1000);
        control_fd = socket_local_client(SOCKET_NAME_MODEM_CTL,
        ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
        LOGD("connect socket %s control_fd=%d", SOCKET_NAME_MODEM_CTL, control_fd);
    }
    while(control_fd < 0);
    LOGD("connect socket %s success", SOCKET_NAME_MODEM_CTL);

    do
    {
        memset(buf, 0, sizeof(buf));
        LOGD("Monioring modem state on socket %s...", SOCKET_NAME_MODEM_CTL);
        do
        {
            numRead = read(control_fd, buf, sizeof(buf));
            LOGD("read %d from fd %d", numRead, control_fd);
        }
        while(numRead < 0 && errno == EINTR);

        if(numRead <= 0)
        {
            close(control_fd);
            goto reconnect;
        }

        LOGD("read numRead=%d, buf=%s", numRead, buf);
        if (strstr(buf, "Modem Alive"))
        {
            break;
        }
    }
    while(1);

    close(control_fd);
    return;
}

static int factrory_parse_cmdline(char * cmdvalue)
{
    int fd = 0, ret = 0;
    char cmdline[CMDLINE_SIZE] = {0};
    char *str = NULL;
    int val;

    if(cmdvalue == NULL)
    {
        LOGD("cmd_value = NULL");
        return -1;
    }
    fd = open("/proc/cmdline", O_RDONLY);
    if (fd >= 0)
    {
        if ((ret = read(fd, cmdline, sizeof(cmdline) - 1)) > 0)
        {
            cmdline[ret] = '\0';
            LOGD("cmdline %s", cmdline);
            str = strstr(cmdline, "modem=");
            if ( str != NULL)
            {
                str += (sizeof("modem=") - 1);
                *(strchr(str, ' ')) = '\0';
            }
            else
            {
                LOGD("cmdline 'modem=' is not exist");
                goto ERROR;
            }
            LOGD("cmdline len = %d, str=%s", strlen(str), str);
            if(!strcmp(cmdvalue, str))
                val = 1;
            else
                val = 0;
            close(fd);
            return val;
        }
        else
        {
            LOGD("cmdline modem=NULL");
            goto ERROR;
        }
    }
    else
    {
        LOGD("/proc/cmdline open error");
        return 0;
    }
ERROR:
    close(fd);
    return 0;
}

void result_init(){
    parse_config();
    test_result_mkdir();
    read_bin();
}

void log_init()
{
    pthread_t t;
    pthread_create(&t, NULL, test_printlog_thread, NULL);

}

void modem_init()
{
    pthread_t t;
    int row = 2;
    ui_fill_locked();
    ui_set_color(CL_GREEN);
    row = ui_show_text(row, 0, TEXT_TIP_MODEM_INIT);
    gr_flip();
    sleep(1);
    //if(factrory_parse_cmdline("shutdown"))
        //detect_modem_control();

    if (0 != pthread_create(&t, NULL, modem_init_func, NULL))
    {
        LOGE("mmitest create modem_init_func thread failed.");
    }
}

int main()
{
    LOGD("==== factory test start ====");

    //log_init();
    chnl_init();
    ui_init();
    input_init();
    result_init();
    modem_init();
    show_root_menu();

    LOGD("==== factory test start end ====");
    return 1;
}
