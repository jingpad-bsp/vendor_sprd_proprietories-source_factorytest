#include "testitem.h"

#define ENABLE_EXTERN_API 0

int test_lcd_start(void)
{
    if(chnl_is_support(TEST_AT_LCD_TEST_FLAG)){
        return test_lcd_start_extern();
    }else{
        return test_lcd_start_common();
    }
}

int test_msensor_start(void)
{
    if(chnl_is_support(TEST_AT_SENSOR_TEST_FLAG)){
        return test_msensor_start_extern();
    }else{
        return test_msensor_start_common();
    }
}

int test_key_start(void)
{
    if(chnl_is_support(TEST_AT_KEY_TEST_FLAG)){
        return test_key_start_extern();
    }else{
        return test_key_start_common();
    }
}

int test_vibrator_start(void)
{
    if(chnl_is_support(TEST_AT_VIBRATOR_TEST_FLAG)){
        return test_vibrator_start_extern();
    }else{
        return test_vibrator_start_common();
    }
}

int test_version_show(void)
{
    if(chnl_is_support(TEST_AT_VERSION_TEST_FLAG)){
        return test_version_show_extern();
    }else{
        return test_version_show_common();
    }
}

int test_phone_info_show(void)
{
    if(chnl_is_support(TEST_AT_PHONEINFO_TEST_FLAG)){
        return test_phone_info_show_extern();
    }else{
        return test_phone_info_show_common();
    }
}

int test_cali_info(void)
{
    if(chnl_is_support(TEST_AT_CALIINFO_TEST_FLAG)){
        return test_cali_info_extern();
    }else{
        return test_cali_info_common();
    }
}

int test_vb_bl_start(void)
{
    if(chnl_is_support(TEST_AT_BACKLIGHT_TEST_FLAG)){
        return test_vb_bl_start_extern();
    }else{
        return test_vb_bl_start_common();
    }
}

int test_led_start(void)
{
    if(chnl_is_support(TEST_AT_LED_TEST_FLAG)){
        return test_led_start_extern();
    }else{
        return test_led_start_common();
    }
}

int test_tp_start(void)
{
    if(chnl_is_support(TEST_AT_TP_TEST_FLAG)){
        return test_tp_start_extern();
    }else{
        return test_tp_start_common();
    }
}

int test_multi_touch_start(void)
{
    if(chnl_is_support(TEST_AT_TP_TEST_FLAG)){
        return test_multi_touch_start_extern();
    }else{
        return test_multi_touch_start_common();
    }
}

int test_mainloopback_start(void)
{
    if(chnl_is_support(TEST_AT_AUDIO_LOOP_FLAG)){
        return test_mainloopback_start_extern();
    }else{
        return test_mainloopback_start_common();
    }
}

int test_assisloopback_start(void)
{
    if(chnl_is_support(TEST_AT_AUDIO_LOOP_FLAG)){
        return test_assisloopback_start_extern();
    }else{
        return test_assisloopback_start_common();
    }
}

int test_sdcard_start(void)
{
    if(chnl_is_support(TEST_AT_STORAGE_TEST_FLAG)){
        return test_sdcard_start_extern();
    }else{
        return test_sdcard_start_common();
    }
}

int test_sdcard_pretest(void)
{
    if(chnl_is_support(TEST_AT_STORAGE_TEST_FLAG)){
        return test_sdcard_pretest_extern();
    }else{
        return test_sdcard_pretest_common();
    }
}

int test_emmc_start(void)
{
    if(chnl_is_support(TEST_AT_STORAGE_TEST_FLAG)){
        return test_emmc_start_extern();
    }else{
        return test_emmc_start_common();
    }
}

int test_rtc_start(void)
{
    if(chnl_is_support(TEST_AT_RTC_TEST_FLAG)){
        return test_rtc_start_extern();
    }else{
        return test_rtc_start_common();
    }
}

