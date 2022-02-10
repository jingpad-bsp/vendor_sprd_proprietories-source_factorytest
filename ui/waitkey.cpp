/*
 * Copyright (C) 2014 The CyanogenMod Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cutils/properties.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>

#include "minui.h"
#include "common.h"
#include "waitkey.h"
#include "factorytest.h"
#include "testitem.h"
#include "ui.h"
#include "events.h"
#include "dev_header.h"
#include "parse_config.h"

#ifndef ABS_MT_ANGLE
#define ABS_MT_ANGLE 0x38
#endif

/* Handle axis swap */
#ifdef BOARD_RECOVERY_SWIPE_SWAPXY
#define EV_CODE_POS_X  ABS_MT_POSITION_Y
#define EV_CODE_POS_Y  ABS_MT_POSITION_X
#else
#define EV_CODE_POS_X  ABS_MT_POSITION_X
#define EV_CODE_POS_Y  ABS_MT_POSITION_Y
#endif

#define KEY_SWIPE_UP   KEY_VOLUMEUP
#define KEY_SWIPE_DOWN KEY_VOLUMEDOWN

#define BOARD_RECOVERY_CHAR_HEIGHT 20

static input_device input_devices[MAX_INPUT_DEVS];

// Defaults, will be changed based on dpi in calibrate_swipe()
static int min_swipe_dx = 100;
static int min_swipe_dy = 80;

//if no devices plug in, the below test item will not start, not display "PASS" button
//charge test, USB plug in state
//OTG test, OTG plug in state
//FM test & headset test, headset line plug in state
int plugin_state = 0;

// Key event input queue
static pthread_mutex_t key_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t key_queue_cond = PTHREAD_COND_INITIALIZER;
static int key_queue[256], key_queue_len = 0;
static int headset_key_pressed;
static int headset_key_test;
static volatile char key_pressed[KEY_MAX + 1];

struct test_key key_info[128];
int key_cnt = 0;

struct test_key* getKeyInfo()
{
    return key_info;
}

int getKeyCnt()
{
    return key_cnt;
}

void setKeyCnt(int value)
{
    key_cnt = value;
}

static void show_event(int fd, struct input_event *ev)
{
    char typebuf[40];
    char codebuf[40];
    const char *evtypestr = NULL;
    const char *evcodestr = NULL;
    static int count = 0;

    snprintf(typebuf, sizeof(typebuf), "0x%04x", ev->type);
    evtypestr = typebuf;

    snprintf(codebuf, sizeof(codebuf), "0x%04x", ev->code);
    evcodestr = codebuf;
    count++;
    switch (ev->type)
    {
    case EV_SYN:
        evtypestr = "EV_SYN";
        switch (ev->code)
        {
        case SYN_REPORT:
            evcodestr = "SYN_REPORT";
            break;
        case SYN_MT_REPORT:
            evcodestr = "SYN_MT_REPORT";
            break;
        }
        break;
    case EV_KEY:
        evtypestr = "EV_KEY";
        switch (ev->code)
        {
        case KEY_HOME: /* 102 */
            evcodestr = "KEY_HOME";
            break;
        case KEY_VOLUMEUP: /* 73 */
            evcodestr = "KEY_VOLUMEUP";
            break;
        case KEY_VOLUMEDOWN: /* 72 */
            evcodestr = "KEY_VOLUMEDOWN";
            break;
        case KEY_CAMERA: /* 0XD4 */
            evcodestr = "KEY_CAMERA";
            break;
        case KEY_POWER: /* 116 */
            evcodestr = "KEY_POWER";
            break;
        case KEY_MENU: /* 139 */
            evcodestr = "KEY_MENU";
            break;
        case KEY_BACK: /* 158 */
            evcodestr = "KEY_BACK";
            break;
        case KEY_HOMEPAGE: /* 172 */
            evcodestr = "KEY_HOMEPAGE";
            break;
        case KEY_SEARCH: /* 217 */
            evcodestr = "KEY_SEARCH";
            break;
        case BTN_TOOL_FINGER: /* 0x145 */
            evcodestr = "BTN_TOOL_FINGER";
            break;
        case BTN_TOUCH: /* 0x14a */
            evcodestr = "BTN_TOUCH";
            break;
        }
        break;
    case EV_REL:
        evtypestr = "EV_REL";
        switch (ev->code)
        {
        case REL_X:
            evcodestr = "REL_X";
            break;
        case REL_Y:
            evcodestr = "REL_Y";
            break;
        case REL_Z:
            evcodestr = "REL_Z";
            break;
        }
        break;
    case EV_ABS:
        evtypestr = "EV_ABS";
        switch (ev->code)
        {
        case ABS_MT_TOUCH_MAJOR:
            evcodestr = "ABS_MT_TOUCH_MAJOR";
            break;
        case ABS_MT_TOUCH_MINOR:
            evcodestr = "ABS_MT_TOUCH_MINOR";
            break;
        case ABS_MT_WIDTH_MAJOR:
            evcodestr = "ABS_MT_WIDTH_MAJOR";
            break;
        case ABS_MT_WIDTH_MINOR:
            evcodestr = "ABS_MT_WIDTH_MINOR";
            break;
        case ABS_MT_ORIENTATION:
            evcodestr = "ABS_MT_ORIENTATION";
            break;
        case ABS_MT_POSITION_X:
            evcodestr = "ABS_MT_POSITION_X";
            break;
        case ABS_MT_POSITION_Y:
            evcodestr = "ABS_MT_POSITION_Y";
            break;
        case ABS_MT_TRACKING_ID:
            evcodestr = "ABS_MT_TRACKING_ID";
            break;
        case ABS_MT_PRESSURE:
            evcodestr = "ABS_MT_PRESSURE";
            break;
        case ABS_MT_ANGLE:
            evcodestr = "ABS_MT_ANGLE";
            break;
        }
        break;
    }
    SPRD_DBG("touchevent: fd=%d, type=%s, code=%s, val=%d count=%d", fd, evtypestr, evcodestr, ev->value, count);
}

