#include "common.h"
#include "sdcardutils.h"
#include <fcntl.h>
#include <linux/fs.h> s

#define NEW_API 1

static int sdcard_rw(char* external_path)
{
    int fd;
    int ret = -1;
    int i = 0;
    unsigned char w_buf[RW_LEN];
    unsigned char r_buf[RW_LEN];
    //char external_path[MAX_NAME_LEN];
    char sdcard_testfile[MAX_NAME_LEN];

    snprintf(sdcard_testfile, sizeof(sdcard_testfile), "%s/test.txt",external_path);
    LOGD("mmitest the sdcard_testfile : %s", sdcard_testfile);

    for(i = 0; i < RW_LEN; i++) {
        w_buf[i] = 0xff & i;
    }

    fd = open(sdcard_testfile, O_CREAT|O_RDWR, 0666);
    if(fd < 0){
	 memset(sdcard_testfile, 0, sizeof(sdcard_testfile));
	 snprintf(sdcard_testfile, sizeof(sdcard_testfile), "%s/test.txt","/storage/sdcard0");
	 LOGD("create %s failed", sdcard_testfile);
	 fd = open(sdcard_testfile, O_CREAT|O_RDWR, 0666);
        if(fd < 0){
            LOGE("create %s failed", sdcard_testfile);
            goto RW_END;
        }
    }

    if(write(fd, w_buf, RW_LEN) != RW_LEN){
        LOGE("write data failed");
        goto RW_END;
    }

    lseek(fd, 0, SEEK_SET);
    memset(r_buf, 0, sizeof(r_buf));

    read(fd, r_buf, RW_LEN);
    if(memcmp(r_buf, w_buf, RW_LEN) != 0) {
        LOGE("read data failed");
        goto RW_END;
    }

    ret = 0;
RW_END:
    if(fd >= 0) close(fd);
    return ret;
}

int test_sdcard_pretest_common(void)
{
#ifdef NEW_API
    int ret;
    char *sdcard_path1 = NULL, *sdcard_path2 = NULL;
    get_sdcard_path(&sdcard_path1, &sdcard_path2);
    LOGD("test_sdcard_pretest_common open sdcard_path1=%s ,sdcard_path2=%s",sdcard_path1, sdcard_path2);

    ret = is_sdcard_exist(sdcard_path1);
    LOGD("test_sdcard_pretest_common ret=%d",ret);
    if(!ret) {
        ret= RL_FAIL;
    } else {
        ret= RL_PASS;
    }

    save_result(CASE_TEST_SDCARD,ret);
    return ret;
#else
    int fd;
    int ret;
    system(SPRD_MOUNT_DEV);
    fd = open(SPRD_SD_DEV, O_RDWR);
    if(fd < 0) {
        ret= RL_FAIL;
    } else {
        close(fd);
        ret= RL_PASS;
    }

    save_result(CASE_TEST_SDCARD,ret);
    return ret;
#endif
}

int mount_sdcard(char* sdcard_path){
    LOGD("mount_sdcard: %s to %s",sdcard_path,SPRD_MOUNT_TMP_PATH);
    char sdcard_mount_cmd[128];
    memset(sdcard_mount_cmd, 0, sizeof(sdcard_mount_cmd));
    snprintf(sdcard_mount_cmd, sizeof(sdcard_mount_cmd),  "mkdir %s", SPRD_MOUNT_TMP_PATH);

    //Mount sdard to SPRD_MOUNT_TMP_PATH
    LOGD("sdcard_mount_cmd: %s",sdcard_mount_cmd);
    system(sdcard_mount_cmd);
    if(mount(sdcard_path, SPRD_MOUNT_TMP_PATH, "vfat", 0, NULL) < 0 ){
		LOGE("%s mount failed",sdcard_path);
		return -1;
    }
    return 0;
}

