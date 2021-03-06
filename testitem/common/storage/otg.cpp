#include "common.h"

static int thread_run;

#define Y_TYPEA    1
#define Y_TYPEC    2
#define Y_USB3     3
#define NO_OTG    -1

int cur_row;
int usb_type;

int is_port0_partner_support(){
    int fp;
    int ret = -1;
    //add for kernel 4.14
    if (access(OTG_FILE_PATH_PARTNER_K414, F_OK) != -1){
        ret = 1;
    }

    LOGD("is_port0_partner_support ret=%d",ret);
    return ret;
}

int is_support_otg(){
    int fp;
    char support[64] = {0};
    int nRead = 0;
    if(access(OTG_FILE_PATH, F_OK) != -1){
        if ((fp = open(OTG_FILE_PATH,O_RDONLY)) >= 0) {
            nRead = read(fp, &support, sizeof(char));
            close(fp);
            if(nRead > 0 && support[0] == '1'){
                LOGD("support type_A OTG");
                usb_type = Y_TYPEA;
                return Y_TYPEA;
            }
        }
    }
    if (access(USB3_OTG_ENABLE_PATH, F_OK) != -1){
        if ((fp = open(USB3_OTG_ENABLE_PATH,O_RDONLY)) >= 0){
            nRead = read(fp, &support, sizeof(support));
            close(fp);
            if (nRead > 0 && NULL != strstr(support, "enable")){
                LOGD("support USB3 OTG");
                usb_type = Y_USB3;
                return Y_USB3;
            }
        }
    }
    if (access(TPYEC_OTG_ENABLE_PATH, F_OK) != -1){
        if ((fp = open(TPYEC_OTG_ENABLE_PATH,O_RDONLY)) >= 0){
            nRead = read(fp, &support, sizeof(support));
            close(fp);
            if (nRead > 0 && NULL != strstr(support, "enable")){
                LOGD("support type_C OTG");
                usb_type = Y_TYPEC;
                return Y_TYPEC;
            }
        }
    }
    //add for kernel 4.14
    if (access(OTG_FILE_PATH_K414, F_OK) != -1){\
        if ((fp = open(OTG_FILE_PATH_K414,O_RDONLY)) >= 0){
            nRead = read(fp, &support, sizeof(support));
            close(fp);
            LOGD("support Y_TYPEC k4.14 OTG22,support=%s,nRead=%d",support,nRead);
            if (nRead > 0 && NULL != strstr(support, "[dual] source sink")){
                LOGD("support Y_TYPEC k4.14 OTG");
                usb_type = Y_TYPEC;
                return Y_TYPEC;
            }
        }
    }

    LOGD("not support OTG");
    return NO_OTG;
}

int check_typeC_otg_status_k414(){
    LOGD("check_typeC_otg_status_k414");
    char otg_status[64] = {0};
    int fd, j = 30;
    char temp[64];
    int nRead = 0;
    while (j){
        cur_row = 3;
        fd = open(OTG_TPYEC_STATUS_PATH_K414, O_RDONLY);
        if (fd < 0){
            LOGE("open OTG_TPYEC_STATUS_PATH_K414 failed");
            if (j-- < 0) {
                return -1;
            }else{
                usleep(50*1000);
                continue;
            }
        }
        nRead = read(fd, &otg_status, sizeof(otg_status));
        close(fd);
        LOGD("otg_status: %s", otg_status);
        if ((nRead > 0 && NULL != strstr(otg_status, "none")) || (is_port0_partner_support() != 1)){
            ui_clear_rows(cur_row, 4);
            ui_set_color(CL_RED);
            if(j > 1){
                snprintf(temp, sizeof(temp), "%s, %ds", OTG_DEVICE_INSERT, j);
                cur_row = ui_show_text(cur_row, 0, OTG_UNSERT);
                cur_row = ui_show_text(cur_row, 0, temp);
                gr_flip();
                sleep(1);
                setPlugState(1);
                ui_show_button(NULL, NULL,TEXT_FAIL);
                j--;
            }else{
                cur_row = ui_show_text(cur_row, 0, TEXT_OTG_UDISK_NO);
                gr_flip();
                sleep(1);
                ui_push_result(RL_FAIL);
                thread_run = 0;
                return 0;
            }
        }
        if(is_port0_partner_support() != 1) {
              LOGD("insert as device,is_port0_partner_support FAIL!");
              continue;
        }
        if (nRead > 0 && NULL != strstr(otg_status, "[device]")){
            LOGD("check_typeC_otg_status_k414 insert as device");
            return 2;
        }
        if (nRead > 0 && NULL != strstr(otg_status, "[host]")){
            LOGD("check_typeC_otg_status_k414 insert as host");
            return 1;
        }
        if (thread_run == 0){
            return -1;
        }
    }
    return 0;
}

