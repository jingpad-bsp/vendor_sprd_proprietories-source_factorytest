#include "common.h"
#include "modem.h"
#include "eng_tok.h"

extern pthread_mutex_t tel_mutex;


bool isLteOnly(void){
    char testmode[PROPERTY_VALUE_MAX+1];
    property_get("persist.vendor.radio.ssda.testmode", testmode, "0");
    LOGD("isLteOnly: persist.vendor.radio.ssda.testmode=%s\n",testmode);

    if (!strcmp(testmode, "3")) { // if persist.vendor.radio.ssda.testmode=3, then modem is lte only
        return true;
    }else {
        return false;
    }
}

int test_tel_start_common(void)
{
	int cur_row = 2;
	int ret, ps_state;
	char tmp[512];
	char* ptmp = NULL;
	time_t start_time, now_time;
	char property[PROPERTY_VALUE_MAX];
	char moemd_tel_port[BUF_LEN];
	char write_buf[1024] = {0};

	ui_fill_locked();
	ui_show_title(MENU_TEST_TEL);
	ui_set_color(CL_GREEN);
	if (isLteOnly()) {
		cur_row = ui_show_text(cur_row, 0, TEL_TEST_START_LTE);
	}else{
		cur_row = ui_show_text(cur_row, 0, TEL_TEST_START);
	}
	cur_row = ui_show_text(cur_row, 0, TEL_TEST_TIPS);
	gr_flip();
/*
	property_get(PROP_MODEM_LTE_ENABLE, property, "not_find");
	if(!strcmp(property, "1"))
		snprintf(moemd_tel_port, sizeof(moemd_tel_port), "/dev/stty_lte2");
	else
		snprintf(moemd_tel_port, sizeof(moemd_tel_port), "/dev/stty_w2");

	LOGD("mmitest tel test %s", moemd_tel_port);
	fd = open(moemd_tel_port, O_RDWR);
	if(fd < 0)
	{
		LOGE("mmitest tel test failed");
		ret = RL_FAIL;
		goto end;
	}
*/
	pthread_mutex_lock(&tel_mutex);
	telSendAt(0, "AT+SFUN=2", NULL, 0, 0);    //open sim card
	telSendAt(0, "AT+SFUN=5", NULL, 0, 0);     //close protocol
	usleep(3000*1000);
	if (isLteOnly()){
		telSendAt(0, "AT+SPTESTMODEM=21,254", NULL, 0, 0);
	}else {
		telSendAt(0, "AT+SPTESTMODEM=15,10", NULL, 0, 0);
	}
	ret = telSendAt(0, "AT+SFUN=4", NULL, 0, 100); //open protocol stack and wait 100s,if exceed more than 20s,we regard rregistering network fail

	pthread_mutex_unlock(&tel_mutex);
	if(ret < 0 )
	{
		ret = RL_FAIL;
		ui_set_color(CL_RED);
		ui_show_text(cur_row, 0, TEL_DIAL_FAIL);
		gr_flip();
		sleep(1);
		goto end;
	}
	start_time = time(NULL);
	for(;;)
	{
		telSendAt(0, "AT+CREG?", tmp, sizeof(tmp), 0);
		ptmp = strstr(tmp, "CREG");
		LOGD("+CREG =%s", ptmp);
		eng_tok_start(&ptmp);
		eng_tok_nextint(&ptmp, &ps_state);
		LOGD("get ps mode=%d", ps_state);
		if(2 == ps_state)
		{
			eng_tok_nextint(&ptmp, &ps_state);
			LOGD("get ps state=%d", ps_state);
			if((1 == ps_state) || (8 == ps_state) || (5 == ps_state))
			{
				break;
			}
		}
		sleep(2);
		now_time = time(NULL);
		if (now_time - start_time > TEL_TIMEOUT )
		{
			LOGE("mmitest tel test failed");
			ret = RL_FAIL;
			ui_set_color(CL_RED);
			ui_show_text(cur_row, 0, TEL_DIAL_FAIL);
			gr_flip();
			sleep(1);
			goto end;
		}
	}

	if (!isLteOnly()){
		ret=telSendAt(0, "ATD112@1,#;", NULL,0, 0);  //call 112
	}else{
		ret=telSendAt(0, "ATD10086;", NULL,0, 0);  //call 10086
	}

	LOGD("tel send at return: %d", ret);

	cur_row = ui_show_text(cur_row, 0, TEL_DIAL_OVER);
	usleep(200 * 1000);

#ifdef AUDIO_DRIVER_2
	snprintf(write_buf, sizeof(write_buf) - 1, "set_mode=%d;test_out_stream_route=%d;", AUDIO_MODE_IN_CALL, AUDIO_DEVICE_OUT_SPEAKER);    //open speaker
	LOGD("write:%s", write_buf);
	SendAudioTestCmd((const uchar *)write_buf, strlen(write_buf));
#else
	telSendAt(0, "AT+SSAM=1", NULL, 0, 0);  //open speaker
#endif

	gr_flip();

	ret = ui_handle_button(TEXT_PASS, NULL, TEXT_FAIL);
	telSendAt(0, "ATH", NULL, 0, 0);             //hang up
	telSendAt(0, "AT", NULL, 0, 0);

#ifdef AUDIO_DRIVER_2
	snprintf(write_buf, sizeof(write_buf) - 1, "set_mode=%d;", AUDIO_MODE_NORMAL);
	LOGD("write:%s", write_buf);
	SendAudioTestCmd((const uchar *)write_buf, strlen(write_buf));
#endif

end:
/*
	if(fd >= 0)
		close(fd);
*/
	save_result(CASE_TEST_TEL, ret);
	return ret;
}
