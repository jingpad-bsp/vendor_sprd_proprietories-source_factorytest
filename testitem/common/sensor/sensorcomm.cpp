#include "common.h"
#include "sensorcomm.h"

//#include <sensor/Sensor.h>
#include <stdint.h>
#include <sys/types.h>
#include <utils/KeyedVector.h>
#include <utils/Singleton.h>
#include <utils/String8.h>
#include <utils/StrongPointer.h>
#include <unordered_map>
#include <algorithm> //std::max std::min
#include <thread>

#include <sensors/convert.h>
#include <android/hardware/sensors/1.0/ISensors.h>


using android::hardware::hidl_vec;
using namespace android::hardware::sensors::V1_0;
using namespace android::hardware::sensors::V1_0::implementation;

// ---------------------------------------------------------------------------

using namespace android;
using android::hardware::Return;

// ---------------------------------------------------------------------------

Vector<sensor_t> list;
sp<android::hardware::sensors::V1_0::ISensors> mSensors;

int count = 0;
int senloaded = -1;

#define S_ON    1
#define S_OFF   0

const char* stristr(const char* str, const char* subStr)
{
    int len = strlen(subStr);
    if(len == 0)
    {
        return NULL;
    }

    while(*str)
    {
        if(strncasecmp(str, subStr, len) == 0)
        {
            return str;
        }
        ++str;
    }
    
    return NULL;
}

void handleHidlDeath(const std::string & detail) {
    // restart is the only option at present.
    LOG_ALWAYS_FATAL("Abort due to ISensors hidl service failure, detail: %s.", detail.c_str());
}

template<typename T>
static Return<T> checkReturn(Return<T> &&ret) {
    if (!ret.isOk()) {
        handleHidlDeath(ret.description());
    }
    return std::move(ret);
}

int sensor_getlist()
{
    float minPowerMa = 0.001; // 1 microAmp

    checkReturn(mSensors->getSensorsList(
            [&](const auto &outlist) {
                count = outlist.size();

                for (size_t i=0 ; i < count; i++) {
                    sensor_t sensor;
                    convertToSensor(outlist[i], &sensor);
                    // Sanity check and clamp power if it is 0 (or close)
                    if (sensor.power < minPowerMa) {
                        ALOGE("Reported power %f not deemed sane, clamping to %f",
                              sensor.power, minPowerMa);
                        sensor.power = minPowerMa;
                    }
                    list.push_back(sensor);

                    //checkReturn(mSensors->activate(outlist[i].sensorHandle, 0 /* enabled */));
                }
            }));
    return 1;
}

int get_sensor_name(const char * name )
{
    int fd = -1;
    int i = 0;
    char devName[64] = { 0 };
    char EvtName[64] = { 0 };
    struct stat stt;

    int EvtNameLen = strlen(INPUT_EVT_NAME);
    strcpy(EvtName, INPUT_EVT_NAME);
    EvtName[EvtNameLen + 1] = 0;

    for(i = 0; i < 16; ++i ) {
        EvtName[EvtNameLen] = (char)('0' + i);
        LOGD("input evt name = %s", EvtName);

        if( stat(EvtName, &stt) != 0 ) {
            LOGE("stat '%s' error!",EvtName);
            break;
        }

        fd = open(EvtName, O_RDONLY);
        if( fd < 0 ) {
            LOGE("Failed to open %s", EvtName);
            continue;
        }

        if( ioctl(fd, EVIOCGNAME(sizeof(devName)), devName) > 0 &&  strstr(devName, name) != NULL ) {
            LOGD("open '%s' OK, dev fd = %d", devName, fd);
            break;
        }

        LOGD("input evt name = %s, dev name = %s", EvtName, devName);
        close(fd);
        fd = -1;
    }

    if( fd >= 0 ) {
        if( fcntl(fd, F_SETFL, O_NONBLOCK) < 0 ) {
            LOGE("fcntl: set to nonblock error!");
        }
    }

    return fd;
}
/* UNISOC: Bug1568329 It needs to be compatible with different types of TP @{ */
char* get_sensor_ts_name()
{
    int fp;
    static char support[64] = { 0 };
    char *tmp = NULL;
    if (access(SENSOR_TS_PATH, F_OK) != -1)
    {
        if ((fp = open(SENSOR_TS_PATH,O_RDONLY)) >= 0)
        {
            read(fp, &support, sizeof(support));
            close(fp);
        }
    }
     if ((tmp = strstr(support, "\n")))
     {
         *tmp = '\0';
     }
    return support;
}
/* @} */
int getSensorId(const char *sensorname)
{
	int i;

	for(i = 0 ; i < count; i++)
	{
		LOGD("get sensorname = %s, ID = %d, sensor name = %s!!", sensorname, i, list[i].name);
		if(stristr(list[i].name, sensorname))
		{
			LOGD("get sensor success! ID = %d, sensor name = %s, type = %d!!", i, list[i].name, list[i].type);
			return i;
		}
	}
	LOGD("failed to get the sensor ID!!");
	return -1;
}

int getSensorType(int id)
{
    return list[id].type;
}

char* getSensorName(int id)
{
    return (char*)(list[id].name);
}