static int setup_virtual_keys(input_device *dev)
{
    char name[256] = "unknown";
    char vkpath[PATH_MAX] = "/sys/board_properties/virtualkeys.";
    char tempath[PATH_MAX] ={0};
    char vir_key[1024] = {0};
    char item_key[1024] = {0};
    char temp_key[1024] = {0};
    int bl = 0;
    int i, j;
    FILE *f;
    int maxline = 40 * MAX_VIRTUAL_KEYS;
    char buf[MAX_VIRTUAL_KEYS][maxline];
    char tmp[maxline];
    int height = gr_fb_height();
    int width = gr_fb_width();

#ifdef SPRD_VIRTUAL_TOUCH
    for(i = 0; i < ui_getMaxItems(); i++)
    {
        if(0 == i)
            snprintf(item_key, sizeof(item_key), "0x01:%d:%d:%d:%d:%d", KEY_VIR_ITEMS + i, width / 2, i*ui_getItemHeight()+ui_getTitleHeight()+ui_getItemHeight()/2, width, ui_getItemHeight());
        else
        {
            memcpy(temp_key, item_key, sizeof(item_key));
            snprintf(item_key, sizeof(item_key), "%s:0x01:%d:%d:%d:%d:%d", temp_key, KEY_VIR_ITEMS + i, width / 2, i*ui_getItemHeight()+ui_getTitleHeight()+ui_getItemHeight()/2, width, ui_getItemHeight());
        }
    }
    snprintf(vir_key, sizeof(vir_key), "0x01:%d:%d:%d:%d:%d:0x01:%d:%d:%d:%d:%d:0x01:%d:%d:%d:%d:%d:%s", KEY_VIR_PASS, ui_getButtonWidth()/2, height - ui_getButtonHeight()/2,
        ui_getButtonWidth(), ui_getButtonHeight(), KEY_VIR_FAIL, width - ui_getButtonWidth()/2, height - ui_getButtonHeight()/2, ui_getButtonWidth(), ui_getButtonHeight(), KEY_VIR_NEXT_PAGE, width / 2,
        height - ui_getButtonHeight()/2, ui_getButtonWidth(), ui_getButtonHeight(), item_key);
#endif

    for(i = 0; i < getKeyCnt(); i++)
    {
        if(!strcmp(getKeyInfo()[i].key_name, "Menu"))
        {
            if (ioctl(dev->fd, EVIOCGNAME(sizeof(name)), name) < 0)
            {
                LOGE("[Touch] Could not find virtual keys device name");
                return -1;
            }
            memcpy(tempath, vkpath, sizeof(vkpath));
            snprintf(vkpath, PATH_MAX, "%s%s", tempath, name);
            SPRD_DBG("[Touch] Loading virtual keys file: %s", vkpath);

            f = fopen(vkpath, "r");
            if (f != NULL)
            {
                while (fgets(buf[bl], maxline, f) && bl < MAX_VIRTUAL_KEYS)
                {
                    if (buf[bl][0] == '#')
                        continue;
                    if (buf[bl][strlen(buf[bl]) - 1] == '\n')
                        buf[bl][strlen(buf[bl]) - 1] = '\0';

                    bl++;
                }
                fclose(f);
            }
            else
            {
                LOGE("[Touch] Could not open virtual keys file");
                //return -1;
            }
            break;
        }
    }
    memcpy(buf[bl], vir_key, strlen(vir_key));
    bl++;
    // Set the virtual key parameters
    int count = 0;
    char *token, *endp;
    const char *sep = ":";
    for (j = 0; j < bl; j++)
    {
        snprintf(tmp, maxline, "%s", buf[j]);
        token = strtok(tmp, sep);
        while (token && (count / 6) < MAX_VIRTUAL_KEYS)
        {
            if (count % 6 == 0 && strcmp("0x01", token) != 0)
            {
                for (i = 0; i < 6; i++)
                    token = strtok(NULL, sep);
                continue;
            }
            if (count % 6 == 1)
                dev->virtual_keys[count / 6].keycode = (int)strtol(token, &endp, 10);
            if (count % 6 == 2)
                dev->virtual_keys[count / 6].center_x = (int)strtol(token, &endp, 10);
            if (count % 6 == 3)
                dev->virtual_keys[count / 6].center_y = (int)strtol(token, &endp, 10);
            if (count % 6 == 4)
                dev->virtual_keys[count / 6].width = (int)strtol(token, &endp, 10);
            if (count % 6 == 5)
                dev->virtual_keys[count / 6].height = (int)strtol(token, &endp, 10);

            token = strtok(NULL, sep);
            count++;
        }
    }
    dev->virtual_key_count = count / 6;

#if DEBUG_TOUCH_EVENTS
    for (i = 0; i < dev->virtual_key_count; i++)
    {
        SPRD_DBG("[Touch] Virtual key: code=%d, x=%d, y=%d, width=%d, height=%d",
                 dev->virtual_keys[i].keycode, dev->virtual_keys[i].center_x,
                 dev->virtual_keys[i].center_y, dev->virtual_keys[i].width,
                 dev->virtual_keys[i].height);
    }
#endif

    return 0;
}

