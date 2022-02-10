#include "common.h"
#include "chnmgr.h"
#include "modem.h"
#include <stdlib.h>

static int bits_per_pixel = 0;
static int camera_id;
static int framcount = 0;       //camera fram count
static unsigned char *temprgb = NULL;

#define MAX_PREVIEW_H 960
#define MAX_PREVIEW_W 720
#define TEXT_CAMERA_FACTORY_CALL "camera_draw_cb"

extern int test_case_support(unsigned char id);
extern void *flash_thread(void *);
extern void close_flash_light(int i);
extern bool flashthread_run;
extern int tp_flag1;

typedef enum CAMERA_INTERFACE
{
    interface_eng_tst_camera_init,
    interface_eng_tst_camera_deinit,
    interface_eng_camera_close,
};

static void pic_rotate(void *pDest, void *pSrc, int *Width, int *Height, int angle)
{
    unsigned int *des32 = (unsigned int *)pDest;
    unsigned int *src32 = (unsigned int *)pSrc;
    unsigned short *des16 = (unsigned short *)pDest;
    unsigned short *src16 = (unsigned short *)pSrc;

    int i = 0, j = 0;
    int size;
    int ori_width = *Width, ori_height = *Height;

    if ((!pDest) || (!pSrc))
    {
        LOGE("pic rotate error, data is NULL");
        return;
    }

    LOGD("RGBRotate %d degree",angle);
    if(angle == 270)		//90 anti clock wise
    {
        if (bits_per_pixel == 4)
        {
            des32 --;
            for (j = 0; j < ori_height; j++)
            {
                size = 0;
                for (i = 0; i < ori_width; )
                {
                    size += ori_height; des32[size - j] = src32[i++];
                }
                src32 += ori_width;
            }
        }
        else
        {
            des16--;
            for (j = 0; j < ori_height; j++)
            {
                size = 0;
                for (i = 0; i < ori_width; )
                {
                    size += ori_height;
                    des16[size - j] = src16[i++];
                }
            }
        }
    }
    else if(angle == 90)		//90 clock wise
    {
        if (bits_per_pixel == 4)
        {
            src32--;
            for (j = 0; j < ori_width; j++)
            {
                size = 0;
                for (i = 0; i < ori_height; )
                {
                    size += ori_width;
                    des32[i++] = src32[size - j];
                }
                des32 += ori_height;
            }
        }
        else
        {
            src16--;
            for (j = 0; j < ori_width; j++)
            {
                size = 0;
                for (i = 0; i < ori_height; i++)
                {
                    size += ori_width;
                    des16[i] = src16[size - j];
                }
                des16 += ori_height;
            }
        }
    }
    else if(angle == 180)		//180 clock wise
    {
        if (bits_per_pixel == 4)
        {
            src32 = src32 + ori_width * ori_height - 1;
            for (i = 0; i < ori_width * ori_height; i++)
            {
                des32[i] = *src32--;
            }
        }
        else
        {
            src16 = src16 + ori_width * ori_height - 1;
            for (i = 0; i < ori_width * ori_height; i++)
            {
                des16[i] = *src16--;
            }
        }
    }
    else
    {
        memcpy(pDest, pSrc, ori_width * ori_height * bits_per_pixel);
    }

    //swap Width and Height, if rotate 90 or 270 degree
    if(angle == 90 || angle == 270)
    {
        int temp = *Width;
        *Width = *Height;
        *Height = temp;
    }

}

static void pic_mirror(void *data, int width, int height)
{
    int i, j;
    unsigned int *data32 = (unsigned int *)data;
    unsigned short *data16 = (unsigned short *)data;

    LOGD("picture mirror");
    if (!data)
    {
        LOGE("pic mirror error, data is NULL");
        return;
    }

    if (bits_per_pixel == 4)
    {
        unsigned int swap32;
        for (j = 0; j < height; j++)
        {
            for (i = 0; i < width/2; i++)
            {
                swap32 = data32[i];
                data32[i] = data32[width-i-1];
                data32[width-i-1] = swap32;
            }
            data32 += width;
        }
    }
    else
    {
        unsigned short swap16;
        for (j = 0; j < height; j++)
        {
            for (i = 0; i < width/2; i++)
            {
                swap16 = data16[i];
                data16[i] = data16[width-i-1];
                data16[width-i-1] = swap16;
            }
            data16 += width;
        }
    }
}

