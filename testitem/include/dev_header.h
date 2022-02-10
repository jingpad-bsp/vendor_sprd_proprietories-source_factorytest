#ifndef __TEST_DEV_HEADER_H__
#define __TEST_DEV_HEADER_H__

/**************Open Virtual key************/
#define SPRD_VIRTUAL_TOUCH

/**************Backlight************/
#define LCD_BACKLIGHT_DEV            "/sys/class/backlight/sprd_backlight/brightness"
#define KEY_BACKLIGHT_DEV             "/sys/class/leds/keyboard-backlight/brightness"
#define LCD_BACKLIGHT_MAX_DEV        "/sys/class/backlight/sprd_backlight/max_brightness"
#define KEY_BACKLIGHT_MAX_DEV     "/sys/class/leds/keyboard-backlight/max_brightness"
/*************end Backlight*********/

/************Camera************/
#define LIBRARY_PATH        "/vendor/lib/hw/"
//#define FLASH_SUPPORT "/sys/class/flash_test/flash_test/flash_value"
/*********end Bcamera***********/

/************BT*************/
#define BT_MAC_CONFIG_FILE_PATH            "/data/misc/bluedroid/btmac.txt"
#define BT_MAC_FACTORY_CONFIG_FILE_PATH  "/mnt/vendor/btmac.txt"
#define BD_ADDR_LEN    6                    /* Device address length */
//#define BD_NAME_LEN     248                    /* Device name length */
typedef unsigned char BD_ADDR[BD_ADDR_LEN];       /* Device address */
typedef unsigned char*BD_ADDR_PTR;                /* Pointer to Device Address */
//typedef unsigned char BD_NAME[BD_NAME_LEN + 1];   /* Device name */
//typedef unsigned char*BD_NAME_PTR;                /* Pointer to Device name */
#define MAC_LEN     (6)
#define MAX_REMOTE_DEVICES (10)
#define CASE_RETURN_STR(const) case const: return #const;
/******************BT end***************/

/**************Charge******************/
#define SPRD_CALI_MASK            0x00000200

#define ENG_BATVOL            "/sys/class/power_supply/battery/real_time_voltage"
#define ENG_CHRVOL            "/sys/class/power_supply/battery/charger_voltage"
#define ENG_CURRENT        "/sys/class/power_supply/battery/real_time_current"
#define ENG_USBONLINE    "/sys/class/power_supply/usb/online"
#define ENG_ACONLINE    "/sys/class/power_supply/ac/online"
#define ENG_BATONLINE   "/sys/class/power_supply/battery/health"
#define ENG_STOPCHG        "/sys/class/power_supply/battery/stop_charge"
#define ENG_BATCURRENT  "/sys/class/power_supply/sprdfgu/fgu_current"

#define ENG_BATVOL_k414            "/sys/class/power_supply/battery/voltage_now"
#define ENG_CHRVOL_k414            "/sys/class/power_supply/sc27xx-fgu/constant_charge_voltage"
#define ENG_CURRENT_k414        "/sys/class/power_supply/battery/current_now"
#define ENG_USBONLINE_k414    "/sys/class/power_supply/usb/online"
#define ENG_ACONLINE_k414    "/sys/class/power_supply/ac/online"
#define ENG_BATONLINE_k414   "/sys/class/power_supply/battery/health"
#define ENG_STOPCHG_k414        "/sys/class/power_supply/battery/charger.0/stop_charge"
#define ENG_BATCURRENT_k414  "/sys/class/power_supply/sc27xx-fgu/current_now"
#define ENG_BATSTATUS_k414   "/sys/class/power_supply/battery/status"
/****************end charge*************/

/**************FM5.1******************/

//#define FM_INSMOD_COMMEND    "insmod /system/lib/modules/trout_fm.ko"

