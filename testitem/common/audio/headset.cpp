#include "common.h"
#include "chnmgr.h"
#include "../sensor/sensorcomm.h"
#include "audiocomm.h"

static int headset_status = -1;
static int headset_mic_status = -1;
static int headset_status_event = -1;
static int headset_key = -1;
static int headset_key_event = -1;
static int thread_run;

int at_cmd_audio_loop(int, int, int, int, int, int);
int at_cmd_change_outindevice(int, int);


/*
 * Read key code, check if pressed
 */
static int handle_headset_input(struct input_event *event)
{
	int ret = -1;
	LOGD("type=%d; value=%d; code=%d\n",event->type, event->value, event->code);

	switch (event->type) {
	case EV_SW:
		headset_status_event = 1;
		switch (event->code) {
		case SPRD_HEADSET_INSERT:
			headset_status = event->value;
			ret = 1;
			break;
		case SPRD_HEADPHONE_MIC:
			headset_mic_status = event->value;
			ret = 1;
			break;
		}
		break;
	case EV_KEY:
		headset_key_event = 1;
		switch (event->code) {
		case SPRD_HEADSET_KEY:
			headset_key = event->value;
			ret = 1;
			break;
		case KEY_VOLUMEDOWN:
			headset_key = event->value;
			ret = 1;
			break;
		case KEY_VOLUMEUP:
			headset_key = event->value;
			ret = 1;
			break;
		case SPRD_HEADSET_KEYLONGPRESS:
			headset_key = event->value;
			ret = 1;
			break;
		}
		break;
	default :
		ret = SPRD_HEADSET_SYNC;
		break;
	}
	return ret;
}

/*
 * Read key code, check if pressed
 */
static int headset_keypressed(struct input_event *event)
{
	int ret = -1;
	LOGD("type=%d; value=%d; code=%d\n",event->type, event->value, event->code);

	if(event->type == EV_KEY) {
		if((event->code == SPRD_HEADSET_KEY)||(event->code==SPRD_HEADSET_KEYLONGPRESS)) {
			ret = event->value;
		}
	}

	LOGD("ret=%d", ret);
	return ret;
}