static void pic_copy(void *pDest, int DestWidth, int DestHeight, void *pSrc, int SrcWidth, int SrcHeight)
{
    int i = 0;
    int size, offset;
    unsigned int *des32 = (unsigned int *)pDest;
    unsigned int *src32 = (unsigned int *)pSrc;
    unsigned short *des16 = (unsigned short *)pDest;
    unsigned short *src16 = (unsigned short *)pSrc;
    int start_y = ui_getTitleHeight();        //start display position
    int end_y = gr_fb_height()-ui_getButtonHeight();

    if(bits_per_pixel == 4)
    {
        size = SrcWidth * 4;
        offset = DestWidth * start_y;
        for(i = 0; i < SrcHeight && i  < (end_y-start_y); i++)
        {
            memcpy(des32 + offset, src32, size);
            src32 = src32 + SrcWidth;
            des32 = des32 + DestWidth;
        }
    }
    else
    {
        size = SrcWidth * 2;
        offset = DestWidth * start_y;
        for(i = 0; i < SrcHeight && i < (end_y-start_y) ; i++)
        {
            memcpy(des16 + offset, src16, size);
            src16 = src16 + SrcWidth;
            des16 = des16 + DestWidth;
        }
    }
}

static void* camera_draw_cb(int fram_width, int fram_height, unsigned char *rgb_data)
{
    minui_backend* backend_t = gr_backend_get();
    GRSurface* draw_t = gr_draw_get();
    int bits = draw_t->pixel_bytes;
    void* draw_data = draw_t->data;
    int width = fram_width;
    int height = fram_height;

    static struct timeval startTime,endTime;

    gettimeofday(&startTime,NULL);

    if(fram_width <= 0 || fram_height <= 0)
        LOGE("fram size error, size=(%d, %d)", width, height);
    else
        LOGD("fram size=(%d, %d)", width, height);

    //picture rotate 0/90/180/270 degree clockwise
    if(camera_id == 0)      //if back camera, rotate 270 clockwise
        pic_rotate(temprgb, rgb_data, &width, &height, 270);
    else if(camera_id == 1)      //if front camera, rotate 90 clockwise
    {
        pic_rotate(temprgb, rgb_data, &width, &height, 90);
        pic_mirror(temprgb, width, height);
    }
    else if(camera_id == 2)      //if back auxiliary camera, rotate 270 clockwise
        pic_rotate(temprgb, rgb_data, &width, &height, 270);
    else if(camera_id == 3)      //if front auxiliary camera, rotate 90 clockwise
        pic_rotate(temprgb, rgb_data, &width, &height, 90);

    pic_copy(draw_data, gr_fb_width(), gr_fb_height(), (void*)temprgb, width, height);
    framcount++;

    tp_flag1 = 1;
    draw_t = backend_t->flip(backend_t);
    tp_flag1 = 0;

    gettimeofday(&endTime,NULL);
    ALOGI("camera_draw_cb use %3.1f ms",(endTime.tv_sec - startTime.tv_sec)*1000 + (endTime.tv_usec - startTime.tv_usec)/1000.0);

    return NULL;
}

// 预定义接口，还没实现
int eng_tst_camera_at_cmd(int interface_eng_camera,int camera_id,int preview_width,int preview_height,int bits_per_pixel)
{
    int ret = -1;
    char cmd[256] = {0};
    char buff[256] = {0};
    char *temp="AT+";
    LOGD("FT: eng_tst_camera_at_cmd interface=%d,id=%d,w=%d,h=%d,bit=%d",interface_eng_camera, camera_id , preview_width,preview_height,bits_per_pixel);
    switch(interface_eng_camera){
        case interface_eng_tst_camera_init:
                snprintf(cmd, sizeof(cmd),  "%s=%d,%d,%d,%d,%d", temp, interface_eng_camera, camera_id ,preview_width, preview_height,bits_per_pixel);
                break;
        case interface_eng_tst_camera_deinit:
                snprintf(cmd, sizeof(cmd),  "%s=%d", temp, interface_eng_camera);
                break;
        case interface_eng_camera_close:
                snprintf(cmd, sizeof(cmd),  "%s=%d", temp, interface_eng_camera);
                break;
        default:
                LOGD("FT: eng_tst_camera_at_cmd: unknow interface!");
                break;
    }
    LOGD("FT: eng_tst_camera_at_cmd: cmd=%s",cmd);
    ret = chnl_send(CHNL_AT,cmd, strlen(cmd), buff, sizeof(buff));
    LOGD("FT: eng_tst_camera_at_cmd: ret=%d",ret);
    return ret;
}

int eng_tst_camera_init(int camera_id,int preview_width,int preview_height,int bits_per_pixel)
{
    return eng_tst_camera_at_cmd(interface_eng_tst_camera_init,camera_id,preview_width,preview_height,bits_per_pixel);
}

int eng_tst_camera_deinit(int camera_id)
{
    return eng_tst_camera_at_cmd(interface_eng_tst_camera_deinit ,camera_id,0,0,0);
}

int eng_camera_close(int camera_id)
{
    return eng_tst_camera_at_cmd(interface_eng_camera_close,camera_id,0,0,0);
}