#define FM_IOCTL_BASE           'R'
#define FM_IOCTL_ENABLE       _IOW(FM_IOCTL_BASE, 0, int)
#define FM_IOCTL_GET_ENABLE   _IOW(FM_IOCTL_BASE, 1, int)
#define FM_IOCTL_SET_TUNE     _IOW(FM_IOCTL_BASE, 2, int)
#define FM_IOCTL_GET_FREQ     _IOW(FM_IOCTL_BASE, 3, int)
#define FM_IOCTL_SEARCH       _IOW(FM_IOCTL_BASE, 4, int[4])
#define FM_IOCTL_STOP_SEARCH  _IOW(FM_IOCTL_BASE, 5, int)
#define FM_IOCTL_SET_VOLUME   _IOW(FM_IOCTL_BASE, 7, int)
#define FM_IOCTL_GET_VOLUME   _IOW(FM_IOCTL_BASE, 8, int)
#define Trout_FM_IOCTL_CONFIG _IOW(FM_IOCTL_BASE, 9, int)
#define FM_IOCTL_GET_RSSI     _IOW(FM_IOCTL_BASE, 10, int)

//#ifdef PLATFORM_VERSION6
#define TROUT_FM_DEV_NAME           "/dev/fm"
//#else
//#define TROUT_FM_DEV_NAME               "/dev/Trout_FM"
//#endif
/******************END FM5.1*******************/

/**************FM6.0******************/
typedef enum fm_bool {
    fm_false = 0,
    fm_true  = 1
} fm_bool;

typedef struct fm_tune_parm {
    uint8_t err;
    uint8_t band;
    uint8_t space;
    uint8_t hilo;
    uint16_t freq;
}fm_tune_parm;

typedef struct fm_seek_parm {
    uint8_t err;
    uint8_t band;
    uint8_t space;
    uint8_t hilo;
    uint8_t seekdir;
    uint8_t seekth;
    uint16_t freq;
}fm_seek_parm;

typedef struct fm_softmute_tune_t {
    int32_t rssi; // RSSI of current channel
    uint32_t freq; // current frequency
    fm_bool valid; // current channel is valid(true) or not(false)
}fm_softmute_tune_t;

// band
#define FM_BAND_UNKNOWN 0
#define FM_BAND_UE      1 // US/Europe band  87.5MHz ~ 108MHz (DEFAULT)
#define FM_BAND_JAPAN   2 // Japan band      76MHz   ~ 90MHz
#define FM_BAND_JAPANW  3 // Japan wideband  76MHZ   ~ 108MHz
#define FM_BAND_SPECIAL 4 // special   band  between 76MHZ   and  108MHz
#define FM_BAND_DEFAULT FM_BAND_UE

#define FM_UE_FREQ_MIN  875
#define FM_UE_FREQ_MAX  1080
#define FM_JP_FREQ_MIN  760
#define FM_JP_FREQ_MAX  1080
#define FM_FREQ_MIN  FM_UE_FREQ_MIN
#define FM_FREQ_MAX  FM_JP_FREQ_MAX

// auto HiLo
#define FM_AUTO_HILO_OFF    0
#define FM_AUTO_HILO_ON     1

// seek direction
#define FM_SEEK_UP          1
#define FM_SEEK_DOWN        0

// seek threshold
#define FM_SEEKTH_LEVEL_DEFAULT 4

#define FM_SEEK_SPACE      1 // FM radio seek space,1:100KHZ; 2:200KHZ
#define FM_VOICE_MUTE    1
#define FM_VOICE_ON    0
#define FM_IOC_MAGIC        0xf5

#define FM_IOCTL_POWERUP       _IOWR(FM_IOC_MAGIC, 0, struct fm_tune_parm*)
#define FM_IOCTL_POWERDOWN     _IOWR(FM_IOC_MAGIC, 1, int32_t*)
#define FM_IOCTL_TUNE          _IOWR(FM_IOC_MAGIC, 2, struct fm_tune_parm*)
#define FM_IOCTL_SEEK          _IOWR(FM_IOC_MAGIC, 3, struct fm_seek_parm*)
#define FM_IOCTL_SETVOL        _IOWR(FM_IOC_MAGIC, 4, uint32_t*)
#define FM_IOCTL_GETVOL        _IOWR(FM_IOC_MAGIC, 5, uint32_t*)
#define FM_IOCTL_MUTE          _IOWR(FM_IOC_MAGIC, 6, uint32_t*)
#define FM_IOCTL_GETRSSI       _IOWR(FM_IOC_MAGIC, 7, int32_t*)
//#define FM_IOCTL_SCAN          _IOWR(FM_IOC_MAGIC, 8, struct fm_scan_parm*)
#define FM_IOCTL_STOP_SCAN     _IO(FM_IOC_MAGIC,   9)
//#define FM_IOCTL_SOFT_MUTE_TUNE _IOWR(FM_IOC_MAGIC, 63, struct fm_softmute_tune_t*)/*for soft mute tune*/

