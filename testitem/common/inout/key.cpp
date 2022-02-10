#include "common.h"

int test_key_start_common(void)
{
    int ret;
    int menu_count=0;
    int key = -1;
    int i = 0;
    int cur_row = 2;
    int count = 0;

    LOGD("mmitest start");
    ui_fill_locked();
    ui_show_title(MENU_TEST_KEY);

    ui_set_color(CL_GREEN);
    cur_row=ui_show_text(cur_row, 0, TEXT_KEY_ILLUSTRATE);
    
    for(i = 0; i < getKeyCnt(); i++) {
        getKeyInfo()[i].done = 0;
    }

    for(;;) {
        cur_row = 4;
        for(i = 0; i < getKeyCnt(); i++) {
            if(getKeyInfo()[i].done) {
                ui_set_color(CL_GREEN);
            } else {
                ui_set_color(CL_RED);
            }
            cur_row = ui_show_text(cur_row, 0, getKeyInfo()[i].key_shown);
        }

        gr_flip();
        if((count >= getKeyCnt())) break;
        if(key==ETIMEDOUT) break;
        key = ui_wait_key();
        LOGD("mmitest key = %d",key);
        for(i = 0; i < getKeyCnt(); i++) {
            if((getKeyInfo()[i].key == key)
                    &&(getKeyInfo()[i].done == 0))  {
                getKeyInfo()[i].done = 1;
                count++;
            }
        }

        LOGD("mmitest count=%d",count);
    
    }

    LOGD("mmitest key over");

    if(key==ETIMEDOUT){
        ui_set_color(CL_RED);
        ui_show_text(cur_row+2, 0, TEXT_TEST_FAIL);
        gr_flip();
        sleep(1);
        ret=RL_FAIL;
    }else{
        ui_set_color(CL_GREEN);
        ui_show_text(cur_row+2, 0, TEXT_TEST_PASS);
        gr_flip();
        sleep(1);
        ret=RL_PASS;
    }
    save_result(CASE_TEST_KEY,ret);

    usleep(500 * 1000);
    return ret;
}