static void calibrate_swipe()
{
    min_swipe_dx = gr_fb_width() / 4;
    min_swipe_dy = 3 * BOARD_RECOVERY_CHAR_HEIGHT;
}

static int calibrate_touch(input_device *dev)
{
    struct input_absinfo info;

    memset(&info, 0, sizeof(info));
    if (ioctl(dev->fd, EVIOCGABS(EV_CODE_POS_X), &info) == 0)
    {
        dev->touch_min.x = info.minimum;
        dev->touch_max.x = info.maximum;
        dev->touch_pos.x = info.value;
    }
    memset(&info, 0, sizeof(info));
    if (ioctl(dev->fd, EVIOCGABS(EV_CODE_POS_Y), &info) == 0)
    {
        dev->touch_min.y = info.minimum;
        dev->touch_max.y = info.maximum;
        dev->touch_pos.y = info.value;
    }

    if (dev->touch_min.x == dev->touch_max.x
    || dev->touch_min.y == dev->touch_max.y)
        return 0; // Probably not a touch device

    return 1; // Success
}

static void handle_press(input_device *dev, struct input_event *ev)
{
    dev->touch_start = dev->touch_pos;
    dev->touch_track = dev->touch_pos;
    dev->in_touch = 1;
    dev->in_swipe = 0;
}