int test_camera_start_comm_ex(void)
{
    int rtn = RL_FAIL;
    int preview_width = gr_fb_width();
    int preview_height = gr_fb_height() - ui_getTitleHeight() - ui_getButtonHeight();

    if(preview_width > MAX_PREVIEW_W)
        preview_width = MAX_PREVIEW_W;
    if(preview_height > MAX_PREVIEW_H)
        preview_height = MAX_PREVIEW_H;

    LOGD("camera id:%d, preview size:(%d,%d)", camera_id, preview_width, preview_height);

    //Register callback
    LOGD("FT: add callback: %x", camera_draw_cb);
    chnl_fw_ptrFunc_add(TEXT_CAMERA_FACTORY_CALL, (void **)(&camera_draw_cb));

    while(1)
    {
        if(eng_tst_camera_init(camera_id, preview_width, preview_height, bits_per_pixel))
        {
            LOGE("fail to call eng_test_camera_init");
            rtn = RL_FAIL;

            ui_set_color(CL_RED);
            ui_show_text(3, 0, TEXT_PREVIEW_ERROR);
            gr_flip();
            sleep(1);
            break;
        }

        //only back camera test open flashlight
        if(camera_id == 0)
        {
            pthread_t flash_t;

            LOGD("open flash success.");
            if(test_case_support(CASE_TEST_FLASH))
            {
                flashthread_run = true;
                pthread_create(&flash_t, NULL, flash_thread, NULL);
            }

            rtn = ui_handle_button(TEXT_PASS, TEXT_CAPTURE, TEXT_FAIL);
            eng_tst_camera_deinit(camera_id);

            if(test_case_support(CASE_TEST_FLASH))
            {
                flashthread_run = false;
                pthread_join(flash_t, NULL);
            }
        }
        else
        {
            rtn = ui_handle_button(TEXT_PASS, TEXT_CAPTURE, TEXT_FAIL);
            eng_tst_camera_deinit(camera_id);
        }

        if(RL_NEXT_PAGE == rtn)     //press capture button
        {
            LOGD("switch to capture");
            sleep(1);
            continue;
        }
        else if(RL_FAIL == rtn || RL_PASS == rtn)
        {
            break;
        }
        else
        {
            rtn = RL_FAIL;
            break;
        }
    }

    eng_camera_close(camera_id);

    return rtn;
}

//back camera test
int test_bcamera_start_extern(void)
{
        int  rtn = RL_FAIL;

    camera_id = 0;
    LOGD("enter back camera test, cameraID= %d",camera_id);
    ui_fill_locked();
    ui_show_title(MENU_TEST_BCAMERA);

    if(temprgb)
        free(temprgb);
    temprgb = (unsigned char *)malloc(gr_fb_width()*gr_fb_height()*bits_per_pixel);
    rtn = test_camera_start_comm_ex();
    free(temprgb);
    temprgb = NULL;

    save_result(CASE_TEST_BCAMERA, rtn);
    if(test_case_support(CASE_TEST_FLASH))
        save_result(CASE_TEST_FLASH, rtn);

    return rtn;
}

//front camera test
int test_fcamera_start_extern(void)
{
    int  rtn = RL_FAIL;

    camera_id = 1;
    LOGD("enter front camera test, cameraID= %d",camera_id);
    ui_fill_locked();
    ui_show_title(MENU_TEST_FCAMERA);

    if(temprgb)
        free(temprgb);
    temprgb = (unsigned char *)malloc(gr_fb_width()*gr_fb_height()*bits_per_pixel);
    rtn = test_camera_start_comm_ex();
    free(temprgb);
    temprgb = NULL;

    save_result(CASE_TEST_FCAMERA, rtn);

    return rtn;
}

//rear auxiliary camera
int test_acamera_start_extern(void)
{
    int  rtn = RL_FAIL;

    camera_id = 2;
    LOGD("enter rear auxiliary camera test, cameraID= %d",camera_id);
    ui_fill_locked();
    ui_show_title(MENU_TEST_ACAMERA);

    if(temprgb)
        free(temprgb);
    temprgb = (unsigned char *)malloc(gr_fb_width()*gr_fb_height()*bits_per_pixel);
    rtn = test_camera_start_comm_ex();
    free(temprgb);
    temprgb = NULL;

    save_result(CASE_TEST_ACAMERA, rtn);

    return rtn;
}

//front auxiliary camera
int test_facamera_start_extern(void)
{
    int  rtn = RL_FAIL;

    camera_id = 3;
    LOGD("enter front auxiliary camera test, cameraID= %d",camera_id);
    ui_fill_locked();
    ui_show_title(MENU_TEST_FACAMERA);

    if(temprgb)
        free(temprgb);
    temprgb = (unsigned char *)malloc(gr_fb_width()*gr_fb_height()*bits_per_pixel);
    rtn = test_camera_start_comm_ex();
    free(temprgb);
    temprgb = NULL;


    save_result(CASE_TEST_FACAMERA, rtn);

    return rtn;
}

