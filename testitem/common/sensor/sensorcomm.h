#ifndef _SENSOR_COMMON_H
#define _SENSOR_COMMON_H

#include <hardware/sensors.h>

#define SENCALI "/sys/class/sprd_sensorhub/sensor_hub/calibrator_cmd"
#define SENDATA "/sys/class/sprd_sensorhub/sensor_hub/calibrator_data"

typedef enum {
    CALIB_EN,
    CALIB_CHECK_STATUS,
    CALIB_DATA_WRITE,
    CALIB_DATA_READ,
    CALIB_FLASH_WRITE,
    CALIB_FLASH_READ,
} CALIBRATOR_CMD;

int sensor_getlist();
int get_sensor_name(const char * name );
char* get_sensor_ts_name();
int getSensorId(const char *sensorname);
int getSensorType(int id);
char* getSensorName(int id);
int enable_sensor();
int sensor_poll(sensors_event_t* buffer, size_t count);
int sensor_enable(const char *sensorname);
int sensor_disable(const char *sensorname);
int sensor_stop();
int sensor_load();
int isSensorLoad();
int SenCaliCmd(const char * cmd);

#endif