/*
 * Read headset switch node
 */

 // read node to check headsetin state
 void check_headset_by_node(){
	int fd, ret, input_fd=-1;
	int tmp = 0;
	fd_set rfds;
	char buffer[8];
	char str[128];
	struct timeval timeout;
	struct input_event event;
	int headset_key_count=0;
	int first_row = 2;
	int last_row = 3;
	int key_row = 3;
	headset_key = -1;
	headset_status = -1;

	memset(&timeout, 0, sizeof(timeout));
	timeout.tv_sec = 0;
	timeout.tv_usec = 500*1000;

	fd = open(SPRD_HEADSET_SWITCH_DEV, O_RDONLY);
	if(fd < 0) {
		LOGE("open %s fail", SPRD_HEADSET_SWITCH_DEV);
		goto err;
	}

	input_fd = get_sensor_name(SPRD_HEASETKEY_DEV);
	//input_fd = find_input_dev(O_RDONLY, SPRD_HEASETKEY_DEV);
	if(input_fd < 0) {
		LOGD("open %s fail", SPRD_HEASETKEY_DEV);
		goto err;
	}

	while(thread_run==1) {

		//check headset
		memset(buffer, 0, sizeof(buffer));
		lseek(fd, 0, SEEK_SET);
		ret = read(fd, buffer, sizeof(buffer));
		if(ret < 0) {
			LOGE("read fd fail error");
		} else {
			tmp = atoi(buffer);
		}
		LOGD("%d-%d",  tmp, headset_status);
		if(tmp != headset_status) {
			ui_clear_rows(first_row, (last_row - first_row + 1));
			ui_set_color(CL_WHITE);
			headset_status = tmp;
			//LOGD("[%s]: headset_status = %d\n", __func__, headset_status);
			switch(headset_status) {
				case 0:
					setPlugState(1);
					ui_set_color(CL_RED);
					last_row = ui_show_text(first_row, 0, TEXT_HD_UNINSERT);
					ui_show_button(NULL, NULL,TEXT_FAIL);
					gr_flip();
					at_cmd_audio_loop(0,0,0,0,0,0);
					break;
				case 1:
					setPlugState(0);
					last_row = ui_show_text(first_row, 0, TEXT_HD_INSERTED);
					last_row = ui_show_text(last_row, 0, TEXT_HD_HAS_MIC);
					last_row = ui_show_text(last_row, 0, TEXT_HD_MICHD);
					snprintf(str, sizeof(str), "%s%s", TEXT_HD_KEY_STATE, TEXT_HD_KEY_RELEASE);
					key_row = last_row;
					ui_show_text(key_row, 0, str);
					ui_show_button(TEXT_PASS, NULL, TEXT_FAIL);
					gr_flip();
					at_cmd_audio_loop(1,2,8,2,3,0);
					at_cmd_change_outindevice(4,4);
					break;
				case 2:
					setPlugState(0);
					last_row = ui_show_text(first_row, 0, TEXT_HD_INSERTED);
					last_row = ui_show_text(last_row, 0, TEXT_HD_NO_MIC);
					last_row = ui_show_text(last_row, 0, TEXT_HD_MICHD);
					snprintf(str, sizeof(str), "%s%s", TEXT_HD_KEY_STATE, TEXT_HD_KEY_RELEASE);
					key_row = last_row;
					ui_show_text(key_row, 0, str);
					ui_show_button(TEXT_PASS, NULL, TEXT_FAIL);
					gr_flip();
					at_cmd_audio_loop(1,4,8,2,3,0);
					at_cmd_change_outindevice(4,1);
					break;
			}
			gr_flip();
		}

		//check headset key
		if(headset_status > 0) {
			FD_ZERO(&rfds);
			FD_SET(input_fd, &rfds);
			timeout.tv_sec = 0;
			timeout.tv_usec = 500*1000;
			ret = select(input_fd+1, &rfds, NULL, NULL, &timeout);

			if(ret > 0 && FD_ISSET(input_fd, &rfds)) {
				//read input event
				ret = read(input_fd, &event, sizeof(event));
				if (ret == sizeof(event)) {
					//handle key pressed
					tmp = headset_keypressed(&event);
					if(tmp >= 0 && tmp != headset_key) {
						headset_key = tmp;
						switch(headset_key) {
							case 0:
								snprintf(str, sizeof(str), "%s%s", TEXT_HD_KEY_STATE, TEXT_HD_KEY_RELEASE);
								ui_clear_rows(key_row, 1);
								ui_set_color(CL_WHITE);
								ui_show_text(key_row, 0, str);
								headset_key_count++;
								break;
							case 1:
								snprintf(str, sizeof(str), "%s%s", TEXT_HD_KEY_STATE, TEXT_HD_KEY_PRESSED);
								ui_clear_rows(key_row, 1);
								ui_set_color(CL_WHITE);
								ui_show_text(key_row, 0, str);
								headset_key_count++;
								break;
						}
						gr_flip();
					}
				}else {
					LOGD("read event too small %d",ret);
				}
			} else {
				//LOGD("%s: fd is not set", __func__);
			}
		} else {
			if(headset_key < 1) {
				headset_key = -1;
			}
			usleep(200*1000);
		}
		if(headset_key_count>=3)break;

		//show
		//headset_show();
	}

err:
//	at_cmd_audio_loop(0,0,0,0,0,0);
    if(headset_status==2)
        at_cmd_audio_loop(0,4,8,2,3,0);
    else
        at_cmd_audio_loop(0,2,8,2,3,0);
	if(fd >= 0) close(fd);
	if(input_fd >= 0) close(input_fd);
	ui_push_result(RL_PASS);
 }


 //read events to check headsetin stete
 void check_headset_by_event(){
 	int ret, max_fd = -1,fd = -1,fd_key = -1,fd_jack = -1;
	int tmp = -1;
	fd_set rfds;
	char buffer[8];
	char str[128];
	char write_buf[1024] = {0};
	struct timeval timeout;
	struct input_event event;
	int headset_key_count=0;
	int first_row = 2;
	int last_row = 3;
	int key_row = 3;

	memset(&timeout, 0, sizeof(timeout));
	timeout.tv_sec = 0;
	timeout.tv_usec = 500*1000;

	LOGD("mmitest HEK_DEV_NAME = %s; HED_DEV_NAME = %s", getSensroTypeName(SENSOR_HDK), getSensroTypeName(SENSOR_HDE));
	if(strlen(getSensroTypeName(SENSOR_HDK))){
		fd_key = get_sensor_name(getSensroTypeName(SENSOR_HDK));
		LOGD("++++++++++++++++");
		if(fd_key < 0) {
			ui_push_result(RL_FAIL);
			ui_set_color(CL_RED);
			last_row = ui_show_text(first_row, 0, SPRD_HEASETKEY_DEV);
			gr_flip();
			sleep(1);
		}
	}else{
		LOGD("mmitest get headset keyboard event %s failed",getSensroTypeName(SENSOR_HDK));
	}

	if(strlen(getSensroTypeName(SENSOR_HDE))){
		fd_jack = get_sensor_name(getSensroTypeName(SENSOR_HDE));
		LOGD("------------------");
		if(fd_jack < 0) {
			ui_push_result(RL_FAIL);
			ui_set_color(CL_RED);
			last_row = ui_show_text(first_row, 0, SPRD_HEADSET_DEV);
			gr_flip();
			sleep(1);
		}
	}else{
		LOGD("mmitest get headset jack event %s failed",getSensroTypeName(SENSOR_HDE));
	}

	ui_clear_rows(first_row, (last_row - first_row + 1));
	ui_set_color(CL_RED);
	last_row = ui_show_text(first_row, 0, TEXT_HD_REINSERT);
	gr_flip();

	while(thread_run == 1) {
		//check headset_input
		ret = -1;
		FD_ZERO(&rfds);
		FD_SET(fd_jack, &rfds);
		if(fd_key >= 0 )
		    FD_SET(fd_key, &rfds);
		max_fd = fd_jack>fd_key?fd_jack:fd_key;
		ret = select(max_fd+1, &rfds, NULL, NULL, &timeout);
		if (ret > 0) {
		    if (FD_ISSET(fd_jack, &rfds)) {
		        fd = fd_jack;
		    } else if (FD_ISSET(fd_key, &rfds)) {
		        fd = fd_key;
		    }
			//read input event
		    ret = read(fd, &event, sizeof(event));
		    if(ret == sizeof(event)) {
				//handle headset input
				tmp = handle_headset_input(&event);
				LOGD("headset event: tmp= %d, headset_status_event= %d, headset_status= %d, headset_mic_status= %d, headset_key_event= %d, headset_key= %d", tmp, headset_status_event, headset_status, headset_mic_status, headset_key_event, headset_key);
			}
		}

		//check headset
		if(SPRD_HEADSET_SYNC == tmp && 1 == headset_status_event) {
			ui_clear_rows(first_row, (last_row - first_row + 1));
			ui_set_color(CL_WHITE);

			switch(headset_status) {
			case 0:
				setPlugState(1);
				ui_show_button(NULL, NULL,TEXT_FAIL);
				ui_set_color(CL_RED);
				last_row = ui_show_text(first_row, 0, TEXT_HD_UNINSERT);
				gr_flip();
#ifdef AUDIO_DRIVER_2
				memset(write_buf,0,sizeof(write_buf));
				snprintf(write_buf,sizeof(write_buf) - 1,"dsp_loop=0");
				LOGD("write:%s", write_buf);
				SendAudioTestCmd((const uchar *)write_buf,strlen(write_buf));
#else
                         at_cmd_audio_loop(0,0,0,0,0,0);
                          usleep(5*1000);
//                          at_cmd_audio_loop(1,1,8,2,3,0);
//                          at_cmd_change_outindevice(2,1);
#endif
				break;
			case 1:
				if(1 == headset_mic_status){
					setPlugState(0);
					ui_show_button(TEXT_PASS, NULL, TEXT_FAIL);
					ui_set_color(CL_WHITE);
					last_row = ui_show_text(first_row, 0, TEXT_HD_INSERTED);
					last_row = ui_show_text(last_row, 0, TEXT_HD_HAS_MIC);
					last_row = ui_show_text(last_row, 0, TEXT_HD_MICHD);
					snprintf(str, sizeof(str), "%s%s", TEXT_HD_KEY_STATE, TEXT_HD_KEY_RELEASE);
					key_row = last_row;
					ui_show_text(key_row, 0, str);
					gr_flip();
#ifdef AUDIO_DRIVER_2
					memset(write_buf,0,sizeof(write_buf));
					snprintf(write_buf,sizeof(write_buf) - 1,"test_out_stream_route=%d;test_in_stream_route=0x80000010;dsploop_type=1;dsp_loop=1;", AUDIO_DEVICE_OUT_WIRED_HEADSET);
					LOGD("write:%s", write_buf);
					SendAudioTestCmd((const uchar *)write_buf,strlen(write_buf));
#else
                                at_cmd_audio_loop(0,0,0,0,0,0);
                                usleep(5*1000);
                                at_cmd_audio_loop(1,1,8,2,3,0);
                                at_cmd_change_outindevice(4,4);
#endif
					break;
				}else{
					setPlugState(0);
					ui_show_button(TEXT_PASS, NULL, TEXT_FAIL);
					ui_set_color(CL_WHITE);
					last_row = ui_show_text(first_row, 0, TEXT_HD_INSERTED);
					ui_set_color(CL_RED);
					last_row = ui_show_text(last_row, 0, TEXT_HD_NO_MIC);
					ui_set_color(CL_WHITE);
					last_row = ui_show_text(last_row, 0, TEXT_HD_MICHD);
					//snprintf(str, sizeof(str), "%s%s", TEXT_HD_KEY_STATE, TEXT_HD_KEY_RELEASE);
					key_row = last_row;
					//ui_show_text(key_row, 0, str);
					gr_flip();
#ifdef AUDIO_DRIVER_2
					memset(write_buf,0,sizeof(write_buf));
					snprintf(write_buf,sizeof(write_buf) - 1,"test_out_stream_route=%d;test_in_stream_route=0x80000004;dsploop_type=1;dsp_loop=1;", AUDIO_DEVICE_OUT_WIRED_HEADPHONE);
					LOGD("write:%s", write_buf);
					SendAudioTestCmd((const uchar *)write_buf,strlen(write_buf));
#else
                                at_cmd_audio_loop(0,0,0,0,0,0);
                                usleep(5*1000);
                                at_cmd_audio_loop(1,1,8,2,3,0);
                                at_cmd_change_outindevice(4,1);
#endif
					break;
				}
			default:
				last_row = ui_show_text(first_row, 0, TEXT_HD_REINSERT);
				gr_flip();
				break;
			}
		headset_status_event =-1;
		headset_status = -1;
		headset_mic_status= -1;
		}

		//check headset key
		if(SPRD_HEADSET_SYNC == tmp && 1 == headset_key_event) {
			switch(headset_key) {
			case 0:
				snprintf(str, sizeof(str), "%s%s", TEXT_HD_KEY_STATE, TEXT_HD_KEY_RELEASE);
				ui_clear_rows(key_row, 1);
				ui_set_color(CL_WHITE);
				ui_show_text(key_row, 0, str);
				headset_key_count++;
				break;
			case 1:
				snprintf(str, sizeof(str), "%s%s", TEXT_HD_KEY_STATE, TEXT_HD_KEY_PRESSED);
				ui_clear_rows(key_row, 1);
				ui_set_color(CL_WHITE);
				ui_show_text(key_row, 0, str);
				headset_key_count++;
				break;
			}
		gr_flip();
		headset_key_event = -1;
		headset_key = -1;
		}

		if(headset_key_count>=3)break;

	}

#ifdef AUDIO_DRIVER_2
	memset(write_buf,0,sizeof(write_buf));
	snprintf(write_buf,sizeof(write_buf) - 1,"dsp_loop=0;");
	LOGD("write:%s", write_buf);
	SendAudioTestCmd((const uchar *)write_buf,strlen(write_buf));
#else
      at_cmd_audio_loop(0,1,8,2,3,0);
#endif
	close(fd);
	ui_push_result(RL_PASS);
 }