static void handle_release(input_device *dev, struct input_event *ev)
{
    int i;
    int dx, dy;

    if (dev->in_swipe)
    {
        dx = dev->touch_pos.x - dev->touch_start.x;
        dy = dev->touch_pos.y - dev->touch_start.y;
        if (abs(dx) > abs(dy) && abs(dx) > min_swipe_dx)
        {
            ev->type = EV_KEY;
            ev->code = (dx > 0 ? KEY_ENTER : KEY_BACK);
            ev->value = 2;
        }
        else
        {
            /* Vertical swipe, handled real-time */
        }
    }
    else
    {
        // Check if virtual key pressed
        for (i = 0; i < dev->virtual_key_count; i++)
        {
            dx = dev->virtual_keys[i].center_x - dev->touch_pos.x;
            dy = dev->virtual_keys[i].center_y - dev->touch_pos.y;
            if (abs(dx) < dev->virtual_keys[i].width / 2 && abs(dy) < dev->virtual_keys[i].height / 2)
            {
            #if DEBUG_TOUCH_EVENTS
                SPRD_DBG("Virtual key: code=%d, touch-x=%d, touch-y=%d",dev->virtual_keys[i].keycode, dev->touch_pos.x, dev->touch_pos.y);
            #endif
                ev->type = EV_KEY;
                ev->code = dev->virtual_keys[i].keycode;
                ev->value = 2;
                dev->touch_pos.x = 0;
                dev->touch_pos.y = 0;
            }
        }
    }

    dev->in_touch = 0;
    dev->in_swipe = 0;
    ui_clear_key_queue();
}

static void handle_gestures(input_device *dev, struct input_event *ev)
{
    int dx = dev->touch_pos.x - dev->touch_start.x;
    int dy = dev->touch_pos.y - dev->touch_start.y;
    if (abs(dx) > abs(dy))
    {
        if (abs(dx) > min_swipe_dx)
        {
            /* Horizontal swipe, handle it on release */
            dev->in_swipe = 1;
        }
    }
    else
    {
        dy = dev->touch_pos.y - dev->touch_track.y;
        if (abs(dy) > min_swipe_dy)
        {
            dev->in_swipe = 1;
            dev->touch_track = dev->touch_pos;
            ev->type = EV_KEY;
            ev->code = (dy < 0) ? KEY_SWIPE_UP : KEY_SWIPE_DOWN;
            ev->value = 2;
        }
    }
}
    /*
     * Type A device release:
     *   1. Lack of position update
     *   2. BTN_TOUCH | ABS_PRESSURE | SYN_MT_REPORT
     *   3. SYN_REPORT
     *
     * Type B device release:
     *   1. ABS_MT_TRACKING_ID == -1 for "first" slot
     *   2. SYN_REPORT
     */
static void process_syn(input_device *dev, struct input_event *ev)
{
    if (ev->code == SYN_MT_REPORT)
    {
        if (!dev->in_touch && (dev->saw_pos_x && dev->saw_pos_y))
        {
        #if DEBUG_TOUCH_EVENTS
            SPRD_DBG("[Touch] handle_press(fd=%d): type A", dev->fd);
        #endif
            handle_press(dev, ev);
        }
        dev->saw_mt_report = 1;
        return;
    }

    if (ev->code == SYN_REPORT)
    {
        if (dev->in_touch)
        {
            //handle_gestures(dev, ev); //del by qing
        }
        else
        {
            if (dev->saw_tracking_id)
            {
            #if DEBUG_TOUCH_EVENTS
                SPRD_DBG("[Touch] handle_press(fd=%d): type B", dev->fd);
            #endif
                handle_press(dev, ev);
            }
        }

        /* Detect release */
        if (dev->saw_mt_report)
        {
            if (!dev->saw_pos_x && !dev->saw_pos_y)
            {
            #if DEBUG_TOUCH_EVENTS
                SPRD_DBG("[Touch] handle_release(fd=%d): type A", dev->fd);
            #endif
                handle_release(dev, ev);
                dev->slot_first = 0;
            }
        }
        else
        {
            if (dev->saw_tracking_id && dev->slot_current == dev->slot_first && dev->tracking_id == -1)
            {
            #if DEBUG_TOUCH_EVENTS
                SPRD_DBG("[Touch] handle_release(fd=%d): type B", dev->fd);
            #endif
                handle_release(dev, ev);
                dev->slot_first = 0;
            }
        }

        dev->saw_pos_x = 0;
        dev->saw_pos_y = 0;
        dev->saw_mt_report = 0;
        dev->saw_tracking_id = 0;
    }
}

