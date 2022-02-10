#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <cutils/log.h>

#include "common.h"
#include "chnmgr.h"

int chnl_at_send(char *buf, int len, char *rsp, int rsp_len)
{
    struct list_head* pHead;
    eng_modules *modules_list = NULL;
    struct list_head *list_find;
    int rlen = -1;
    pHead = chnl_get_call_list_head();
    if (pHead == NULL)
    {
        return -1;
    }

    list_for_each(list_find, pHead)
    {
        modules_list = list_entry(list_find, eng_modules, node);
        if ((0 != strlen(modules_list->callback.at_cmd)) &&
        (0 == strncmp((const char *) buf, (const char *)(modules_list->callback.at_cmd), strlen(modules_list->callback.at_cmd))))
        { // at command
            LOGD("%s: Dymic CMD=%s finded\n", __FUNCTION__, modules_list->callback.at_cmd);
            if (NULL != modules_list->callback.eng_linuxcmd_func) {
                rlen = modules_list->callback.eng_linuxcmd_func((char *)buf, rsp);
                break;
            }
            else
            {
                LOGE("%s: Dymic eng_linuxcmd_func == NULL\n", __FUNCTION__);
                break;
            }
        }
        else
        {
            continue;
        }
    }
    LOGD("chnl_at_send ret=%d", rlen);
    return rlen;
}