static int activate_sensors(int id, int delay, int opt)
{
	int err = 0;
	//struct sensors_poll_device_1 * dev = (struct sensors_poll_device_1 *)device;

	LOGD("sensor parameters: dalay = %d; opt = %d! %d IN",delay, opt, __LINE__);
	//checkReturn(mSensors->activate(list[id].handle, 0));
	if (opt == S_ON){
		mSensors->flush(list[id].handle);
		LOGD("flush sensor event! %d IN", __LINE__);
		mSensors->batch(list[id].handle, 1, ms2ns(delay));
		SPRD_DBG("Sets a sensor's parameters! %d IN", __LINE__);
	}
	LOGD("active sensor 0! %d IN", __LINE__);
	checkReturn(mSensors->activate(list[id].handle, opt));
	LOGD("active sensor 1! %d IN", __LINE__);

	return err;
}

int enable_sensor()
{
    int i,err;

    for ( i = 0; i < count; i++) {
        err = activate_sensors(i, 1, S_ON);
        LOGD("activate_sensors(ON) for '%s'", list[i].name);
        if (err != 0) {
            LOGE("activate_sensors(ON) for '%s'failed", list[i].name);
            return 0;
        }
    }

    return err;
}

int sensor_poll(sensors_event_t* buffer, size_t count)
{
    if (mSensors == nullptr) return NO_INIT;

    ssize_t err;
    auto ret = mSensors->poll(
         count,
         [&](auto result,
             const auto &events,
             const auto &dynamicSensorsAdded) {
            if (result == Result::OK) {
                for (size_t i = 0; i < events.size(); ++i) {
                    convertToSensorEvent(events[i], &buffer[i]);
                }
                err = (ssize_t)events.size();
            } else {
                err = -1;
            }
         });

    return err;
}

int sensor_enable(const char *sensorname)
{
	int id = -1;
	int rtn = -1;

	id = getSensorId(sensorname);
	if (id != -1){
		LOGD("activate_sensors(ON) for '%s'", list[id].name);
		rtn = activate_sensors(id, 50, S_ON);
	}
	if (rtn != 0 && id != -1)
	{
		LOGE("activate_sensors(ON) for '%s'failed", list[id].name);
		return -1;
	}

	return id;
}

int sensor_disable(const char *sensorname)
{
	int id = -1;
	int err = -1;

	/*********activate_sensors(OFF)***************/
	id = getSensorId(sensorname);
	if (id != -1){
		err = activate_sensors(id, 0, S_OFF);
	}
	if (err != 0 && id != -1) {
		LOGE("activate_sensors(OFF) for '%s'failed", list[id].name);
		return -1;
	}
#if 0
	/*********close sensor***************/
	err = sensors_close(device);
	if (err != 0) {
		LOGE("mmitest sensors_close() failed");
	}
#endif
	return id;
}

int sensor_stop()
{
    int i,err;

    /*********activate_sensors(OFF)***************/
    for (i = 0; i < count; i++) {
        err = activate_sensors(i, 0, S_OFF);
        if (err != 0) {
            LOGE("activate_sensors(OFF) for '%s'failed", list[i].name);
            return 0;
        }
    }
    /*********close sensor***************/
    /*err = sensors_close(device);
    if (err != 0) {
        LOGE("mmitest sensors_close() failed");
    }*/
    return err;
}

int sensor_load()
{
    // SensorDevice may wait upto 100ms * 10 = 1s for hidl service.
    constexpr auto RETRY_DELAY = std::chrono::milliseconds(100);
    size_t retry = 10;

    while (true) {
        int initStep = 0;
        mSensors = ISensors::getService();
        if (mSensors != nullptr) {
            if(mSensors->poll(0, [](auto, const auto &, const auto &) {}).isOk()) {
                // ok to continue
                break;
            }
            // hidl service is restarting, pointer is invalid.
            mSensors = nullptr;
        }

        if (--retry <= 0) {
            ALOGE("Cannot connect to ISensors hidl service!");
            break;
        }
        // Delay 100ms before retry, hidl service is expected to come up in short time after
        // crash.
        ALOGI("%s unsuccessful, try again soon (remaining retry %zu).",
                (initStep == 0) ? "getService()" : "poll() check", retry);
        std::this_thread::sleep_for(RETRY_DELAY);
    }

    if (mSensors != nullptr) {
        sensor_getlist();
    }
    
    senloaded = (mSensors != nullptr)?0:-1;
    return senloaded;
}

int isSensorLoad()
{
    return senloaded;
}

int SenCaliCmd(const char * cmd)
{
    int fd = -1;
    int ret = -1;
    int bytes_to_write = 0;

    if(cmd == NULL){
        return -1;
    }

    bytes_to_write = strlen(cmd);
    if(fd < 0) {
        fd = open(SENCALI, O_WRONLY | O_NONBLOCK);
    }

    if(fd < 0) {
        return -1;
    }else{
        ret = write(fd, cmd, bytes_to_write);
        if( ret != bytes_to_write) {
			LOGD("cali_write_cmd ret is %d, byte is %d", ret, bytes_to_write);
			return -1;
        }
    }

    if(fd >= 0) {
        close(fd);
    }

    return 0;
}