static void process_abs(input_device *dev, struct input_event *ev)
{
    if (ev->code == ABS_MT_SLOT)
    {
        dev->slot_current = ev->value;
        if (dev->slot_first == -1)
            dev->slot_first = ev->value;

        return;
    }
    if (ev->code == ABS_MT_TRACKING_ID)
    {
        /*
         * Some devices send an initial ABS_MT_SLOT event before switching
         * to type B events, so discard any type A state related to slot.
         */
        dev->saw_tracking_id = 1;
        dev->slot_first = 0;
        dev->slot_current = 0;

        if (ev->value != dev->tracking_id)
        {
            dev->tracking_id = ev->value;
            if (dev->tracking_id < 0)
                dev->slot_count_active--;
            else
                dev->slot_count_active++;
        }
        return;
    }
    /*
     * For type A devices, we "lock" onto the first coordinates by ignoring
     * position updates from the time we see a SYN_MT_REPORT until the next
     * SYN_REPORT
     *
     * For type B devices, we "lock" onto the first slot seen until all slots
     * are released
     */
    if (dev->slot_count_active == 0)
    {
        /* type A */
        if (dev->saw_pos_x && dev->saw_pos_y)
        {
            return;
        }
    }
    else
    {
        /* type B */
        if (dev->slot_current != dev->slot_first)
            return;
    }
    if (ev->code == EV_CODE_POS_X)
    {
        dev->saw_pos_x = 1;
        dev->touch_pos.x = ev->value * gr_fb_width()
                           / (dev->touch_max.x - dev->touch_min.x);
    }
    else if (ev->code == EV_CODE_POS_Y)
    {
        dev->saw_pos_y = 1;
        dev->touch_pos.y = ev->value * gr_fb_height()
                           / (dev->touch_max.y - dev->touch_min.y);
    }
}

static int one_time_initd = 0;
static void one_time_init()
{
    if (one_time_initd)
        return;
    one_time_initd = 1;

    int i;
    for (i = 0; i < MAX_INPUT_DEVS; i++)
        input_devices[i].fd = -1;

    calibrate_swipe();
}
//extern struct dirent *testde;

void touch_handle_input(int fd, struct input_event *ev)
{
    int i;
    input_device *dev = NULL;
    static int count = 0;

#if DEBUG_TOUCH_EVENTS
    show_event(fd, ev);
#endif

    one_time_init();
    count++;

    SPRD_DBG("touchevent %d", count);
    for (i = 0; i < MAX_INPUT_DEVS; i++)
    {
        SPRD_DBG("touchevent input_devices[%d].fd=%d", i, input_devices[i].fd);
        if (fd == input_devices[i].fd)
        {
            dev = &input_devices[i];
            break;
        }
        if (input_devices[i].fd == -1)
        {
            dev = &input_devices[i];
            dev->fd = fd;
            dev->tracking_id = -1;
            dev->touch_min.x = 0;
            dev->touch_max.x = 0;
            dev->touch_min.y = 0;
            dev->touch_max.y = 0;
            dev->touch_calibrated = calibrate_touch(dev);
            if(dev->touch_calibrated)
                setup_virtual_keys(dev);

            SPRD_DBG("touchevent %s", "testtest");
            break;
        }
    }

    if (!dev)
    {
        SPRD_DBG("[Touch] Maximum # of input devices reached");
        return;
    }

    if(!dev->touch_calibrated)
        return;
    //SPRD_DBG("touchevent dev->fd=%d\n",dev->fd);

    switch (ev->type)
    {
    case EV_SYN:
        process_syn(dev, ev);
        break;
    case EV_ABS:
        process_abs(dev, ev);
        break;
    }
}

