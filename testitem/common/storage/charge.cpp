#include "common.h"
#include "modem.h"

static int thread_run=0;
static int ext_kernel_k414 = 0;

unsigned int charge_get_adccali(void)
{
    unsigned int ret = 0;
    char cali[16];
    char adccali[20];
    char *ptr, *start_ptr, *end_ptr;

    memset(adccali, 0, sizeof(adccali));
    if(sendATCmd(0, "AT+SGMR=0,0,4", adccali, sizeof(adccali), 0) < 0)
    {
        LOGD("get adc cali fail");
        return 0;
    }

    if(strstr(adccali, "ERR"))
    {
        LOGD("get adc cali error info");
        return 0;
    }

    ptr = strchr(adccali, ':');
    ptr++;
    while(isspace(*ptr) || (*ptr == 0x0d) || (*ptr == 0x0a))
        ptr++;

    start_ptr = ptr;

    while(!isspace(*ptr) && (*ptr != 0x0d) && (*ptr != 0x0a) && (*ptr))
        ptr++;

    end_ptr = ptr;

    memset(cali, 0, sizeof(cali));
    snprintf(cali, end_ptr - start_ptr + 1, "%s", start_ptr);
    ret = strtoul(cali, 0, 16);
    LOGD("cali=%s, ret=0x%x",  cali, ret);
    if((ret & SPRD_CALI_MASK) > 0)
        ret = 1;
    else
        ret = 0;

    return ret;
}

float charge_get_batvol(void)
{
    int fd = -1;
    int voltage = 0, n = 0;
    float vol = 0.000;
    char buffer[16];

    int kernel_k414 = 0;

    if (access(ENG_BATVOL_k414, F_OK) == 0) {
        fd = open(ENG_BATVOL_k414, O_RDONLY);
        kernel_k414 = 1;
        LOGD("open %s ,fd=%d",ENG_BATVOL_k414,fd);
    }else{
        fd = open(ENG_BATVOL, O_RDONLY);
        kernel_k414 = 0;
        LOGD("open %s ,fd=%d",ENG_BATVOL,fd);
    }
    if(fd >= 0)
    {
        memset(buffer, 0, sizeof(buffer));
        n = read(fd, buffer, sizeof(buffer));
        if(n > 0)
        {
            voltage = atoi(buffer);
            if(kernel_k414){
                voltage = voltage / 1000;
            }
            vol = ((float) voltage) * 0.001;
            LOGD("voltage=%d, vol=%f", voltage, vol);
        }
        close(fd);
    }
    LOGD("NATIVE MMI %s, %d IN", __func__, __LINE__);
    return vol;
}

float charge_get_chrvol(void)
{
    int fd = -1;
    int voltage = 0, n = 0;
    float vol = 0.000;
    char buffer[16];

    int kernel_k414 = 0;
    if (access(ENG_CHRVOL_k414, F_OK) == 0) {
        fd = open(ENG_CHRVOL_k414, O_RDONLY);
        kernel_k414 = 1;
        LOGD("open %s ,fd=%d",ENG_CHRVOL_k414,fd);
    }else{
        fd = open(ENG_CHRVOL, O_RDONLY);
        kernel_k414 = 0;
        LOGD("open %s ,fd=%d",ENG_CHRVOL,fd);
    }
    if(fd >= 0)
    {
        memset(buffer, 0, sizeof(buffer));
        n = read(fd, buffer, sizeof(buffer));
        if(n > 0)
        {
            voltage = atoi(buffer);
            if(kernel_k414){
                voltage = voltage / 1000;
            }
            vol = ((float) voltage) * 0.001;
            LOGD("voltage=%d, vol=%f", voltage, vol);
        }
        close(fd);
    }
    return vol;
}