int test_gsensor_start(void)
{
    if(chnl_is_support(TEST_AT_SENSOR_TEST_FLAG)){
        return test_gsensor_start_extern();
    }else{
        return test_gsensor_start_common();
    }
}

int test_lsensor_start(void)
{
    if(chnl_is_support(TEST_AT_SENSOR_TEST_FLAG)){
        return test_lsensor_start_extern();
    }else{
        return test_lsensor_start_common();
    }
}

int test_pxysensor_start(void)
{
    if(chnl_is_support(TEST_AT_SENSOR_TEST_FLAG)){
        return test_pxysensor_start_extern();
    }else{
        return test_lsensor_start_common();
    }
}

int test_psensor_start(void)
{
    if(chnl_is_support(TEST_AT_SENSOR_TEST_FLAG)){
        return test_psensor_start_extern();
    }else{
        return test_psensor_start_common();
    }
}

int test_asensor_start(void)
{
    if(chnl_is_support(TEST_AT_SENSOR_TEST_FLAG)){
        return test_asensor_start_extern();
    }else{
        return test_asensor_start_common();
    }
}

int cali_asensor_start(void)
{
    if(chnl_is_support(TEST_AT_SENSOR_CALI_FLAG)){
        return cali_asensor_start_extern();
    }else{
        return cali_asensor_start_common();
    }
}

int cali_gsensor_start(void)
{
    if(chnl_is_support(TEST_AT_SENSOR_CALI_FLAG)){
        return cali_gsensor_start_extern();
    }else{
        return cali_gsensor_start_common();
    }
}

int cali_msensor_start(void)
{
    if(chnl_is_support(TEST_AT_SENSOR_CALI_FLAG)){
        return cali_msensor_start_extern();
    }else{
        return cali_msensor_start_common();
    }
}

int cali_lsensor_start(void)
{
    if(chnl_is_support(TEST_AT_SENSOR_CALI_FLAG)){
        return cali_lsensor_start_extern();
    }
    return 2;
}

int cali_prosensor_start(void)
{
    if(chnl_is_support(TEST_AT_SENSOR_CALI_FLAG) && ENABLE_EXTERN_API){
        return cali_prosensor_start_extern();
    }else{
        return cali_prosensor_start_common();
    }
}

int cali_auto_prosensor_start(void)
{
    if(chnl_is_support(TEST_AT_SENSOR_CALI_FLAG)){
        return cali_auto_prosensor_start_extern();
    }else{
        return cali_auto_prosensor_start_common();
    }
}

int test_bcamera_start(void)
{
    if(chnl_is_support(TEST_AT_CAMERA_TEST_FLAG)){
        return test_bcamera_start_extern();
    }else{
        return test_bcamera_start_common();
    }
}

int test_acamera_start(void)
{
    if(chnl_is_support(TEST_AT_CAMERA_TEST_FLAG)){
        return test_acamera_start_extern();
    }else{
        return test_acamera_start_common();
    }
}

int test_facamera_start(void)
{
    if(chnl_is_support(TEST_AT_CAMERA_TEST_FLAG)){
        return test_facamera_start_extern();
    }else{
        return test_facamera_start_common();
    }
}

int test_fcamera_start(void)
{
    if(chnl_is_support(TEST_AT_CAMERA_TEST_FLAG)){
        return test_fcamera_start_extern();
    }else{
        return test_fcamera_start_common();
    }
}

int test_third_camera_start(void)
{
	return test_camera_start_common(3);
}

int test_flash_start(void)
{
    if(chnl_is_support(TEST_AT_FLASHLIGHT_TEST_FLAG)){
        return test_flash_start_extern();
    }else{
        return test_flash_start_common();
    }
}

int test_charge_start(void)
{
    if(chnl_is_support(TEST_AT_CHARGE_TEST_FLAG)){
        return test_charge_start_extern();
    }else{
        return test_charge_start_common();
    }
}

