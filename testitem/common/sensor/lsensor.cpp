#include "common.h"
#include "sensorcomm.h"

static int thread_run;
static float proximity_value=1;
static int proximity_modifies=0;
static int light_value=0;
static int light_pass=0;
static int cur_row = 2;
static int sensor_id, sensor_id1;
static sensors_event_t data;

static time_t begin_time,over_time;

int type_num;

#define S_ON	1
#define S_OFF	0

static void lsensor_show()
{
	char buf[64];
	int row = cur_row;

	ui_clear_rows(row, 2);

	if(proximity_modifies >= 2) {
		ui_set_color(CL_GREEN);
	} else {
		ui_set_color(CL_RED);
	}

	if(proximity_value == 0){
		row = ui_show_text(row, 0, TEXT_PS_NEAR);
	} else {
		row = ui_show_text(row, 0, TEXT_PS_FAR);
	}

	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), "%s %d", TEXT_LS_LUX, light_value);

	if(light_pass == 1) {
		ui_set_color(CL_GREEN);
	} else {
		ui_set_color(CL_RED);
	}
	ui_show_text(row, 0, buf);
	gr_flip();
}

static void *lsensor_thread(void *)
{
	int cnt=0;
	int i,n;
	int type_num, type_num1;
	static const size_t numEvents = 16;
	int flush_value = 0;
	sensors_event_t buffer[numEvents];

	type_num = getSensorType(sensor_id);
	type_num1 = getSensorType(sensor_id1);
	LOGD("activate sensor success!!!: sensor name = %s, type_num = %d; sensor name = %s, type_num1 = %d" \
		, getSensorName(sensor_id), type_num, getSensorName(sensor_id1), type_num1);

	do {
		LOGD("mmitest here while\n!");
		//n = device->poll(device, buffer, numEvents);
		n = sensor_poll(buffer, numEvents);
		LOGD("mmitest here afterpoll\n n = %d",n);
		if (n < 0) {
			LOGD("mmitest poll() failed\n");
			break;
		}
		for (i = 0; i < n; i++) {
			data = buffer[i];

//			if (data.version != sizeof(sensors_event_t)) {
				LOGD("mmitestsensor incorrect event version (version=%d, expected=%d, data.type = %d)!!",data.version, sizeof(sensors_event_t), data.type);
//				break;
//			}

			switch(data.type){
				case SENSOR_TYPE_META_DATA:
					LOGD("data.meta_data.what = %d ,  flush_value = %d, ! %d IN", data.meta_data.what, flush_value, __LINE__);
					if(data.meta_data.what == META_DATA_FLUSH_COMPLETE)
						flush_value++;
					break;
				case SENSOR_TYPE_LIGHT:
					LOGD("mmitest light value=<%5.1f>, flush_value = %d\n",data.light, flush_value);
					if(flush_value >= 2 && light_value != data.light){
						cnt++;
						if(cnt>=5)
							light_pass = 1;
						light_value = data.light;
						lsensor_show();
					}
					break;
				case SENSOR_TYPE_PROXIMITY:
					LOGD("mmitest Proximity:%5.1f, flush_value = %d", data.distance, flush_value);
					if(flush_value >= 2 && proximity_value != data.distance){
						proximity_modifies++;
						proximity_value = data.distance;
						lsensor_show();
					}
					break;
				default:
					LOGD("ERROR DATA.TYPE! %d IN", __LINE__);
					break;
			}
		}

		if(light_pass == 1 && proximity_modifies > 1){
			ui_push_result(RL_PASS);
			ui_set_color(CL_GREEN);
			ui_show_text(cur_row+2, 0, TEXT_TEST_PASS);
			gr_flip();
			sleep(1);
			break;
		}
	} while (1 == thread_run);

	return NULL;
}

int test_lsensor_start_common(void)
{
	int ret = 0;
	const char *ptr = getSensroTypeName(SENSOR_PXY);
	const char *ptr1 = getSensroTypeName(SENSOR_LUX);
	pthread_t thread;
	proximity_value=1;
	proximity_modifies=0;
	light_value=0;
	light_pass=0;
	cur_row = 2;

	ui_fill_locked();
	ui_show_title(MENU_TEST_LSENSOR);
	ui_set_color(CL_WHITE);

	begin_time=time(NULL);
	cur_row = ui_show_text(cur_row, 0, TEXT_SENSOR_DEV_INFO);
	if(isSensorLoad() < 0){
		cur_row = ui_show_text(cur_row, 0, TEXT_SENSOR_OPEN);
		gr_flip();
		sensor_load();
		ui_clear_rows((cur_row-1), 2);
		ui_set_color(CL_WHITE);
	}else{
		cur_row++;
	}

	cur_row = ui_show_text(cur_row, 0, TEXT_ACC_OPER);
	lsensor_show();

	if(isSensorLoad() < 0){
		ui_set_color(CL_RED);
		ui_show_text(cur_row, 0, TEXT_SENSOR_OPEN_FAIL);
		gr_flip();
		sleep(1);
		goto end;
	}else{
		//enable lsensor
		sensor_id = sensor_enable(ptr);
		sensor_id1 = sensor_enable(ptr1);
		LOGD("test lsensor ID is %d && %d~", sensor_id, sensor_id1);
		if(sensor_id < 0 || sensor_id1 < 0){
			ui_show_text(cur_row, 0, TEXT_SENSOR_OPEN_FAIL);
			goto end;
		}
		thread_run = 1;
		pthread_create(&thread, NULL, lsensor_thread, NULL);
		ret = ui_handle_button(NULL,NULL,NULL);//, TEXT_GOBACK
		thread_run = 0;
		pthread_join(thread, NULL); /* wait "handle key" thread exit. */

		sensor_disable(ptr);
		sensor_disable(ptr1);
	}

end:
	save_result(CASE_TEST_LPSOR,ret);
	over_time=time(NULL);
	LOGD("mmitest casetime lsensor is %ld s",(over_time-begin_time));

	usleep(500 * 1000);
	return ret;
}