void charge_get_chrcur(char *current, int length)
{
    int fd = -1;
    int n = 0;

    int current_int = 0;
    float current_float = 0.000;
    char temp[56] = {0};

    int kernel_k414 = 0;
    if (access(ENG_CURRENT_k414, F_OK) == 0) {
        fd = open(ENG_CURRENT_k414, O_RDONLY);
        kernel_k414 = 1;
        ext_kernel_k414 = 1;
        LOGD("open %s ,fd=%d",ENG_CURRENT_k414,fd);
    }else{
        fd = open(ENG_CURRENT, O_RDONLY);
        kernel_k414 = 0;
        ext_kernel_k414 = 0;
        LOGD("open %s ,fd=%d",ENG_CURRENT,fd);
    }

    if(fd >= 0)
    {
     if(kernel_k414){
        n = read(fd, temp, sizeof(temp));
        if(n > 0)
        {
            LOGD("%s temp current=%s", ENG_CURRENT_k414, temp);
        }
        current_int = atoi(temp);
        current_float = ((float) current_int) * 0.001;
        LOGD("%s current_int=%d", ENG_CURRENT_k414, current_int);
        snprintf(current, length, "%.2f", current_float);
    }else{
        n = read(fd, current, length);
        if(n > 0)
        {
            LOGD("%s current=%s", ENG_CURRENT, current);
        }
    }
        close(fd);
    }
}

int charge_get_usbin(void)
{
    int fd = -1;
    int usbin = 0, n = 0;
    char buffer[16];

    int kernel_k414 = 0;
    fd = open(ENG_USBONLINE_k414, O_RDONLY);
    if (fd >= 0) {
        kernel_k414 = 1;
        LOGD("open %s ,fd=%d",ENG_USBONLINE_k414,fd);
    }else{
        fd = open(ENG_USBONLINE, O_RDONLY);
        kernel_k414 = 0;
        LOGD("open %s ,fd=%d",ENG_USBONLINE,fd);
    }

    if(fd >= 0)
    {
        memset(buffer, 0, sizeof(buffer));
        n = read(fd, buffer, sizeof(buffer));
        if(n > 0)
        {
            usbin = atoi(buffer);
            LOGD("%s usbin=%d", ENG_USBONLINE, usbin);
        }
        close(fd);
    }

    return usbin;
}

int charge_get_acin(void)
{
    int fd = -1;
    int acin = 0, n = 0;
    char buffer[16];

    int kernel_k414 = 0;
    fd = open(ENG_ACONLINE_k414, O_RDONLY);
    if (fd >= 0) {
        kernel_k414 = 1;
        LOGD("open %s ,fd=%d",ENG_ACONLINE_k414,fd);
    }else{
        fd = open(ENG_ACONLINE, O_RDONLY);
        kernel_k414 = 0;
        LOGD("open %s ,fd=%d",ENG_ACONLINE,fd);
    }

    if(fd >= 0)
    {
        memset(buffer, 0, sizeof(buffer));
        n = read(fd, buffer, sizeof(buffer));
        if(n > 0)
        {
            acin = atoi(buffer);
            LOGD("%s acin=%d", ENG_ACONLINE, acin);
        }
        close(fd);
    }

    return acin;
}

static int enable_charge(void)
{
    int fd;
    int ret = -1;

    int kernel_k414 = 0;
    if (access(ENG_STOPCHG_k414, F_OK) == 0) {
        fd = open(ENG_STOPCHG_k414, O_WRONLY);
        kernel_k414 = 1;
        LOGD("open %s ,fd=%d",ENG_STOPCHG_k414,fd);
    }else{
        fd = open(ENG_STOPCHG, O_WRONLY);
        kernel_k414 = 0;
        LOGD("open %s ,fd=%d",ENG_STOPCHG,fd);
    }

    if(fd >= 0)
    {
        ret = write(fd, "0", 2);
        if(ret < 0)
        {
            close(fd);
            return 0;
        }
        close(fd);
    }
    else
    {
        LOGE("mmitest open %s failed!", ENG_STOPCHG);
        return 0;
    }
    return 1;
}

static int disable_charge(void)
{
    int fd;
    int ret = -1;

    int kernel_k414 = 0;
    if (access(ENG_STOPCHG_k414, F_OK) == 0) {
        fd = open(ENG_STOPCHG_k414, O_WRONLY);
        kernel_k414 = 1;
        LOGD("open %s ,fd=%d",ENG_STOPCHG_k414,fd);
    }else{
        fd = open(ENG_STOPCHG, O_WRONLY);
        kernel_k414 = 0;
        LOGD("open %s ,fd=%d",ENG_STOPCHG,fd);
    }

    if(fd >= 0)
    {
        ret = write(fd, "1", 2);
        if(ret < 0)
        {
            close(fd);
            return 0;
        }
        close(fd);
    }
    else
    {
        LOGE("mmitest open %s failed!", ENG_STOPCHG);
        return 0;
    }
    return 1;
}