int check_typeC_otg_status(){
    //LOGD("Do you poke the otg device into phone");
    char otg_status[10] = {0};
    int fd, j = 30;
    char temp[64];
    int nRead = 0;
    while (j){
        cur_row = 3;
        fd = open(OTG_TPYEC_STATUS_PATH, O_RDONLY);
        if (fd < 0){
            LOGE("open OTG_TPYEC_STATUS_PATH failed");
            if (j-- < 0) {
                return -1;
            }else{
                usleep(50*1000);
                continue;
            }
        }
        nRead = read(fd, &otg_status, sizeof(otg_status));
        close(fd);
        LOGD("otg_status: %s", otg_status);
        if (nRead > 0 && NULL != strstr(otg_status, "none")){
            ui_clear_rows(cur_row, 4);
            ui_set_color(CL_RED);
            if(j > 1){
                snprintf(temp, sizeof(temp), "%s, %ds", OTG_DEVICE_INSERT, j);
                cur_row = ui_show_text(cur_row, 0, OTG_UNSERT);
                cur_row = ui_show_text(cur_row, 0, temp);
                gr_flip();
                sleep(1);
                setPlugState(1);
                ui_show_button(NULL, NULL,TEXT_FAIL);
                j--;
            }else{
                cur_row = ui_show_text(cur_row, 0, TEXT_OTG_UDISK_NO);
                gr_flip();
                sleep(1);
                ui_push_result(RL_FAIL);
                thread_run = 0;
                return 0;
            }
        }
        if (nRead > 0 && NULL != strstr(otg_status, "device")){
            LOGD("insert as device");
            return 2;
        }
        if (nRead > 0 && NULL != strstr(otg_status, "host")){
            LOGD("insert as host");
            return 1;
        }
        if (thread_run == 0){
            return -1;
        }
    }
    return 0;
}

int check_typeA_otg_status(){
    char otg_status[10] = {0};
    char sta_insert[4] = {0};
    char sta_device[12]={0};
    char temp[64];
    int fd, j = 30;
    int nRead = 0;
    while (j){
        cur_row = 3;
        fd = open(OTG_INSERT_STATUS, O_RDONLY);
        if (fd < 0) {
            LOGE("open OTG_INSERT_STATUS failed");
            if (j-- < 0) {
                return -1;
            }else{
                usleep(50*1000);
                continue;
            }
        }
        nRead = read(fd, &sta_insert, sizeof(sta_insert));
        close(fd);
        if(nRead > 0 && NULL != strstr(sta_insert, "low")){
            fd = open(OTG_DEVICE_HOST, O_RDONLY);
            if (fd < 0) {
                LOGD("open OTG_DEVICE_HOST failed");
                return -1;
            }
            nRead = read(fd,&sta_device,sizeof(sta_device));
            close(fd);
            if(nRead > 0 && NULL != strstr(sta_device, "Mode = 0x1")){
                LOGD("mmitest %s  Mode = 0x1",__func__ );
                return 1;
            }else if(nRead > 0 && NULL != strstr(sta_device, "Mode = 0x0")){
                LOGD("insert as host");
                return 2;
            }
        } else if (nRead > 0 && NULL!=strstr(sta_insert, "high")){
            ui_clear_rows(cur_row, 4);
            ui_set_color(CL_RED);
            if(j > 1){
                snprintf(temp, sizeof(temp), "%s, %ds", OTG_DEVICE_INSERT, j);
                cur_row = ui_show_text(cur_row, 0, OTG_UNSERT);
                cur_row = ui_show_text(cur_row, 0, temp);
                gr_flip();
                sleep(1);
                setPlugState(1);
                ui_show_button(NULL, NULL,TEXT_FAIL);
                j--;
            }else{
                cur_row = ui_show_text(cur_row, 0, TEXT_OTG_UDISK_NO);
                gr_flip();
                sleep(1);
                ui_push_result(RL_FAIL);
                thread_run = 0;
                return 0;
            }
        }
        if (thread_run == 0){
            return -1;
        }
    }
    return 0;
}