void* headset_check_thread_default(void *)
{
	if(access(SPRD_HEADSET_SWITCH_DEV, F_OK) != -1){
		check_headset_by_node();
	} else {
		check_headset_by_event();
	}

	return NULL;
}

void* headset_check_thread(void *)
{
    int ret = -1;
    int type = 0;
    char buff[64] = {0};
    int row = 2;
    int headset_key_count = 0;
    int key = -1;
    char str[128];

    while(thread_run == 1)
    {
        //step1.check headset insert
        ret = chnl_send(CHNL_AT, TEST_AT_HEADSET_TYPE, strlen(TEST_AT_HEADSET_TYPE), buff, sizeof(buff));
        if (ret == -1)
        {
            LOGE("send AT fail");
            ui_push_result(RL_FAIL);
            return NULL;
        }
        else
        {
            type = atoi((char *)buff);
            switch (type)
            {
            case 0:
                LOGD("headset not insert");
                row = 2;
                ui_clear_rows(row, 6);
                ui_set_color(CL_RED);
                row = ui_show_text(row, 0, TEXT_HD_UNINSERT);
                gr_flip();
                setPlugState(1);
                usleep(10 * 1000);
                continue;
            case 1:
                LOGD("found 4P headset");
                row = 2;
                ui_clear_rows(row, 1);
                ui_set_color(CL_GREEN);
                row = ui_show_text(row, 0, TEXT_HD_INSERTED);
                row = ui_show_text(row, 0, TEXT_HD_HAS_MIC);
                ui_set_color(CL_WHITE);
                row = ui_show_text(row, 0, TEXT_HD_MICHD);
                gr_flip();
                chnl_callback(TEST_AUDIO_CALLBACK_DESC);        //callback function, for send AT
                chnl_send(CHNL_AT, TEST_AT_AUDIO_HEADSET_LOOP_OPEN, strlen(TEST_AT_AUDIO_HEADSET_LOOP_OPEN), buff, sizeof(buff));
                usleep(10 * 1000);
                setPlugState(0);
                break;
            case 2:
                LOGD("found 3P headset");
                row = 2;
                ui_clear_rows(row, 1);
                ui_set_color(CL_GREEN);
                row = ui_show_text(row, 0, TEXT_HD_INSERTED);
                row = ui_show_text(row, 0, TEXT_HD_NO_MIC);
                ui_set_color(CL_WHITE);
                row = ui_show_text(row, 0, TEXT_HD_MICHD);
                gr_flip();
                chnl_callback(TEST_AUDIO_CALLBACK_DESC);        //callback function, for send AT
                chnl_send(CHNL_AT, TEST_AT_AUDIO_HEADSET_LOOP_OPEN, strlen(TEST_AT_AUDIO_HEADSET_LOOP_OPEN), buff, sizeof(buff));
                usleep(10 * 1000);
                setPlugState(0);
                break;
            default:
                LOGD("unknow type:%d", type);
                continue;
            }

            //step2.check headset key
            while(thread_run == 1)
            {
                LOGD("headset key test, count = %d", headset_key_count);
                ui_clear_rows(row, 1);
                snprintf(str, sizeof(str), "%s%d", TEXT_HD_KEY_TIMES, headset_key_count);
                ui_set_color(CL_WHITE);
                ui_show_text(row, 0, str);
                gr_flip();
                if(headset_key_count >= 2)
                {
                    LOGD("headset test pass");
                    ui_set_color(CL_GREEN);
                    ui_show_text(row + 1, 0, TEXT_PASS);
                    gr_flip();
                    ui_push_result(RL_PASS);
                    chnl_send(CHNL_AT, TEST_AT_AUDIO_HEADSET_LOOP_CLOSE, strlen(TEST_AT_AUDIO_HEADSET_LOOP_CLOSE), buff, sizeof(buff));
                    return NULL;
                }
                if(wait_headset_key() == 1)
                    headset_key_count++;
            }
        }
    }

    chnl_send(CHNL_AT, TEST_AT_AUDIO_HEADSET_LOOP_CLOSE, strlen(TEST_AT_AUDIO_HEADSET_LOOP_CLOSE), buff, sizeof(buff));

    return NULL;
}

int test_headset_start_common(void)
{
    int ret = 0;
    pthread_t t1, t2;
    ui_fill_locked();
    ui_show_title(MENU_TEST_HEADSET);
    gr_flip();

    thread_run = 1;

    if(chnl_is_support(TEST_AT_HEADSET_TYPE))
        pthread_create(&t1, NULL, headset_check_thread, NULL);
    else
        pthread_create(&t1, NULL, headset_check_thread_default, NULL);

    setPlugState(1);
    usleep(200*1000);
    ret = ui_handle_button(TEXT_PASS, NULL, TEXT_FAIL);
    thread_run = 0;

    pthread_join(t1, NULL);

    save_result(CASE_TEST_HEADSET, ret);
    setPlugState(0);

    usleep(500 * 1000);
    return ret;
}
