#include "common.h"
#include "sensorcomm.h"

static int thread_run=0;
static int cur_row = 2;
static int x_row;
static time_t begin_time,over_time;
static int sensor_id = -1;
static sensors_event_t data;
static int msencali_result=0;
extern pthread_mutex_t freetype_lock;

static void thread_exit_handler(int sig) {
  LOGD("receive signal %d , eng_receive_data_thread exit\n", sig);
  pthread_mutex_unlock(&freetype_lock);
  pthread_exit(0);
}

static void *msensorcali_thread(void *)
{
	int i,n;
	int type_num = 0;
	static const size_t numEvents = 16;
	sensors_event_t buffer[numEvents];
	char asinfo[64];

	struct sigaction actions;
	sigemptyset(&actions.sa_mask);
	actions.sa_flags = 0;
	actions.sa_handler = thread_exit_handler;
	sigaction(SIGUSR1, &actions, NULL);

	begin_time=time(NULL);

	type_num = getSensorType(sensor_id);
	LOGD("activate sensor: %s success!!! type_num = %d.", getSensorName(sensor_id), type_num);

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
//#if 0
//			if (data.version != sizeof(sensors_event_t)) {
				LOGD("mmitestsensor incorrect event version (version=%d, expected=%d)!!",data.version, sizeof(sensors_event_t));
//				break;
//			}
//#endif
			if (type_num == data.type){
				LOGD("mmitest value=<%5.1f,%5.1f,%5.1f>\n",data.data[0], data.data[1], data.data[2]);
				ui_clear_rows(x_row, 3);
				ui_set_color(CL_GREEN);
				memset(asinfo,0,strlen(asinfo));
				snprintf(asinfo, sizeof(asinfo), "%s %5.1f", TEXT_MS_X, data.data[0]);
				cur_row = ui_show_text(x_row, 0, asinfo);
				memset(asinfo,0,strlen(asinfo));
				snprintf(asinfo, sizeof(asinfo), "%s %5.1f", TEXT_MS_Y, data.data[1]);
				cur_row = ui_show_text(cur_row, 0, asinfo);
				memset(asinfo,0,strlen(asinfo));
				snprintf(asinfo, sizeof(asinfo), "%s %5.1f", TEXT_MS_Z, data.data[2]);
				cur_row = ui_show_text(cur_row, 0, asinfo);
				gr_flip();
				usleep(2*1000);
			}
		}

	} while (thread_run);

    return NULL;
}