/******************END FM6.0*******************/


#define SPRD_HEADSETOUT             0
#define SPRD_HEADSETIN              1

typedef enum
{
    HEADSET_CLOSE = 0,
    HEADSET_OPEN ,
    HEADSET_CHECK,
    HEADSET_INVALID
}HEADSET_CMD_TYPE;

#define STATE_CLEAN                 0
#define STATE_DISPLAY               1
#define MAX_NAME_LEN 4096

#define START_FRQ    10410
#define END_FRQ     8750
#define THRESH_HOLD 100
#define DIRECTION   128
#define SCANMODE    0
#define MUTI_CHANNEL false
#define CONTYPE     0
#define CONVALUE    0
/******************END FM*******************/

/****************GPS****************/
#ifdef FUNCTION_DEBUG
#define FUN_ENTER    LOGD("%s enter \n",__FUNCTION__);
#define FUN_EXIT     LOGD("%s exit \n",__FUNCTION__);
#else
#define FUN_ENTER
#define FUN_EXIT
#endif

#define GPSNATIVETEST_WAKE_LOCK_NAME  "gps_native_test"
#define GPS_TEST_PASS  TEXT_TEST_PASS   //"PASS"
#define GPS_TEST_FAILED  TEXT_TEST_FAIL //"FAILED"
#define GPS_TESTING  TEXT_BT_SCANING    //"TESTING......"
#define GPS_TEST_TIME_OUT         (30) // s120
/********************end GPS*************/

/*****************AGsensor*************/
#define SPRD_GSENSOR_OFFSET                    2
#define SPRD_GSENSOR_1G                        5
#define SPRD_ASENSOR_OFFSET                    1.5
#define SPRD_ASENSOR_1G                        10

#define ASENSOR_TIMEOUT                     20
#define GSENSOR_TIMEOUT                     20
/***************end AGsensor************/

/****************Headerset***********/
#define SPRD_HEADSET_SWITCH_DEV         "/sys/class/switch/h2w/state"
#define SPRD_HEADSET_DEV                "sprdphone-Jack"
#define SPRD_HEASETKEY_DEV                "headset-keyboard"
#define SPRD_HEADSET_KEY                   KEY_MEDIA
#define SPRD_HEADSET_KEYLONGPRESS        KEY_END
#define SPRD_HEADSET_INSERT                SW_HEADPHONE_INSERT
#define SPRD_HEADPHONE_MIC                4
#define SPRD_HEADSET_SYNC                SYN_REPORT
/**************end Headerset***********/

/***************key**************/
#define KEY_TIMEOUT 20
/***************end key**********/

enum sensor_type
{
    SENSOR_TS,
    SENSOR_ACC,
    SENSOR_PXY,
    SENSOR_LUX,
    SENSOR_HDE,
    SENSOR_HDK,
    SENSOR_GYROSCOPE,
    SENSOR_MAGNETIC,
    SENSOR_ORIENTATION,
    SENSOR_PRESSURE,
    SENSOR_TEMPERATURE,
    SENSOR_NUM
};

/**************Modem**********/
#define PROP_MODEM_W_COUNT  "ro.vendor.modem.w.count"
//#define PROP_MODEM_LTE_COUNT  "ro.vendor.modem.l.count"
#define PROP_MODEM_PHONE_COUNT  "persist.vendor.radio.phone_count"
#define PROP_MODEM_LTE_ENABLE  "persist.vendor.modem.l.enable"
#define SOCKET_NAME_WCND "wcnd"
#define SOCKET_NAME_SLOG "slogmodem"


#define MAX_MODEM_COUNT     2
/*************end modem************/