int check_usb3_otg_status(){
    char otg_status[10] = {0};
    char sta_insert[4] = {0};
    char sta_device[12]={0};
    char temp[64];
    int fd, j = 30;
    int nRead = 0;
    while (j){
        cur_row = 3;
        fd = open(OTG_USB3_STATUS_PATH, O_RDONLY);
        if (fd < 0) {
            LOGE("open OTG_USB3_STATUS_PATH failed");
            if (j-- < 0) {
                return -1;
            }else{
                usleep(50*1000);
                continue;
            }
        }
        nRead = read(fd, &sta_insert, sizeof(sta_insert));
        close(fd);
        if(nRead > 0 && NULL != strstr(sta_insert, "low")){
            LOGD("insert as host");
            return 1;
        } else if (nRead > 0 && NULL!=strstr(sta_insert, "high")){
            fd = open(USB_STATUS_PATH, O_RDONLY);
            if (fd < 0) {
                return -1;
            }
            nRead = read(fd,&sta_device,sizeof(sta_device));
            close(fd);
            if(nRead > 0 && NULL != strstr(sta_device, "CONFIGURED")){
                LOGD("insert as device");
                return 2;
            }

            ui_clear_rows(cur_row, 4);
            ui_set_color(CL_RED);
            if(j > 1){
                snprintf(temp, sizeof(temp), "%s, %ds", OTG_DEVICE_INSERT, j);
                cur_row = ui_show_text(cur_row, 0, OTG_UNSERT);
                cur_row = ui_show_text(cur_row, 0, temp);
                gr_flip();
                sleep(1);
                setPlugState(1);
                ui_show_button(NULL, NULL,TEXT_FAIL);
                j--;
            }else{
                cur_row = ui_show_text(cur_row, 0, TEXT_OTG_UDISK_NO);
                gr_flip();
                sleep(1);
                ui_push_result(RL_FAIL);
                thread_run = 0;
                return 0;
            }
        }
        if (thread_run == 0){
            return -1;
        }
    }
    return 0;
}