static int  battery_online(void)
{
    int fd;
    int ret = -1;
    char online[32];


    int kernel_k414 = 0;
    fd = open(ENG_BATONLINE_k414, O_RDONLY);
    LOGD("open1 %s ,fd=%d",ENG_BATONLINE_k414,fd);
    if (fd >= 0) {
        kernel_k414 = 1;
    }else{
        fd = open(ENG_BATONLINE, O_RDONLY);
        kernel_k414 = 0;
        LOGD("open %s ,fd=%d",ENG_BATONLINE,fd);
    }

    if(fd >= 0)
    {
        ret = read(fd, online, sizeof(online));
        close(fd);
    }

    LOGD("online = %s",online);
    if(strncmp(online, "Good", 4) == 0)
        return 1;
    else
        return 0;
}

static int battery_current(void)
{
    int fd = -1;
    int batcur = 0, n = 0;
    unsigned char current[16];

    memset(current, 0, sizeof(current));

    int kernel_k414 = 0;
    if (access(ENG_BATCURRENT_k414, F_OK) == 0) {
        fd = open(ENG_BATCURRENT_k414, O_RDONLY);
        kernel_k414 = 1;
        LOGD("open %s ,fd=%d",ENG_BATCURRENT_k414,fd);
    }else{
        fd = open(ENG_BATCURRENT, O_RDONLY);
        kernel_k414 = 0;
        LOGD("open %s ,fd=%d",ENG_BATCURRENT,fd);
    }

    if(fd >= 0)
    {
        n = read(fd, current, sizeof(current));
        if(n > 0)
        {
            batcur = atoi((const char *)current);
         LOGD("mmitest batcur1=%d", batcur);
            if(kernel_k414) batcur = batcur * 0.001;
            LOGD("mmitest batcur2=%d", batcur);
        }
        close(fd);
    }
    return batcur;
}

static int  battery_is_charging(void)
{
    int fd;
    int ret = -1;
    char status[32];
    fd = open(ENG_BATSTATUS_k414, O_RDONLY);
    LOGD("open %s ,fd=%d",ENG_BATSTATUS_k414,fd);

    if(fd >= 0)
    {
        ret = read(fd, status, sizeof(status));
        close(fd);
    }

    LOGD("status = %s",status);
    if(strncmp(status, "Charging", 8) == 0)
        return 1;
    else
        return 0;
}

static void bubble_sort(int a[], int n)
{
    int temp, i, j;
    for (i = 0; i < n; i++)
    {
        for (j = i; j < n; j++)
        {
            if (a[i] > a[j])
            {
                temp = a[i];
                a[i] = a[j];
                a[j] = temp;
            }
        }
    }
}