/***************key*************/
struct test_key {
    int key;
    int done;
    char key_name[BUF_LEN];
    char key_shown[BUF_LEN];
};

enum key_type
{
    KEY_TYPE_POWER,
    KEY_TYPE_VOLUMEDOWN,
    KEY_TYPE_VOLUMEUP,
    KEY_TYPE_CAMERA,
    KEY_TYPE_HOME,
    KEY_TYPE_MENU,
    KEY_TYPE_BACK,
    KEY_TYPE_NUM
};
/***************end key*************/

/***************sdcard*************/
#ifdef CONFIG_NAND
#define SPRD_SD_DEV            "/dev/block/mmcblk0p1"
#define SPRD_SD_DEV_SIZE    "/sys/block/mmcblk1/size"
#define SPRD_MOUNT_DEV        "mount -t vfat /dev/block/mmcblk0p1  /sdcard"
#else
#define SPRD_SD_DEV            "/dev/block/mmcblk1p1"
#define SPRD_SD_DEV_SIZE    "/sys/block/mmcblk1/size"
#define SPRD_MOUNT_DEV        "mount -t vfat /dev/block/mmcblk1p1  /sdcard"
#define SPRD_SD_MOUNT_DEV    "/dev/block/platform/sdio_sd/mmcblk1p1"
#endif
#define SPRD_MOUNT_TMP_PATH       "/mnt/nativemmi_sdcard"

#define RW_LEN    512
/******************end sdcard*************/

#define ABS_MT_POSITION_X            0x35    /* Center X ellipse position */
#define ABS_MT_POSITION_Y            0x36    /* Center Y ellipse position */
#define ABS_MT_TOUCH_MAJOR            0x30
#define SYN_REPORT                    0
#define SYN_MT_REPORT                2
#define SENSOR_TS_PATH                "/sys/touchscreen/input_name"
/****************end tp***************/


/****************version**********/
#define PROP_ANDROID_VER    "ro.build.version.release"
#define PROP_SPRD_VER        "ro.build.description"
#define PROP_PRODUCT_VER    "ro.product.model"
#define PATH_LINUX_VER        "/proc/version"
/**************end version***********/


/***************vibrator***********/
#define VIBRATOR_ENABLE_DEV            "/sys/class/timed_output/vibrator/enable"
/*************end vibartor********/

/****************TEL************************/
#define TEL_TIMEOUT       (100)
/****************end TEL*******************/

/****************Sound Trigger************************/
#define TRI_TIMEOUT    (30)
/****************end Sound Trigger*******************/

/************wifi*****************/
#define WIFI_ADDRESS        "/sys/class/net/wlan0/address"
#define WIFI_MAC_CONFIG_FILE_PATH  "/data/misc/wifi/wifimac.txt"
#define WIFI_MAC_FACTORY_CONFIG_FILE_PATH  "/mnt/vendor/wifimac.txt"
/*************end wifi********/

/****************OTG*************/
#define USB_FILE_PATH "/vendor/etc/permissions/android.hardware.usb.host.xml"
#define OTG_FILE_PATH  "/sys/bus/platform/drivers/dwc_otg/is_support_otg"
#define TPYEC_OTG_ENABLE_PATH  "/sys/class/dual_role_usb/sprd_dual_role_usb/device/bbat_typec_ctrl"
#define OTG_INSERT_STATUS "sys/devices/20200000.usb/otg_status"
#define OTG_TPYEC_STATUS_PATH  "/sys/class/dual_role_usb/sprd_dual_role_usb/data_role"
#define OTG_DEVICE_HOST "sys/devices/20200000.usb/mode"
#define USB_STATUS_PATH "/sys/class/android_usb/android0/state"
#define USB3_OTG_ENABLE_PATH "/sys/devices/soc/soc:ap-ahb/20500000.usb3/host_enable"
#define OTG_USB3_STATUS_PATH "/sys/devices/soc/soc:ap-ahb/20500000.usb3/otg_status"
#define OTG_TESTFILE_PATH "/system/bin/test"