void get_disk_size(){
    int fd, i = 25;
    int read_len;
    unsigned long value=0;
    char buffer[64],temp[64];
    char *endptr;
    int row_temp = cur_row;
    //UNISOC:Bug 1492728 get otg capacity error
    bool is_ufs = false;
    char *otg_path = "/sys/block/sda";
    char *otg_size_path = "/sys/block/sda/size";
    if (0 == access("/dev/block/sda2",F_OK)) {
        is_ufs = true;
        otg_path = "/sys/block/sdd";
        otg_size_path = "/sys/block/sdd/size";
    }
    LOGD("is_ufs =%d, otg_path =%s", is_ufs, otg_path);
    while (i){
        cur_row = row_temp;
        ui_clear_rows(cur_row, 2);
        if ((!is_ufs && (0 == access("/sys/block/sda",F_OK)))
                || (is_ufs && (0 == access("/sys/block/sdd",F_OK)))){
            LOGD("otg is insert");
            break;
        }
        cur_row = row_temp;
        ui_set_color(CL_RED);
        if(i > 1){
            snprintf(temp, sizeof(temp), "%s, %ds", TEXT_OTG_UDISK_SCANING, i);
            ui_show_text(cur_row, 0, temp);
            gr_flip();
            sleep(1);
            i--;
        }else{
            cur_row = ui_show_text(cur_row, 0, TEXT_OTG_UDISK_NO);
            gr_flip();
            ui_push_result(RL_FAIL);
            thread_run = 0;
        }
        setPlugState(1);
        ui_show_button(NULL, NULL,TEXT_FAIL);
        if (thread_run == 0){
            return ;
        }
    }
    ui_set_color(CL_GREEN);
    cur_row = ui_show_text(cur_row, 0, TEXT_OTG_UDISK_OK);
    //gr_flip();
    fd = open(otg_size_path,O_RDONLY);
    if (fd < 0) {
        return ;
    }
    read_len = read(fd,buffer,sizeof(buffer));
    if(read_len > 0)
    value = strtoul(buffer,&endptr,0);
    close(fd);
    LOGD("%s size value = %lu, read_len = %d \n",__FUNCTION__, value, read_len);
    snprintf(temp, sizeof(temp), "%s %ld MB", TEXT_OTG_CAPACITY,(value/2/1024));
    cur_row = ui_show_text(cur_row, 0, temp);
    gr_flip();
    setPlugState(0);
    ui_show_button(TEXT_PASS, NULL, TEXT_FAIL);

}

void get_udisk_size(){
    int fd, i = 10;
    int read_len;
    unsigned long value=0;
    char buffer[64],temp[64];
    char *endptr;
    int row_temp = cur_row;
    while (i){
        cur_row = row_temp;
        ui_clear_rows(cur_row, 2);
        if (0 == access("/sys/block/sda",F_OK)){
            ui_set_color(CL_GREEN);
            cur_row = ui_show_text(cur_row, 0, TEXT_OTG_UDISK_OK);
            //gr_flip();
            fd = open("/sys/block/sda/size",O_RDONLY);
            if (fd < 0) {
                return ;
            }
            read_len = read(fd,buffer,sizeof(buffer));
            if(read_len > 0)
                value = strtoul(buffer,&endptr,0);
            close(fd);
            LOGD("%s /sys/block/sda/size value = %lu, read_len = %d \n",__FUNCTION__, value, read_len);
            snprintf(temp, sizeof(temp), "%s %ld MB", TEXT_OTG_CAPACITY,(value/2/1024));
            cur_row = ui_show_text(cur_row, 0, temp);
            gr_flip();
            setPlugState(0);
            ui_show_button(TEXT_PASS, NULL, TEXT_FAIL);
            return;
        }else{
            if (Y_TYPEA == usb_type)
                if (0 == check_typeA_otg_status())
                    return;
            if (Y_TYPEC == usb_type){
                if (0 == check_typeC_otg_status())
                    return;
                if (check_typeC_otg_status_k414() == 0)
                    return;
            }

            cur_row = row_temp;
            ui_set_color(CL_RED);
            if(i > 1){
                snprintf(temp, sizeof(temp), "%s, %ds", TEXT_OTG_UDISK_SCANING, i);
                ui_show_text(cur_row, 0, temp);
                gr_flip();
                sleep(1);
                i--;
            }else{
                cur_row = ui_show_text(cur_row, 0, TEXT_OTG_UDISK_NO);
                gr_flip();
                ui_push_result(RL_FAIL);
                thread_run = 0;
            }
            setPlugState(1);
            ui_show_button(NULL, NULL,TEXT_FAIL);
        }
        if (thread_run == 0){
            return;
        }
    }
}

void* otg_test_check_thread(void *){
    int flag = 0;
    int i;
    int fp;
    LOGD("otg_test_check_thread....");

    while(thread_run == 1){
        cur_row = 3;
        if(access(USB_FILE_PATH, F_OK) != -1){
            get_disk_size();
        }else{
            LOGD("not support OTG");
            ui_set_color(CL_RED);
            cur_row = ui_show_text(cur_row, 0, OTG_NOT_SUPPORT);
            gr_flip();
            usleep(500*1000);
            setPlugState(1);
            ui_show_button(NULL, NULL,TEXT_FAIL);
            ui_push_result(RL_NA);
        }
    }
    return NULL;
}