int test_headset_start(void)
{
    if(chnl_is_support(TEST_AT_HEADSET_TYPE)){
        return test_headset_start_extern();
    }else{
        return test_headset_start_common();
    }
}

int test_tel_start(void)
{
    if(chnl_is_support(TEST_AT_TEL_TEST_FLAG)){
        return test_tel_start_extern();
    }else{
        return test_tel_start_common();
    }
}

int test_otg_start(void)
{
    if(chnl_is_support(TEST_AT_OTG_TEST_FLAG)){
        return test_otg_start_extern();
    }else{
        return test_otg_start_common();
    }
}

int test_nfc_start(void)
{
    if(chnl_is_support(TEST_AT_NFC_TEST_FLAG)){
        return test_nfc_start_extern();
    }else{
        return test_nfc_start_common();
    }
}

int test_fm_start(void)
{
    if(chnl_is_support(TEST_AT_FM_TEST_FLAG)){
        return test_fm_start_extern();
    }else{
        return test_fm_start_common();
    }
}

int test_bt_start(void)
{
    if(chnl_is_support(TEST_AT_BT_TEST_FLAG)){
        return test_bt_start_extern();
    }else{
        return test_bt_start_common();
    }
}

int test_bt_pretest(void)
{
    if(chnl_is_support(TEST_AT_BT_TEST_FLAG)){
        return test_bt_pretest_extern();
    }else{
        return test_bt_pretest_common();
    }
}

int test_wifi_start(void)
{
    if(chnl_is_support(TEST_AT_WIFI_TEST_FLAG)){
        return test_wifi_start_extern();
    }else{
        return test_wifi_start_common();
    }
}

int test_wifi_pretest(void)
{
    if(chnl_is_support(TEST_AT_WIFI_TEST_FLAG)){
        return test_wifi_pretest_extern();
    }else{
        return test_wifi_pretest_common();
    }
}

int test_gps_start(void)
{
    if(chnl_is_support(TEST_AT_GPS_TEST_FLAG)){
        return test_gps_start_extern();
    }else{
        return test_gps_start_common();
    }
}

int test_gps_pretest(void)
{
    if(chnl_is_support(TEST_AT_GPS_TEST_FLAG)){
        return test_gps_pretest_extern();
    }else{
        return test_gps_pretest_common();
    }
}

int test_flashlight_start(void)
{
    if(chnl_is_support(TEST_AT_FLASHLIGHT_TEST_FLAG)){
        return test_flashlight_start_extern();
    }else{
        return test_flashlight_start_common();
    }
}

int test_sim_start(void)
{
    if(chnl_is_support(TEST_AT_SIM_TEST_FLAG)){
        return test_sim_start_extern();
    }else{
        return test_sim_start_common();
    }
}

int test_sim_pretest(void)
{
    if(chnl_is_support(TEST_AT_SIM_TEST_FLAG)){
        return test_sim_pretest_extern();
    }else{
        return test_sim_pretest_common();
    }
}

int test_receiver_start(void)
{
    if(chnl_is_support(TEST_AT_AUDIO_LOOP_FLAG)){
        return test_receiver_start_extern();
    }else{
        return test_receiver_start_common();
    }
}

int test_speaker_start(void)
{
    if(chnl_is_support(TEST_AT_AUDIO_LOOP_FLAG)){
        return test_speaker_start_extern();
    }else{
        return test_speaker_start_common();
    }
}

int test_soundtrigger_start(void)
{
    if(chnl_is_support(TEST_AT_SOUNDTRIGGER_TEST_FLAG)){
        return test_soundtrigger_start_extern();
    }else{
        return test_soundtrigger_start_common();
    }
}

int test_fingersor_start(void)
{
    if(chnl_is_support(TEST_AT_FINGER_TEST_FLAG)){
        return test_fingersor_start_extern();
    }else{
        return test_fingersor_start_common();
    }
}