static void *charge_thread(void *)
{
    float vol = 0.0, chrvol = 0.0;
    int current = 0, usbin = 0, acin = 0, chrin = 0;
    unsigned int cali = 0;
    char buffer[64];
    char tmpbuf[32];
    int current_array[50];
    int start_row = 2;
    int last_row;
    int i, j, vol_count = 0;
    int current_avr = 0;
    int current_sum = 0;
    int bat_cur1, bat_cur2, bat_cur3, cur_diff1, cur_diff2;
    int extic_charge = 0, is_charge = 0;

    while(!battery_online() && thread_run)
    {
        LOGD("Native MMI Test: %s, %d IN\n", __func__, __LINE__);
        if( (charge_get_usbin() || charge_get_acin()))
        {
            LOGD("Native MMI Test: %s, %d IN\n", __func__, __LINE__);
            enable_charge();//open charge
            setPlugState(OFF);
            break;
        }
        else
        {
            LOGD("Native MMI Test: %s, %d IN\n", __func__, __LINE__);
            ui_set_color(CL_RED);
            last_row = ui_show_text(start_row, 0, TEXT_CHG_RUN_N);
            gr_flip();
            usleep(100 * 1000);
            ui_clear_rows(start_row, last_row - start_row);
            setPlugState(ON);
        }
    }

    usleep(100 * 1000);
    while(thread_run == 1)
    {
        //show battery voltage
        memset(buffer, 0, sizeof(buffer));
        vol = charge_get_batvol();
        LOGD("NATIVE MMI %s, %d IN", __func__, __LINE__);
        snprintf(buffer, sizeof(buffer), "%s%.2f %s", TEXT_CHG_BATTER_VOL, vol, "V");
        ui_clear_rows(start_row, 5);
        ui_set_color(CL_WHITE);
        last_row = ui_show_text(start_row, 0, buffer);

        //show charing or not-judge charge in
        usbin = charge_get_usbin();
        acin = charge_get_acin();
        if(usbin == 1 || acin == 1)
        {
            setPlugState(OFF);
            enable_charge();//open charge
        }
        else
        {
            setPlugState(ON);
        }
        //show charing or not-get current value
        memset(tmpbuf, 0, sizeof(tmpbuf));
        charge_get_chrcur(tmpbuf, sizeof(tmpbuf));
        LOGD("mmitest extic_charge=%s", tmpbuf);
        if(NULL != strstr(tmpbuf, "ext charge ic"))
        {
            LOGD("NATIVE MMI %s, %d IN", __func__, __LINE__);
            extic_charge = 1;
            is_charge = 1;
        }
        else if(NULL != strstr(tmpbuf, "discharging"))
        {
            LOGD("NATIVE MMI %s, %d IN", __func__, __LINE__);
            is_charge = 0;
        }
        else
        {
            LOGD("NATIVE MMI %s, %d IN", __func__, __LINE__);
         if(ext_kernel_k414 == 1){
          extic_charge = 1;
         }
            is_charge = 1;
            for(i = 0; i < 50; i++)
            {
                usleep(10);
                charge_get_chrcur(tmpbuf, sizeof(tmpbuf));
                current_array[i] = atoi(tmpbuf);
            }
            bubble_sort(current_array, 50);

            current_sum = 0;
            for(i = 10; i < 40; i++)
            {
                current_sum += current_array[i];
            }
            current_avr = current_sum / 30;

            LOGD("mmitest current after=%d, charge current=%d", current_sum, current_avr);
        }

        if(getPlugState() == 0 && is_charge == 1)
        {
            ui_set_color(CL_GREEN);
            last_row = ui_show_text(last_row, 0, TEXT_CHG_RUN_Y);
            LOGD("NATIVE MMI %s, %d IN", __func__, __LINE__);
            gr_flip();
        }
        else
        {
            ui_set_color(CL_RED);
            last_row = ui_show_text(last_row, 0, TEXT_CHG_RUN_N);
            LOGD("NATIVE MMI %s, %d IN", __func__, __LINE__);
            gr_flip();
            continue;
        }

        //show charger type
        memset(buffer, 0, sizeof(buffer));
        ui_set_color(CL_WHITE);
        if(usbin == 1)
        {
            snprintf(buffer, sizeof(buffer), "%s%s", TEXT_CHG_TYPE, "USB");
        }
        else if (acin == 1)
        {
            snprintf(buffer, sizeof(buffer), "%s%s", TEXT_CHG_TYPE, "AC");
        }
        last_row = ui_show_text(last_row, 0, buffer);

        if(getPlugState() == 0 && is_charge == 1)
        {
            memset(buffer, 0, sizeof(buffer));
            //show charger voltage
            chrvol = charge_get_chrvol();
            snprintf(buffer, sizeof(buffer), "%s%.2f %s", TEXT_CHG_VOL, chrvol, "V");
            last_row = ui_show_text(last_row, 0, buffer);
            gr_flip();
            //show charger current
            if(1 == extic_charge)
            {
                sleep(2);
                bat_cur1 = battery_current();
                if(bat_cur1 >= -40)
                {
                    ui_set_color(CL_GREEN);
                    memset(buffer, 0, sizeof(buffer));
                    snprintf(buffer, sizeof(buffer), "%s%d %s", TEXT_BATTARY_CUR, bat_cur1, "mA");
                    last_row = ui_show_text(last_row, 0, buffer);
                    last_row = ui_show_text(last_row, 0, TEXT_TEST_PASS);
                    gr_flip();
                    sleep(1);
                    ui_push_result(RL_PASS);
                    disable_charge();
                    return NULL;
                }
                else
                {
                    ui_set_color(CL_GREEN);
                    memset(buffer, 0, sizeof(buffer));
                    snprintf(buffer, sizeof(buffer), "%s%d %s", TEXT_BATTARY_CUR, bat_cur1, "mA");
                    ui_show_text(last_row, 0, buffer);
                    gr_flip();
                    disable_charge();
                    sleep(2);
                    bat_cur2 = battery_current();
                    ui_clear_rows(last_row, 1);
                    ui_set_color(CL_GREEN);
                    memset(buffer, 0, sizeof(buffer));
                    snprintf(buffer, sizeof(buffer), "%s%d %s", TEXT_BATTARY_CUR, bat_cur2, "mA");
                    ui_show_text(last_row, 0, buffer);
                    gr_flip();
                    enable_charge();
                    sleep(2);
                    bat_cur3 = battery_current();
                    ui_clear_rows(last_row, 1);
                    ui_set_color(CL_GREEN);
                    memset(buffer, 0, sizeof(buffer));
                    snprintf(buffer, sizeof(buffer), "%s%d %s", TEXT_BATTARY_CUR, bat_cur3, "mA");
                    last_row = ui_show_text(last_row, 0, buffer);
                    gr_flip();
                    disable_charge();
                    cur_diff1 = bat_cur1 - bat_cur2;
                    cur_diff2 = bat_cur3 - bat_cur2;
                    LOGD("mmitest cur_diff1=%d,cur_diff2=%d", cur_diff1, cur_diff2);
                    if((cur_diff1 > 300 && cur_diff2 > 300) || (bat_cur3 > -40))
                    {
                        ui_set_color(CL_GREEN);
                        last_row = ui_show_text(last_row, 0, TEXT_TEST_PASS);
                        gr_flip();
                        sleep(2);
                        ui_push_result(RL_PASS);
                        return  NULL;
                    }
                    else
                    {
                        ui_set_color(CL_RED);
                        last_row = ui_show_text(last_row, 0, TEXT_TEST_FAIL);
                        gr_flip();
                        sleep(2);
                        ui_push_result(RL_FAIL);
                        return  NULL;
                    }
                }
            }
            else
            {
                memset(buffer, 0, sizeof(buffer));
                snprintf(buffer, sizeof(buffer), "%s%d%s", TEXT_CHG_CUR, current_avr, "mA");
                last_row = ui_show_text(last_row, 0, buffer);
                disable_charge();

                if(current_avr < 200)
                {
                    ui_set_color(CL_RED);
                    if(vol >= 4.15)
                        last_row = ui_show_text(last_row, 0, CHARGE_TIPS);
                    ui_show_text(last_row, 0, TEXT_TEST_FAIL);
                    gr_flip();
                    sleep(1);
                    ui_push_result(RL_FAIL);
                    return  NULL;
                }
                else
                {
                    ui_push_result(RL_PASS);
                    ui_set_color(CL_GREEN);
                    ui_show_text(last_row, 0, TEXT_TEST_PASS);
                    gr_flip();
                    sleep(1);
                    return  NULL;
                }
            }
        }
    }

    return NULL;
}