int cali_msensor_start_common(void)
{
	pthread_t thread = -1;
	const char *ptr = getSensroTypeName(SENSOR_MAGNETIC);
	int ret;
	int err;
	int cali_fd;
	char write_buf[1024] = {0};
	char calibuf[128] = {0};

	ui_fill_locked();
	ui_show_title(MENU_CALI_MAGSOR);
	cur_row = 2;
	ui_set_color(CL_WHITE);
	cur_row = ui_show_text(cur_row, 0, TEXT_SENSOR_DEV_INFO);
//	cur_row = ui_show_text(cur_row, 0, BOARD_HAVE_ACC);
	cur_row = ui_show_text(cur_row, 0, ptr);

	if(isSensorLoad() < 0){
		cur_row = ui_show_text(cur_row, 0, TEXT_SENSOR_OPEN);
		gr_flip();
		sensor_load();
		ui_clear_rows((cur_row-1), 2);
		ui_set_color(CL_WHITE);
	}else{
		cur_row++;
	}

	if(isSensorLoad() < 0){
		ui_set_color(CL_RED);
		ui_show_text(cur_row, 0, TEXT_SENSOR_OPEN_FAIL);
		gr_flip();
		sleep(1);
		goto end;
	}
	ui_set_color(CL_GREEN);
	cur_row = ui_show_text(cur_row, 0, MAG_CALI_OPER1);
	x_row = cur_row+1;
	gr_flip();
	snprintf(write_buf, sizeof(write_buf) - 1,"%d %d 1", CALIB_EN, SENSOR_TYPE_MAGNETIC_FIELD);
	ret = SenCaliCmd((const char *)write_buf);
	LOGD("sensor cali cmd write:%s, ret = %d", write_buf, ret);

	//enable gsensor
	sensor_id = sensor_enable(ptr);
	LOGD("test Asensor ID is %d~", sensor_id);
	if(sensor_id < 0){
		ui_show_text(cur_row, 0, TEXT_SENSOR_OPEN_FAIL);
		goto end;
	}

	thread_run=1;
	pthread_create(&thread, NULL, msensorcali_thread, NULL);
	sleep(4);
	snprintf(write_buf, sizeof(write_buf) - 1,"%d %d 1", CALIB_CHECK_STATUS, SENSOR_TYPE_MAGNETIC_FIELD);
	ret = SenCaliCmd((const char *)write_buf);
	LOGD("sensor cali cmd write:%s, ret = %d", write_buf, ret);
	cali_fd = open(SENDATA,O_RDWR);
	if(cali_fd < 0){
		LOGE("open sensor cali data: %s faild",SENDATA);
		msencali_result = RL_FAIL;
		goto end;
	}else{
		memset(calibuf, 0, sizeof(calibuf));
		ret = read(cali_fd, calibuf, sizeof(calibuf));
		if (ret <= 0) {
			LOGE("mmitest [fp:%d] read calibrator_data length error", cali_fd);
			msencali_result = RL_FAIL;
			goto end;
		}
		ret = atoi(calibuf);
		LOGD("calibrator_data = %s; %d", calibuf, ret);
	}

	if (2 == ret){
		snprintf(write_buf, sizeof(write_buf) - 1,"%d %d 1", CALIB_DATA_READ, SENSOR_TYPE_MAGNETIC_FIELD);
		ret = SenCaliCmd((const char *)write_buf);
		LOGD("sensor cali cmd write:%s, ret = %d", write_buf, ret);
		cali_fd = open(SENDATA,O_RDWR);
		if(cali_fd < 0){
			LOGE("open sensor cali data: %s faild",SENDATA);
			msencali_result = RL_FAIL;
			goto end;
		}else{
			memset(calibuf, 0, sizeof(calibuf));
			ret = read(cali_fd, calibuf, sizeof(calibuf));
			if (ret <= 0) {
				LOGE("mmitest [fp:%d] read calibrator_data length error", cali_fd);
				msencali_result = RL_FAIL;
				goto end;
			}
			ret = atoi(calibuf);
			LOGD("calibrator_data = %s; %d", calibuf, ret);
		}
	}else{
		ui_set_color(CL_RED);
		cur_row = ui_show_text(cur_row, 0, MAG_CALI_OPER2);
		msencali_result = RL_FAIL;
		goto end;
	}

	if (!ret){
		msencali_result = RL_PASS;
	}else{
		msencali_result = RL_FAIL;
	}
end:

	if (thread != -1){
		thread_run = 0;
		//pthread_kill(thread,SIGUSR1);
		pthread_join(thread,NULL);
	}

	if(RL_PASS == msencali_result){
		ui_set_color(CL_GREEN);
		ui_show_text(cur_row+1, 0, TEXT_CALI_PASS);
	}
	else if(RL_FAIL == msencali_result){
		ui_set_color(CL_RED);
		ui_show_text(cur_row+1, 0, TEXT_CALI_FAIL);
	}else{
		ui_set_color(CL_WHITE);
		ui_show_text(cur_row+1, 0, TEXT_CALI_NA);
	}
	gr_flip();
	sleep(1);
	sensor_disable(ptr);
	gr_flip();

	save_result(CASE_CALI_MAGSOR, msencali_result);
	over_time=time(NULL);
	LOGD("mmitest casetime gsensor is %ld s",(over_time-begin_time));
	return msencali_result;
}
