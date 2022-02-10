#include "common.h"

#define FINGERPRINT_LIB_Default "/vendor/lib/libfactorylib.so"             //default fingerprint lib path

typedef int (*CAC_FUNC)(void);

int test_fingersor_start_common(void)
{
    int ret = RL_NA;
    int row = 2;
    int pressed = 0;
    int times = 100;
    int detect_result = -1;

    void *handle;
    char *error;
    CAC_FUNC  cac_func = NULL;

    ui_fill_locked();
    ui_show_title(MENU_TEST_FINGERSOR);
    ui_set_color(CL_WHITE);
    ui_show_text(row, 0, FINGERSOR_TEST_TIPS);
    gr_flip();

    while(KEY_VIR_NEXT_PAGE != pressed)
    {
        pressed = ui_wait_button(NULL, TEXT_START, NULL);
    }
    ui_set_color(CL_SCREEN_BG);
    gr_fill(0, ui_getTitleHeight(), gr_fb_width(), gr_fb_height());

    ui_set_color(CL_WHITE);
    ui_show_text(row, 0, FINGER_INIT_TIPS);
    gr_flip();

    LOGD("dlopen fingerprint lib");
    handle = dlopen(FINGERPRINT_LIB_Default, RTLD_LAZY);
    if (!handle)
    {
        ret = RL_FAIL;
        ui_clear_rows(row, 2);
        ui_set_color(CL_RED);
        row = ui_show_text(row, 0, FINGER_DLOPEN_FAIL);
        gr_flip();
        LOGE("fingersor lib dlopen failed! %s, exist fingerprint test!\n", dlerror());
        goto end;
    }

    LOGD("do factory_init");
    *(void **) (&cac_func) = dlsym(handle, "factory_init");
    if(cac_func && (*cac_func)() == -1)
    {
        ret = RL_FAIL;
        ui_clear_rows(row, 2);
        ui_set_color(CL_RED);
        row = ui_show_text(row, 0, FINGER_INIT_FAIL);
        gr_flip();
        LOGE("factory_init fail");
        goto end;
    }
    else if(!cac_func)
    {
        ret = RL_FAIL;
        LOGE("could not find symbol 'factory_init', %d IN\n", __LINE__);
        goto end;
    }

    ui_clear_rows(row, 2);
    ui_set_color(CL_GREEN);
    row = ui_show_text(row, 0, FINGER_INIT_PASS);
    gr_flip();

    LOGD("do spi_test");
    *(void **) (&cac_func) = dlsym(handle, "spi_test");
    if(cac_func && (*cac_func)() == -1)
    {
        ret = RL_FAIL;
        LOGE("spi_test fail");
        goto end;
    }
    else if(!cac_func)
    {
        ret = RL_FAIL;
        LOGE("could not find symbol 'spi_test', %d IN\n", __LINE__);
        goto end;
    }

    LOGD("do interrupt_test");
    *(void **) (&cac_func) = dlsym(handle, "interrupt_test");
    if(cac_func && (*cac_func)() == -1)
    {
        ret = RL_FAIL;
        LOGE("interrupt_test fail");
        goto end;
    }
    else if(!cac_func)
    {
        ret = RL_FAIL;
        LOGE("could not find symbol 'interrupt_test', %d IN\n", __LINE__);
        goto end;
    }

    ui_set_color(CL_WHITE);
    row = ui_show_text(row, 0, FINGERSOR_START_TIPS);
    gr_flip();

    LOGD("do deadpixel_test");
    *(void **) (&cac_func) = dlsym(handle, "deadpixel_test");
    if(cac_func && (*cac_func)() == -1)
    {
        ret = RL_FAIL;
        LOGE("deadpixel_test fail");
        goto end;
    }
    else if(!cac_func)
    {
        ret = RL_FAIL;
        LOGE("could not find symbol 'deadpixel_test', %d IN\n", __LINE__);
        goto end;
    }


    LOGD("do finger_detect");
    *(void **) (&cac_func) = dlsym(handle, "finger_detect");
    if(!cac_func)
    {
        ret = RL_FAIL;
        LOGE("could not find symbol 'finger_detect', %d IN\n", __LINE__);
        goto end;
    }
    else
    {
        while(times > 0)
        {
            detect_result = (*cac_func)();
            if(detect_result != -1)
            {
                ret = RL_PASS;
                break;
            }
            times--;
            LOGD("not detect fingerprint, try again...");
            usleep(1000*100);
        }
        if(detect_result < 0)
        {
            ret = RL_FAIL;
            LOGE("finger_detect %d times but failed", times);
        }
    }

    LOGD("do factory_exit");
    *(void **) (&cac_func) = dlsym(handle, "factory_exit");
    if(cac_func && (*cac_func)() == -1)
    {
        LOGE("factory_exit fail");
        goto end;
    }
    else if(!cac_func)
    {
        ret = RL_FAIL;
        LOGE("could not find symbol 'factory_exit', %d IN\n", __LINE__);
        goto end;
    }

end:
    if(ret == RL_PASS) {
        ui_set_color(CL_GREEN);
        ui_show_text(row, 0, TEXT_TEST_PASS);
    } else {
        ui_set_color(CL_RED);
        ui_show_text(row, 0, TEXT_TEST_FAIL);
    }
    gr_flip();

    save_result(CASE_TEST_FINGERSOR,ret);
    usleep(500 * 1000);
    return ret;

}
