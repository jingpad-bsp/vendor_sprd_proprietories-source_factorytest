/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RECOVERY_COMMON_H
#define RECOVERY_COMMON_H

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
#include <ctype.h>
#include <wait.h>
#include <semaphore.h>
#include <cutils/android_reboot.h>
#include <cutils/properties.h>
#include <cutils/sockets.h>
#include <utils/Log.h>
#include <hardware/hardware.h>
#include <linux/input.h>
#include <sys/statfs.h>
#include <dlfcn.h>
#include <system/audio.h>
#include <string.h>

#define NO_ACTION           -1
#define HIGHLIGHT_UP        -2
#define HIGHLIGHT_DOWN      -3
#define SELECT_ITEM         -4
#define GO_BACK             -5
#define GO_HOME             -6
#define NEXT_PAGE           -7
#define KEY_VIR_PASS        105
#define KEY_VIR_FAIL        106
#define KEY_VIR_BACK        106
#define KEY_VIR_NEXT_PAGE   107
#define KEY_VIR_ITEMS       1

enum {
    BACKGROUND_ICON_NONE,
    BACKGROUND_ICON_INSTALLING,
    BACKGROUND_ICON_ERROR,
    NUM_BACKGROUND_ICONS
};

enum MENU_TYPE
{
    ROOT_MENU = 1,
    PCBA_TEST_MENU = 11,
    PHONE_TEST_MENU = 12,
    SUGGEST_TEST_MENU = 13,
    PHONE_VERSION_MENU = 14,
    PCBA_RESULT_MENU = 15,
    PHONE_RESULT_MENU = 16,
    SUGGEST_RESULT_MENU = 17,
};

typedef enum{
    CL_WHITE,
    CL_BLACK,
    CL_RED,
    CL_BLUE,
    CL_GREEN,
    CL_YELLOW,
    CL_SCREEN_BG,
    CL_SCREEN_FG,
    CL_TITLE_BG,
    CL_TITLE_FG,
    CL_MENU_HL_BG,
    CL_MENU_HL_FG,
}UI_COLOR;

#undef LOG_TAG
#define LOG_TAG  "FACTORY"
#define LOGD(format, ...)  ALOGD("%s: "  format "\n" , __func__, ## __VA_ARGS__)
#define LOGE(format, ...)  ALOGE("%s: "  format "[%d][%s][%d]\n", __func__,  ## __VA_ARGS__, errno,strerror(errno),__LINE__)
#define LOGI(format, ...)  ALOGI("%s: "  format "\n", __func__,  ## __VA_ARGS__)
#define LOGV(format, ...)  ALOGV("%s: "  format "\n", __func__,  ## __VA_ARGS__)

#define SPRD_DBG       LOGV

#define STRINGIFY(x) #x
#define EXPAND(x) STRINGIFY(x)

/**************Common define************/
#define AT_BUFFER_SIZE  2048
#define BUF_LEN         64
#define S_ON    1
#define S_OFF   0

#ifndef uchar
typedef unsigned char  uchar;
#endif

enum {
    RL_NA = 0,
    RL_PASS = 1,
    RL_FAIL = 2,
    RL_NS = 3,
    RL_NEXT_PAGE = 4,
    RL_BACK = 5,
};

//the state of USB, Headset or OTG
enum {
    OFF = 0,
    ON = 1,
};

#include "ui.h"
#include "at.h"
#include "waitkey.h"
#include "resource.h"
#include "dev_header.h"

typedef enum
{
    CASE_TEST_LCD,
    CASE_TEST_TP,
    CASE_TEST_MULTITOUCH,
    CASE_TEST_KEY,
    CASE_TEST_VIBRATOR,
    CASE_TEST_BACKLIGHT,
    CASE_TEST_FCAMERA,
    CASE_TEST_BCAMERA,
    CASE_TEST_FLASH,
    CASE_TEST_MAINLOOP,
    CASE_TEST_ASSISLOOP,
    CASE_TEST_SPEAKER,
    CASE_TEST_RECEIVER,
    CASE_TEST_HEADSET,
    CASE_TEST_SDCARD,
    CASE_TEST_SIMCARD,
    CASE_TEST_CHARGE,
    CASE_TEST_WIRELESSCHARGER,
    CASE_TEST_FM,
    CASE_TEST_ATV,
    CASE_TEST_DTV,
    CASE_TEST_BT,
    CASE_TEST_WIFI,
    CASE_TEST_GPS,
    CASE_TEST_RTC,
    CASE_TEST_OTG,
    CASE_TEST_TEL,
    CASE_TEST_NFC,
    CASE_TEST_CALIBINFO,
    CASE_TEST_SOFTCHECK,
    CASE_TEST_IRREMOTE,
    CASE_TEST_ACCSOR,
    CASE_TEST_MAGSOR,
    CASE_TEST_ORISOR,
    CASE_TEST_GYRSOR,
    CASE_TEST_LPSOR,
    CASE_TEST_PRESSOR,
    CASE_TEST_TEMPESOR,
    CASE_TEST_GSENSOR,
    CASE_TEST_LSENSOR,
    CASE_TEST_RVSOR,
    CASE_TEST_FINGERSOR,
    CASE_TEST_HUMISOR,
    CASE_TEST_HALLSOR,
    CASE_TEST_LED,
    CASE_TEST_EMMC,
    CASE_TEST_SOUNDTRIGGER,
    CASE_TEST_FLASHLIGHT,
    CASE_TEST_PXYPSOR,
    CASE_TEST_FACAMERA,
    CASE_TEST_ACAMERA,
    CASE_CALI_ACCSOR,
    CASE_CALI_GYRSOR,
    CASE_CALI_MAGSOR,
    CASE_CALI_PROSOR,
    CASE_CALI_LSOR,
    CASE_TEST_THIRD_CAMERA,
    CASE_TEST_FORCUST6,
    CASE_TEST_FORCUST7,
    CASE_TEST_FORCUST8,
    CASE_TEST_FORCUST9,
    CASE_TEST_FORCUST10,
    CASE_TEST_FORCUST11,
    FINAL_RESULT_FLAG,
    TOTAL_NUM
}CASE_TEST_LIST;

typedef enum
{
    RESULT_NOT_TEST = 0,
    RESULT_PASS,
    RESULT_FAIL,
    RESULT_INVALID
}RESULT_TEST_LIST;

typedef struct hardware_result{
    char id;
    char support;
}hardware_result;

hardware_result* getHardwareRes();
void save_result(unsigned char id,char key_result);
char* getSensorDevName();
char* getSensroTypeName(int type);

#endif  // RECOVERY_COMMON_H
