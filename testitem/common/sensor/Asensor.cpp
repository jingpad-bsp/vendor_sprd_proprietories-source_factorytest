#include "common.h"
#include "sensorcomm.h"

static int cur_row = 2;
static int x_row, y_row, z_row;
static time_t begin_time,over_time;
static int sensor_id;
static sensors_event_t data;
static int asensor_result=0;
static int x_pass, y_pass, z_pass;

static int asensor_check(float datas)
{
	int ret = -1;

	float start_1 = SPRD_ASENSOR_1G-SPRD_ASENSOR_OFFSET;
	float start_2 = -SPRD_ASENSOR_1G+SPRD_ASENSOR_OFFSET;

	if( ((start_1<datas) || (start_2>datas)) ){
		ret = 0;
	}

	return ret;
}

static void *asensor_thread(void *)
{
	int i,n;
	int col = 6;
	time_t start_time,now_time;
	int type_num = 0;
	static const size_t numEvents = 16;
	sensors_event_t buffer[numEvents];

	x_pass = y_pass = z_pass = 0;
	begin_time=time(NULL);

	type_num = getSensorType(sensor_id);
	LOGD("activate sensor: %s success!!! type_num = %d.", getSensorName(sensor_id), type_num);

	start_time=time(NULL);
	do {
		now_time=time(NULL);
		ui_set_color(CL_GREEN);
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
//#if 0
//			if (data.version != sizeof(sensors_event_t)) {
				LOGD("mmitestsensor incorrect event version (version=%d, expected=%d)!!",data.version, sizeof(sensors_event_t));
//				break;
//			}
//#endif
			if (type_num == data.type){
				LOGD("mmitest value=<%5.1f,%5.1f,%5.1f>\n",data.data[0], data.data[1], data.data[2]);
				if(x_pass == 0 && asensor_check(data.data[0]) == 0) {
					x_pass = 1;
					LOGD("x_pass");
					ui_show_text(x_row, col, TEXT_SENSOR_PASS);
					gr_flip();
				}

				if(y_pass == 0 && asensor_check(data.data[1]) == 0) {
					y_pass = 1;
					LOGD("y_pass");
					ui_show_text(y_row, col, TEXT_SENSOR_PASS);
					gr_flip();
				}

				if(z_pass == 0 && asensor_check(data.data[2]) == 0) {
					z_pass = 1;
					LOGD("z_pass");
					ui_show_text(z_row, col, TEXT_SENSOR_PASS);
					gr_flip();
				}
				usleep(2*1000);
			}
		}

		if((now_time-start_time)>=ASENSOR_TIMEOUT) break;

	} while (!(x_pass&y_pass&z_pass));

    if(x_pass&y_pass&z_pass)
	asensor_result = RL_PASS;
    else
	asensor_result = RL_FAIL;
    return NULL;
}

int test_asensor_start_common(void)
{
	pthread_t thread;
	const char *ptr = getSensroTypeName(SENSOR_ACC);
	int ret;
	int err;

	ui_fill_locked();
	ui_show_title(MENU_TEST_ASENSOR);
	cur_row = 2;
	ui_set_color(CL_WHITE);
	cur_row = ui_show_text(cur_row, 0, TEXT_SENSOR_DEV_INFO);
//	cur_row = ui_show_text(cur_row, 0, BOARD_HAVE_ACC);
//	cur_row = ui_show_text(cur_row, 0, SENSOR_HUB_ACCELEROMETER);
	if(isSensorLoad() < 0){
		cur_row = ui_show_text(cur_row, 0, TEXT_SENSOR_OPEN);
		gr_flip();
		sensor_load();
		ui_clear_rows((cur_row-1), 2);
		ui_set_color(CL_WHITE);
	}else{
		cur_row++;
	}

	LOGD("cur_row = %d", cur_row);
	cur_row = ui_show_text(cur_row, 0, TEXT_AS_OPER1);
	cur_row = ui_show_text(cur_row, 0, TEXT_AS_OPER2);
	ui_set_color(CL_GREEN);
	x_row = cur_row;
	cur_row = ui_show_text(cur_row, 0, TEXT_GS_X);
	y_row = cur_row;
	cur_row = ui_show_text(cur_row, 0, TEXT_GS_Y);
	z_row = cur_row;
	cur_row = ui_show_text(cur_row, 0, TEXT_GS_Z);
	gr_flip();

	if(isSensorLoad() < 0){
		ui_set_color(CL_RED);
		ui_show_text(cur_row, 0, TEXT_SENSOR_OPEN_FAIL);
		gr_flip();
		sleep(1);
		goto end;
	}else{
		//enable asensor
		sensor_id = sensor_enable(ptr);
		LOGD("test msensor ID is %d~", sensor_id);
		if(sensor_id < 0){
			ui_show_text(cur_row, 0, TEXT_SENSOR_OPEN_FAIL);
			goto end;
		}
		pthread_create(&thread, NULL, asensor_thread, NULL);
		pthread_join(thread, NULL); /* wait "handle key" thread exit. */

		if(RL_PASS == asensor_result){
			ui_set_color(CL_GREEN);
			ui_show_text(cur_row, 0, TEXT_TEST_PASS);
		}
		else if(RL_FAIL == asensor_result){
			ui_set_color(CL_RED);
			ui_show_text(cur_row, 0, TEXT_TEST_FAIL);
		}else{
			ui_set_color(CL_WHITE);
			ui_show_text(cur_row, 0, TEXT_TEST_NA);
		}
		gr_flip();

		sensor_disable(ptr);
	}

end:
	save_result(CASE_TEST_ACCSOR,asensor_result);
	over_time=time(NULL);
	LOGD("mmitest casetime asensor is %ld s",(over_time-begin_time));

	usleep(500 * 1000);
	return asensor_result;
}
