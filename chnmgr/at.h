#ifndef __TEST_ITEM_AT_H__
#define __TEST_ITEM_AT_H__

/******audio******/

#define TEST_AT_AUDIO_LOOP_FLAG "AT+AUDIONPILOOP"

#define TEST_AT_AUDIO_MAIN_LOOP_OPEN "AT+AUDIONPILOOP=1,2,1\r"
#define TEST_AT_AUDIO_MAIN_LOOP_CLOSE "AT+AUDIONPILOOP=0,2,1\r"

#define TEST_AT_AUDIO_ASSIS_LOOP_OPEN "AT+AUDIONPILOOP=1,2,2\r"
#define TEST_AT_AUDIO_ASSIS_LOOP_CLOSE "AT+AUDIONPILOOP=0,2,2\r"

#define TEST_AT_AUDIO_HEADSET_LOOP_OPEN "AT+AUDIONPILOOP=1,4,4\r"
#define TEST_AT_AUDIO_HEADSET_LOOP_CLOSE "AT+AUDIONPILOOP=0,4,4\r"

#define TEST_AUDIO_CALLBACK_DESC "send at command"

#define TEST_AT_AUDIO_RECEIVER_OPEN "AT+AUDIOPLAY=1,1,44100,2,\"/vendor/media/audio_sample.pcm\"\r"
#define TEST_AT_AUDIO_RECEIVER_CLOSE "AT+AUDIOPLAY=0\r"


/*****************/


/******headset****/

#define TEST_AT_HEADSET_TYPE "AT+HEADSETTYPE\r"

/*****************/

/******sensor****/

#define TEST_AT_SENSOR_TEST_FLAG "AT+SENSORTEST"
#define TEST_AT_SENSOR_TEST "AT+SENSORTEST="

#define TEST_AT_SENSOR_INFO_FLAG "AT+SENSORINFO"
#define TEST_AT_SENSOR_INFO "AT+SENSORINFO="

#define TEST_AT_SENSOR_CALI_FLAG "AT+SENSORCALI"
#define TEST_AT_SENSOR_CALI "AT+SENSORCALI="

/*****************/

/******bt****/

#define TEST_AT_BT_TEST_FLAG "AT+BTTEST"
#define TEST_AT_BT_TEST "AT+BTTEST\r"

/*****************/

/******WIFI****/

#define TEST_AT_WIFI_TEST_FLAG "AT+WIFITEST"
#define TEST_AT_WIFI_TEST "AT+WIFITEST\r"

/*****************/

/******GPS****/

#define TEST_AT_GPS_TEST_FLAG "AT+GPSTEST"
#define TEST_AT_GPS_TEST "AT+GPSTEST\r"

/*****************/

/******FM****/

#define TEST_AT_FM_TEST_FLAG "AT+FMTEST"
#define TEST_AT_FM_TEST "AT+FMTEST\r"

/*****************/

/******OTG****/

#define TEST_AT_OTG_TEST_FLAG "AT+OTGTEST"
#define TEST_AT_OTG_TEST "AT+OTGTEST\r"

/*****************/

/******FLASH****/

#define TEST_AT_STORAGE_TEST_FLAG "AT+STORAGETEST"
#define TEST_AT_STORAGE_TEST "AT+STORAGETEST\r"

/*****************/

/******Charge****/

#define TEST_AT_CHARGE_TEST_FLAG "AT+CHARGETEST"
#define TEST_AT_CHARGE_TEST "AT+CHARGETEST="

/*****************/

/******FlashLight****/

#define TEST_AT_FLASHLIGHT_TEST_FLAG "AT+FLASHLIGHTTEST"
#define TEST_AT_FLASHLIGHT_TEST "AT+FLASHLIGHTTEST="

/*****************/

/******BackLight****/

#define TEST_AT_BACKLIGHT_TEST_FLAG "AT+BACKLIGHTTEST"
#define TEST_AT_BACKLIGHT_TEST "AT+BACKLIGHTTEST="

/*****************/

/******LED****/

#define TEST_AT_LED_TEST_FLAG "AT+LEDTEST"
#define TEST_AT_LED_TEST "AT+LEDTEST="

/*****************/

/******tel****/

#define TEST_AT_TEL_TEST_FLAG "AT+TELTEST"
#define TEST_AT_TEL_TEST "AT+TELTEST="

/*****************/

/******SIM****/

#define TEST_AT_SIM_TEST_FLAG "AT+SIMTEST"
#define TEST_AT_SIM_TEST "AT+SIMTEST="

/*****************/

/******Lcd****/

#define TEST_AT_LCD_TEST_FLAG "AT+LCDTEST"
#define TEST_AT_LCD_TEST "AT+LCDTEST="

/*****************/


/******tp****/

#define TEST_AT_TP_TEST_FLAG "AT+TPTEST"
#define TEST_AT_TP_TEST "AT+TPTEST="

/*****************/

/******key****/

#define TEST_AT_KEY_TEST_FLAG "AT+KEYTEST"
#define TEST_AT_KEY_TEST "AT+KEYTEST="

/*****************/

/******vibrator****/

#define TEST_AT_VIBRATOR_TEST_FLAG "AT+VIBRATORTEST"
#define TEST_AT_VIBRATOR_TEST "AT+VIBRATORTEST="

/*****************/

/******version****/

#define TEST_AT_VERSION_TEST_FLAG "AT+VERSIONTEST"
#define TEST_AT_VERSION_TEST "AT+VERSIONTEST\r"

/*****************/

/******phoneinfo****/

#define TEST_AT_PHONEINFO_TEST_FLAG "AT+PHONEINFOTEST"
#define TEST_AT_PHONEINFO_TEST "AT+PHONEINFOTEST\r"

/*****************/

/******caliinfo****/

#define TEST_AT_CALIINFO_TEST_FLAG "AT+CALIINFOTEST"
#define TEST_AT_CALIINFO_TEST "AT+CALIINFOTEST\r"

/*****************/


/******RTC****/

#define TEST_AT_RTC_TEST_FLAG "AT+RTCTEST"
#define TEST_AT_RTC_TEST "AT+RTCTEST\r"

/*****************/

/******Camera****/

#define TEST_AT_CAMERA_TEST_FLAG "AT+CAMERATEST"
#define TEST_AT_CAMERA_TEST "AT+CAMERATEST\r"

/*****************/

/******NFC****/

#define TEST_AT_NFC_TEST_FLAG "AT+NFCTEST"
#define TEST_AT_NFC_TEST "AT+NFCTEST\r"

/*****************/

/******SoundTrigger****/

#define TEST_AT_SOUNDTRIGGER_TEST_FLAG "AT+SOUNDTRIGGERTEST"
#define TEST_AT_SOUNDTRIGGER_TEST "AT+SOUNDTRIGGERTEST\r"

/*****************/

/******finger****/

#define TEST_AT_FINGER_TEST_FLAG "AT+FINGERTEST"
#define TEST_AT_FINGER_TEST "AT+FINGERTEST\r"

/*****************/


#endif /*__TEST_ITEM_H_*/