static void *charge_thread_newapi(void *)
{
    float vol = 0.0, chrvol = 0.0;
    int current = 0, usbin = 0, acin = 0, chrin = 0;
    unsigned int cali = 0;
    char buffer[64];
    char tmpbuf[32];
    int current_array[50];
    int start_row = 2;
    int last_row;
    int i, j, vol_count = 0;
    int bat_cur1, bat_cur2, bat_cur3, cur_diff1, cur_diff2;
    int extic_charge = 0, is_charge = 0;

    usleep(100 * 1000);
    while(thread_run == 1)
    {
        //show battery voltage
        memset(buffer, 0, sizeof(buffer));
        vol = charge_get_batvol();
        LOGD("NATIVE MMI %s, %d IN", __func__, __LINE__);
        snprintf(buffer, sizeof(buffer), "%s%.2f %s", TEXT_CHG_BATTER_VOL, vol, "V");
        ui_clear_rows(start_row, 5);
        ui_set_color(CL_WHITE);
        last_row = ui_show_text(start_row, 0, buffer);

     is_charge = battery_is_charging();

    LOGD("mmitest is_charge=%d", is_charge);
        if(is_charge == 1)
        {
            ui_set_color(CL_GREEN);
            last_row = ui_show_text(last_row, 0, TEXT_CHG_RUN_Y);
            LOGD("NATIVE MMI %s, %d IN", __func__, __LINE__);
            gr_flip();
        }
        else
        {
            ui_set_color(CL_RED);
            last_row = ui_show_text(last_row, 0, TEXT_CHG_RUN_N);
            LOGD("NATIVE MMI %s, %d IN", __func__, __LINE__);
            gr_flip();
            continue;
        }

        if(is_charge == 1)
        {
            memset(buffer, 0, sizeof(buffer));
            //show charger voltage
            chrvol = charge_get_chrvol();
            snprintf(buffer, sizeof(buffer), "%s%.2f %s", TEXT_CHG_VOL, chrvol, "V");
            last_row = ui_show_text(last_row, 0, buffer);
            gr_flip();
            //show charger current
        disable_charge();
           sleep(2);
           bat_cur2 = battery_current();
           ui_clear_rows(last_row, 1);
           ui_set_color(CL_GREEN);
           memset(buffer, 0, sizeof(buffer));
           snprintf(buffer, sizeof(buffer), "%s%d %s", TEXT_BATTARY_CUR, bat_cur2, "mA");
           ui_show_text(last_row, 0, buffer);
           LOGD("mmitest bat_cur2=%d", bat_cur2);
           gr_flip();
           enable_charge();
           sleep(2);
           bat_cur3 = battery_current();
           cur_diff2 = bat_cur3 - bat_cur2;
           LOGD("mmitest cur_diff1=%d,cur_diff2=%d,chrvol=%f", cur_diff1, cur_diff2,chrvol);

           ui_clear_rows(last_row, 1);
           ui_set_color(CL_GREEN);
           memset(buffer, 0, sizeof(buffer));
           snprintf(buffer, sizeof(buffer), "%s%d %s", TEXT_BATTARY_CUR, cur_diff2, "mA");
           LOGD("mmitest cur_diff2=%d", cur_diff2);
           last_row = ui_show_text(last_row, 0, buffer);
           gr_flip();

           if(cur_diff2 >= 200)
           {
               ui_set_color(CL_GREEN);
               last_row = ui_show_text(last_row, 0, TEXT_TEST_PASS);
               gr_flip();
               sleep(1);
               ui_push_result(RL_PASS);
               return  NULL;
           }
           else if(cur_diff2 > 100 && cur_diff2 < 200 && chrvol >= 4.1)
           {
               ui_set_color(CL_GREEN);
               last_row = ui_show_text(last_row, 0, TEXT_TEST_PASS);
               gr_flip();
               sleep(1);
               ui_push_result(RL_PASS);
               return  NULL;
           }
           else
           {
               ui_set_color(CL_RED);
               last_row = ui_show_text(last_row, 0, TEXT_TEST_FAIL);
               gr_flip();
               sleep(1);
               ui_push_result(RL_FAIL);
               return  NULL;
           }
        }
    }

    return NULL;
}

int test_charge_start_common(void)
{
    int ret = 0;
    pthread_t thead;

    ui_fill_locked();
    ui_show_title(MENU_TEST_CHARGE);
    gr_flip();
    LOGD("start");
    thread_run = 1;
    ext_kernel_k414 = 0;
    pthread_create(&thead, NULL, charge_thread_newapi, NULL);
    usleep(10 * 1000);
    ret = ui_handle_button(NULL, NULL, TEXT_FAIL);
    thread_run = 0;
    pthread_join(thead, NULL);
    save_result(CASE_TEST_CHARGE, ret);
    setPlugState(OFF);

    usleep(500 * 1000);
    return ret;
}