int test_sdcard_start_common(void)
{
#ifdef NEW_API
    int fd_dev = -1,fd1_size = -1;
    int ret = RL_FAIL; //fail
    int cur_row = 2;
    int wait_cnt = 0;
    char temp[64],buffer[64];
    int read_len = 0;
    unsigned long value=0;
    char *endptr;

    char sdcard_path[128];
    char sdcard_size_path[128];
    int use_blk0 = 0;

    ui_fill_locked();
    ui_show_title(MENU_TEST_SDCARD);
    ui_set_color(CL_WHITE);
    cur_row = ui_show_text(cur_row, 0, TEXT_SD_START);
    gr_flip();
    //snprintf(sdcard_path, sizeof(sdcard_path),  "%s", SPRD_SD_DEV);

    //check if UFS
    //UNISOC:Bug1509535 Native MMI cat not mount external memory card
    if ((0 == access("/sys/block/mmcblk0",F_OK)) && (0 != access("/sys/block/mmcblk0boot0",F_OK))){
	use_blk0 = 1;
	memset(sdcard_path, 0, sizeof(sdcard_path));
       snprintf(sdcard_path, sizeof(sdcard_path),  "%s", "/dev/block/mmcblk0");
    }else{
	use_blk0 = 0;
	memset(sdcard_path, 0, sizeof(sdcard_path));
       //snprintf(sdcard_path, sizeof(sdcard_path),  "%s", "/dev/block/mmcblk0");
       snprintf(sdcard_path, sizeof(sdcard_path),  "%s", SPRD_SD_DEV);
    }

    mount_sdcard(sdcard_path);

    LOGD("open %s ",sdcard_path);
    fd_dev = open(sdcard_path, O_RDWR);
    if(fd_dev < 0) {
       LOGE("open %s failed",sdcard_path);
       ui_set_color(CL_RED);
       cur_row = ui_show_text(cur_row, 0, TEXT_SD_OPEN_FAIL);
       gr_flip();
       goto TEST_END;
    }
    ui_set_color(CL_GREEN);
    cur_row = ui_show_text(cur_row, 0, TEXT_SD_OPEN_OK);
    gr_flip();
    //if( mount(sdcard_path, "/storage/sdcard0", "vfat", 0, NULL) < 0 )
     //       LOGE("%s mount failed",sdcard_path);

    if(use_blk0){
	    memset(sdcard_size_path, 0, sizeof(sdcard_size_path));
	    snprintf(sdcard_size_path, sizeof(sdcard_size_path),  "%s", "/sys/block/mmcblk0/size");
    }else{
	    memset(sdcard_size_path, 0, sizeof(sdcard_size_path));
	    snprintf(sdcard_size_path, sizeof(sdcard_size_path),  "%s", SPRD_SD_DEV_SIZE);
    }
    LOGD("read %s ",sdcard_size_path);

    fd1_size = open(sdcard_size_path,O_RDONLY);
    if(fd1_size < 0) {
        LOGE("open %s failed",sdcard_size_path);
        ui_set_color(CL_RED);
        cur_row = ui_show_text(cur_row, 0, TEXT_SD_STATE_FAIL);
        gr_flip();
        goto TEST_END;
    }
    read_len = read(fd1_size,buffer,sizeof(buffer));
    if(read_len <= 0){
        LOGE("read %s failed,read_len=%d",sdcard_size_path,read_len);
        ui_set_color(CL_RED);
        cur_row = ui_show_text(cur_row, 0, TEXT_SD_STATE_FAIL);
        gr_flip();
        goto TEST_END;
    }

    value = strtoul(buffer,&endptr,0);
    LOGD("%s value = %lu, read_len = %d",sdcard_size_path, value, read_len);
    ui_set_color(CL_GREEN);
    cur_row = ui_show_text(cur_row, 0, TEXT_SD_STATE_OK);
    snprintf(temp, sizeof(temp), "%ld MB", (value/2/1024));
    cur_row = ui_show_text(cur_row, 0, temp);
    gr_flip();
    if(sdcard_rw(SPRD_MOUNT_TMP_PATH)< 0) {
        ui_set_color(CL_RED);
        cur_row = ui_show_text(cur_row, 0, TEXT_SD_RW_FAIL);
        gr_flip();
        goto TEST_END;
    } else {
        ui_set_color(CL_GREEN);
        cur_row = ui_show_text(cur_row, 0, TEXT_SD_RW_OK);
        gr_flip();
    }

    ret = RL_PASS;
TEST_END:
    if(ret == RL_PASS) {
        ui_set_color(CL_GREEN);
        cur_row = ui_show_text(cur_row, 0, TEXT_TEST_PASS);
    } else {
        ui_set_color(CL_RED);
        cur_row = ui_show_text(cur_row, 0, TEXT_TEST_FAIL);
    }
    gr_flip();
    sleep(1);
    if(fd_dev >= 0) close(fd_dev);
    if(fd1_size >= 0) close(fd1_size);
    save_result(CASE_TEST_SDCARD,ret);

    usleep(500 * 1000);
    return ret;
#else
    int fd_dev = -1,fd1_size = -1;
    int ret = RL_FAIL; //fail
    int cur_row = 2;
    int wait_cnt = 0;
    char temp[64],buffer[64];
    int read_len = 0;
    unsigned long value=0;
    char *endptr;

    ui_fill_locked();
    ui_show_title(MENU_TEST_SDCARD);
    ui_set_color(CL_WHITE);
    cur_row = ui_show_text(cur_row, 0, TEXT_SD_START);
    gr_flip();
    fd_dev = open(SPRD_SD_DEV, O_RDWR);
    if(fd_dev < 0) {
        LOGE("open %s failed",SPRD_SD_DEV);
        ui_set_color(CL_RED);
        cur_row = ui_show_text(cur_row, 0, TEXT_SD_OPEN_FAIL);
        gr_flip();
        goto TEST_END;
    }
    ui_set_color(CL_GREEN);
    cur_row = ui_show_text(cur_row, 0, TEXT_SD_OPEN_OK);
    gr_flip();
    //if( mount(SPRD_SD_DEV, "/storage/sdcard0", "vfat", 0, NULL) < 0 )
     //       LOGE("%s mount failed",SPRD_SD_DEV);

    system(SPRD_MOUNT_DEV);

    fd1_size = open(SPRD_SD_DEV_SIZE,O_RDONLY);
    if(fd1_size < 0) {
        LOGE("open %s failed",SPRD_SD_DEV_SIZE);
        ui_set_color(CL_RED);
        cur_row = ui_show_text(cur_row, 0, TEXT_SD_STATE_FAIL);
        gr_flip();
        goto TEST_END;
    }
    read_len = read(fd1_size,buffer,sizeof(buffer));
    if(read_len <= 0){
        LOGE("read %s failed,read_len=%d",SPRD_SD_DEV_SIZE,read_len);
        ui_set_color(CL_RED);
        cur_row = ui_show_text(cur_row, 0, TEXT_SD_STATE_FAIL);
        gr_flip();
        goto TEST_END;
    }

    value = strtoul(buffer,&endptr,0);
    LOGD("%s value = %lu, read_len = %d",SPRD_SD_DEV_SIZE, value, read_len);
    ui_set_color(CL_GREEN);
    cur_row = ui_show_text(cur_row, 0, TEXT_SD_STATE_OK);
    snprintf(temp, sizeof(temp), "%ld MB", (value/2/1024));
    cur_row = ui_show_text(cur_row, 0, temp);
    gr_flip();
    if(sdcard_rw(SPRD_SD_DEV)< 0) {
        ui_set_color(CL_RED);
        cur_row = ui_show_text(cur_row, 0, TEXT_SD_RW_FAIL);
        gr_flip();
        goto TEST_END;
    } else {
        ui_set_color(CL_GREEN);
        cur_row = ui_show_text(cur_row, 0, TEXT_SD_RW_OK);
        gr_flip();
    }

    ret = RL_PASS;
TEST_END:
    if(ret == RL_PASS) {
        ui_set_color(CL_GREEN);
        cur_row = ui_show_text(cur_row, 0, TEXT_TEST_PASS);
    } else {
        ui_set_color(CL_RED);
        cur_row = ui_show_text(cur_row, 0, TEXT_TEST_FAIL);
    }
    gr_flip();
    sleep(1);
    if(fd_dev >= 0) close(fd_dev);
    if(fd1_size >= 0) close(fd1_size);
    save_result(CASE_TEST_SDCARD,ret);

    usleep(500 * 1000);
    return ret;
#endif
}