void* otg_check_thread(void *){
    int flag = 0;
    int i;

    LOGD("otg_check_thread....");

    while(thread_run == 1){
        cur_row = 3;
        LOGD("is_support_otg: %d", is_support_otg());
        switch (is_support_otg()){
            case NO_OTG:{
                ui_set_color(CL_RED);
                cur_row = ui_show_text(cur_row, 0, OTG_NOT_SUPPORT);
                gr_flip();
                usleep(500*1000);
                setPlugState(1);
                ui_show_button(NULL, NULL,TEXT_FAIL);
                ui_push_result(RL_NA);
                break;
            }
            case Y_TYPEA:{
                flag = check_typeA_otg_status();
                if (flag > 0){
                    cur_row = 3;
                    ui_clear_rows(cur_row, 2);
                    ui_set_color(CL_GREEN);
                    cur_row = ui_show_text(cur_row, 0, OTG_INSERT);
                }
                switch (flag){
                    case 0:{
                        break;
                    }
                    case 1:{
                        cur_row = ui_show_text(cur_row, 0, OTG_HOST);
                        get_udisk_size();
                        break;
                    }
                    case 2:{
                        cur_row = ui_show_text(cur_row, 0, OTG_DEVICE);
                        break;
                    }
                }
                gr_flip();
                break;
            }
            case Y_TYPEC:{
                flag = check_typeC_otg_status();
                if (flag > 0){
                    cur_row = 3;
                    ui_clear_rows(cur_row, 2);
                    ui_set_color(CL_GREEN);
                    cur_row = ui_show_text(cur_row, 0, OTG_INSERT);
                }else{
                    flag = check_typeC_otg_status_k414();
                    if (flag > 0){
                        cur_row = 3;
                        ui_clear_rows(cur_row, 2);
                        ui_set_color(CL_GREEN);
                        cur_row = ui_show_text(cur_row, 0, OTG_INSERT);
                    }
                }
                switch (flag){
                    case 0:{
                        break;
                    }
                    case 1:{
                        cur_row = ui_show_text(cur_row, 0, OTG_HOST);
                        get_udisk_size();
                        break;
                    }
                    case 2:{
                        cur_row = ui_show_text(cur_row, 0, OTG_DEVICE);
                        sleep(1);
                        break;
                    }
                }
                gr_flip();
                break;
            }
            case Y_USB3:{
                flag = check_usb3_otg_status();
                if (flag > 0){
                    cur_row = 3;
                    ui_clear_rows(cur_row, 2);
                    ui_set_color(CL_GREEN);
                    cur_row = ui_show_text(cur_row, 0, OTG_INSERT);
                }
                switch (flag){
                    case 0:{
                        break;
                    }
                    case 1:{
                        cur_row = ui_show_text(cur_row, 0, OTG_HOST);
                        get_udisk_size();
                        break;
                    }
                    case 2:{
                        cur_row = ui_show_text(cur_row, 0, OTG_DEVICE);
                        sleep(1);
                        break;
                    }
                }
                gr_flip();
                break;
            }
        }
    }
    return NULL;
}

int test_otg_start_common(void)
{
    int cur_row=2;
    int ret;
    pthread_t t1;

    ui_fill_locked();
    ui_show_title(MENU_TEST_OTG);
    ui_set_color(CL_WHITE);
    ui_show_text(cur_row, 0, OTG_TEST_START);
    gr_flip();

    thread_run=1;
    pthread_create(&t1, NULL, otg_test_check_thread, NULL);
    setPlugState(1);

    usleep(100*1000);
    ret = ui_handle_button(TEXT_PASS, NULL,TEXT_FAIL);
    thread_run=0;
    pthread_join(t1, NULL);
    save_result(CASE_TEST_OTG,ret);
    setPlugState(0);

    usleep(500 * 1000);
    return ret;
}