#define OTG_FILE_PATH_K414 "/sys/class/typec/port0/port_type"
#define OTG_TPYEC_STATUS_PATH_K414 "/sys/class/typec/port0/data_role"
#define OTG_FILE_PATH_PARTNER_K414 "/sys/class/typec/port0-partner"
/****************end OTG**********/

/****************NFC*************/
#define NFC_TIMEOUT (30)        //max time 30 seconds

enum tag_type
{
    UNKNOWN_TYPE = 0,
    TYPE1 = 1,
    TYPE2 = 2,
    TYPE4 = 4
};
/****************end NFC**********/

/******************Phase check*************/
#define SP09_MAX_SN_LEN                 (24)
#define SP09_MAX_STATION_NUM            (15)
#define SP09_MAX_STATION_NAME_LEN       (10)
#define SP09_SPPH_MAGIC_NUMBER          (0X53503039)    // "SP09"
#define SP05_SPPH_MAGIC_NUMBER          (0x53503035)    // "SP05"
#define SP09_MAX_LAST_DESCRIPTION_LEN   (32)

typedef struct _tagSP09_PHASE_CHECK {
    unsigned int Magic; // "SP09"
    char SN1[SP09_MAX_SN_LEN];      // SN , SN_LEN=24
    char SN2[SP09_MAX_SN_LEN];      // add for Mobile
    int StationNum;         // the test station number of the testing
    char StationName[SP09_MAX_STATION_NUM][SP09_MAX_STATION_NAME_LEN];
    unsigned char Reserved[13];     //
    unsigned char SignFlag;
    char szLastFailDescription[SP09_MAX_LAST_DESCRIPTION_LEN];
    unsigned short iTestSign;       // Bit0~Bit14 ---> station0~station 14
    //if tested. 0: tested, 1: not tested
    unsigned short iItem;   // part1: Bit0~ Bit_14 indicate test Station, 0: Pass, 1: fail
} SP09_PHASE_CHECK_T, *LPSP09_PHASE_CHECK_T;

/*add the struct add define to support the sp15*/
#define SP15_MAX_SN_LEN                 (64)
#define SP15_MAX_STATION_NUM            (20)
#define SP15_MAX_STATION_NAME_LEN       (15)
#define SP15_SPPH_MAGIC_NUMBER          (0X53503135)    // "SP15"
#define SP15_MAX_LAST_DESCRIPTION_LEN   (32)

typedef struct _tagSP15_PHASE_CHECK {
    unsigned int Magic;     // "SP15"
    char SN1[SP15_MAX_SN_LEN];  // SN , SN_LEN=64
    char SN2[SP15_MAX_SN_LEN];  // add for Mobile
    int StationNum;             // the test station number of the testing
    char StationName[SP15_MAX_STATION_NUM][SP15_MAX_STATION_NAME_LEN];
    unsigned char Reserved[13]; //
    unsigned char SignFlag;
    char szLastFailDescription[SP15_MAX_LAST_DESCRIPTION_LEN];
    unsigned int iTestSign;    // Bit0~Bit14 ---> station0~station 14
    //if tested. 0: tested, 1: not tested
    unsigned int iItem;        // part1: Bit0~ Bit_14 indicate test Station, 0: Pass, 1: fail
} SP15_PHASE_CHECK_T, *LPSP15_PHASE_CHECK_T;
/****************end Phase check*************/

/******************wifi*************/
typedef struct wifi_ap_t {
    char smac[32]; // string mac
    char name[64]; //wifi name
    int  sig_level;
    int frequency;
}wifi_ap_t;

#define WIFI_STATUS_UNKNOWN     0x0001
#define WIFI_STATUS_OPENED      0x0002
#define WIFI_STATUS_SCANNING    0x0004
#define WIFI_STATUS_SCAN_END    0x0008

#define WIFI_CMD_RETRY_NUM      3
#define WIFI_CMD_RETRY_TIME     1 // second
#define WIFI_MAX_AP             20
#define EVT_MAX_LEN 127
/******************wifi end*************/

#define INPUT_EVT_NAME "/dev/input/event"

typedef struct display_content
{
    int color;
    const char* content;
}DISPLAY_CNT, *PDISPLAY_CNT;

#endif 
