#include "common.h"

volatile bool flashthread_run = true;
int set_file_value(char *file_name, int value);

int set_file_value(char *file_name, char* value)
{
    int fd;
    int ret;
    char buffer[8];
    LOGD("SetFileValue file_name=%s,value=%s",file_name,value);
    fd = open(file_name, O_RDWR);
    if(fd < 0){
        LOGE("open %s failed! %d IN", file_name, __LINE__);
        return -1;
    }
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "%s", value);
    ret = write(fd, buffer, strlen(buffer));
    close(fd);
    return ret;
}

void open_flash_light(int i)
{
    int ret = 0;
#ifndef HAL_FLASH_FUN
    //ret = system("echo 0x30 > /sys/class/misc/sprd_flash/test") ? -1 : 0;		//for sprdroid 7.0
    ret = set_file_value("/sys/class/misc/sprd_flash/test","0x30");
    if( -1 == ret )
        //ret = system("echo 0x02 > /sys/devices/virtual/flash_test/flash_test/flash_value") ? -1 : 0;		//for sprdroid 6.0
        ret = set_file_value("/sys/devices/virtual/flash_test/flash_test/flash_value","0x02");
#else
   if (i == 0 || i == -1)
   {
        //ret = system("echo 0x10 > /sys/class/misc/sprd_flash/test") ? -1 : 0;
        ret = set_file_value("/sys/class/misc/sprd_flash/test","0x10");
   }
   if (i == 1 || i == -1)
   {
        //ret = system("echo 0x20 > /sys/class/misc/sprd_flash/test") ? -1 : 0;
        ret = set_file_value("/sys/class/misc/sprd_flash/test","0x20");
   }
#endif
}

void close_flash_light(int i)
{
    int ret = 0;
#ifndef HAL_FLASH_FUN
    //ret = system("echo 0x31 > /sys/class/misc/sprd_flash/test") ? -1 : 0;		//for sprdroid 7.0
    ret = set_file_value("/sys/class/misc/sprd_flash/test","0x31");
    if( -1 == ret )
        //ret = system("echo 0x00 > /sys/devices/virtual/flash_test/flash_test/flash_value") ? -1 : 0;		//for sprdroid 6.0
        ret = set_file_value("/sys/devices/virtual/flash_test/flash_test/flash_value","0x00");
#else
   if (i == 0 || i == -1)
   {
        //ret = system("echo 0x11 > /sys/class/misc/sprd_flash/test") ? -1 : 0;
        ret = set_file_value("/sys/class/misc/sprd_flash/test","0x11");
   }
   if (i == 1 || i == -1)
   {
        //ret = system("echo 0x21 > /sys/class/misc/sprd_flash/test") ? -1 : 0;
        ret = set_file_value("/sys/class/misc/sprd_flash/test","0x21");
   }
#endif
}

void *flash_thread(void *)
{
    int i = 0;

    LOGD("flash thread begin...");

    if(getHardwareRes()[CASE_TEST_FLASH].support == 1)		// flashing light
    {
        LOGD("flashing light");
        while(flashthread_run)
        {
            LOGD("%d is on", i % 2);
            open_flash_light(i % 2);
            usleep(1000 * 1000);
            LOGD("%d is off", i % 2);
            close_flash_light(i % 2);
            usleep(1000 * 1000);
            i++;
        }
    }
    else if(getHardwareRes()[CASE_TEST_FLASH].support == 2)		//always light on
    {
        LOGD("always light on");
        while(flashthread_run)
        {
            LOGD("%d is on", i % 2);
            open_flash_light(i % 2);
            usleep(500 * 1000);
            i++;
        }
    }
    LOGD("flash thread end...");

    close_flash_light(-1);

    return NULL;
}

int test_flashlight_start_common(void)
{
    pthread_t t1;
    volatile int  rtn = RL_FAIL;
    
    LOGD("enter flashlight test");
    ui_fill_locked();
    ui_show_title(MENU_TEST_FLASHLIGHT);
    
    flashthread_run = true;
    pthread_create(&t1, NULL, flash_thread, NULL);
    
    rtn = ui_handle_button(TEXT_PASS,NULL,TEXT_FAIL);//TEXT_GOBACK
    gr_flip();
    flashthread_run = false;
    pthread_join(t1, NULL);
    close_flash_light(-1);
    
    gr_flip();
    save_result(CASE_TEST_FLASHLIGHT,rtn);
    
    usleep(500 * 1000);
    return rtn;
}