// Reads input events, handles special hot keys, and adds to the key queue.
void *input_thread(void *)
{
    int rel_sum = 0;
    int fake_key = 0;
    int fd = -1;

    pthread_detach(pthread_self()); 	//free by itself
    for (;;)
    {
        // wait for the next key event
        struct input_event ev;
        do
        {
            fd = ev_get(&ev, 0);
            SPRD_DBG("eventtype %d,eventcode %d,eventvalue %d", ev.type, ev.code, ev.value);
            if(fd != -1)
                touch_handle_input(fd, &ev);

            if (ev.type == EV_SYN)
                continue;
            else if (ev.type == EV_REL)
            {
                if (ev.code == REL_Y)
                {
                    // accumulate the up or down motion reported by
                    // the trackball.  When it exceeds a threshold
                    // (positive or negative), fake an up/down
                    // key event.
                    rel_sum += ev.value;
                    if (rel_sum > 3)
                    {
                        fake_key = 1;
                        ev.type = EV_KEY;
                        ev.code = KEY_DOWN;
                        ev.value = 1;
                        rel_sum = 0;
                    }
                    else if (rel_sum < -3)
                    {
                        fake_key = 1;
                        ev.type = EV_KEY;
                        ev.code = KEY_UP;
                        ev.value = 1;
                        rel_sum = 0;
                    }
                }
            }
            else
                rel_sum = 0;
        }
        while (ev.type != EV_KEY || ev.code > KEY_MAX);

        pthread_mutex_lock(&key_queue_mutex);
        if (!fake_key)
        {
            // our "fake" keys only report a key-down event (no
            // key-up), so don't record them in the key_pressed
            // table.
            key_pressed[ev.code] = ev.value;
            SPRD_DBG("ev.value=%d", ev.value);
        }
        fake_key = 0;
        const int queue_max = sizeof(key_queue) / sizeof(key_queue[0]);
        if (ev.value > 0 && key_queue_len < queue_max)
        {
            if( headset_key_test == 1 && ev.code == SPRD_HEADSET_KEY )
            {
                headset_key_pressed = 1;
            }
            else
            {
                key_queue[key_queue_len++] = ev.code;
                pthread_cond_signal(&key_queue_cond);
            }
        }
        pthread_mutex_unlock(&key_queue_mutex);
    }
    return NULL;
}

int wait_headset_key()
{
    int times = 1000;   //times out, 10ms
    headset_key_test = 1;

    while(times >= 0)
    {
        if(headset_key_pressed)
        {
            headset_key_pressed = 0;
            headset_key_test = 0;
            return 1;
        }
        else
        {
            usleep(1000);
            times--;
        }
    }

    return 0;
}

int wait_key()
{
    int ret = 0;
    int key = -1;
    struct timespec ntime;
    ntime.tv_sec = time(NULL);
    ntime.tv_nsec = 0;
    pthread_mutex_lock(&key_queue_mutex);

    while (key_queue_len == 0)
    {
        ntime.tv_sec = ntime.tv_sec + 20;
        ret = pthread_cond_timedwait(&key_queue_cond, &key_queue_mutex, &ntime);
        if(ret == ETIMEDOUT)
            break;
    }

    if(0 == ret)
    {
        key = key_queue[0];
        memcpy(&key_queue[0], &key_queue[1], sizeof(int) * --key_queue_len);
    }
    else
        key = ret;

    pthread_mutex_unlock(&key_queue_mutex);

    return key;
}

void push_result(int result)
{
    pthread_mutex_lock(&key_queue_mutex);
    const int queue_max = sizeof(key_queue) / sizeof(key_queue[0]);
    if (key_queue_len < queue_max)
    {
        switch(result)
        {
        case RL_NA: //cancel
            key_queue[key_queue_len++] = KEY_BACK;
            break;
        case RL_FAIL: //fail
            key_queue[key_queue_len++] = KEY_POWER;
            break;
        case RL_PASS: //ok
            key_queue[key_queue_len++] = KEY_VOLUMEDOWN;
            break;
        default: //cancel
            //    key_queue[key_queue_len++] = KEY_BACK;
            break;
        }
        pthread_cond_signal(&key_queue_cond);
    }
    pthread_mutex_unlock(&key_queue_mutex);
}

void clear_key_queue()
{
    pthread_mutex_lock(&key_queue_mutex);
    key_queue_len = 0;
    pthread_mutex_unlock(&key_queue_mutex);
}

void setPlugState(int value)
{
    plugin_state = value;
}

int getPlugState()
{
    return plugin_state;
}

void input_init()
{
    pthread_t t;
    ev_init();
    pthread_create(&t, NULL, input_thread, NULL);
}
