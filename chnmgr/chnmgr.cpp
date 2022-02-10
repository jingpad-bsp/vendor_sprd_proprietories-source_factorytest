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

#include "chnmgr.h"
#include "common.h"
#include "at.h"

#define AT_LEN_MAX 1024
typedef void (*REGISTER_FW_FUNC)(struct fw_callback *reg);

extern int sendATCmd(int phoneId, char* cmd, char* buf, unsigned int buf_len, int wait);

struct list_head eng_head[3];

int chnl_init()
{
    int ret = -1;
    ret = eng_modules_load(eng_head);
    if (ret == -1)
    {
        return -1;
    }

    return 0;
}

void chnl_uninit(){
}

struct list_head* chnl_get_call_list_head(){
    return &eng_head[0];
}

struct list_head* chnl_get_callback_list_head(){
    return &eng_head[1];
}

struct list_head* chnl_get_callback_ptrFunc_head(){
    return &eng_head[2];
}

int chnl_send(CHNL_TYPE type, char *buf, int len, char *rsp, int rsp_len)
{
    if (type == CHNL_DIAG)
    {
        return chnl_diag_send(buf, len, rsp, rsp_len);
    }
    else if (type == CHNL_AT)
    {
        return chnl_at_send(buf, len, rsp, rsp_len);
    }
    else
    {
        LOGE("chnl_send failed");
        return -1;
    }
}

int chnl_send_at_interface(char *buf, int len)
{
    char rsp[AT_LEN_MAX] = {0};
    return chnl_send(CHNL_AT,buf, len, rsp, sizeof(rsp));
}

int chnl_send_diag_interface(char *buf, int len)
{
    char rsp[AT_LEN_MAX] = {0};
    return chnl_send(CHNL_DIAG,buf, len, rsp, sizeof(rsp));
}

//select callback base the module_desc
int chnl_select_callback(char *module_desc,const void** ptrFunc)
{
    if( module_desc == NULL ){
        return -1;
    }

    *ptrFunc = (void *)chnl_fw_ptrFunc_find(module_desc);
    return ptrFunc==NULL?-1:0;
}

int chnl_callback(char* module_desc)
{
    struct list_head* pHead;
    struct list_head *list_find;
    struct fw_callback reg;
    eng_callbacks *callback_list = NULL;

    if( module_desc != NULL)
    {
        pHead = chnl_get_callback_list_head();
        if (pHead == NULL)
            return -1;

        list_for_each(list_find, pHead)
        {
            callback_list = list_entry(list_find, eng_callbacks, node);
            if ((0 != strlen(callback_list->callback.moudel_des)) &&
            (0 == strncmp((const char *) module_desc, (const char *)(callback_list->callback.moudel_des), strlen(callback_list->callback.moudel_des))))
            {
                LOGD("callback function desc=[%s] found", callback_list->callback.moudel_des);
                if (NULL != callback_list->callback.eng_cb)
                {
                    reg.ptfQueryInterface = chnl_select_callback;
                    callback_list->callback.eng_cb(&reg);
                    break;
                }
                else
                    LOGE("error, callback function is NULL, %d", callback_list->callback.eng_cb);
            }
        }
        return 0;
    }
    else
        return -1;

}

//check supported cmder or callback from the list
int chnl_is_support(char *cmd)
{
    struct list_head* call_head;
    struct list_head* callback_head;
    eng_modules* modules_list = NULL;
    eng_callbacks* callback_list = NULL;
    struct list_head* list_find;

    call_head = chnl_get_call_list_head();
    if (call_head != NULL)
    {
        list_for_each(list_find, call_head)
        {
            modules_list = list_entry(list_find, eng_modules, node);
            if ((0 != strlen(modules_list->callback.at_cmd)) &&
            (0 == strncmp((const char *) cmd, (const char *)(modules_list->callback.at_cmd), strlen(modules_list->callback.at_cmd))))
            {
                LOGD("%s found, supported", modules_list->callback.at_cmd);
                return 1;
            }
        }
        LOGD("%s is not supported", cmd);
    }

    return 0;
}

int chnl_is_support_callback(char *desc)
{
    struct list_head* callback_head;
    eng_callbacks* callback_list = NULL;
    struct list_head* list_find;

    callback_head = chnl_get_callback_list_head();
    if (callback_head != NULL)
    {
        list_for_each(list_find, callback_head)
        {
            callback_list = list_entry(list_find, eng_callbacks, node);
            if ((0 != strlen(callback_list->callback.moudel_des)) &&
            (0 == strncmp((const char *) desc, (const char *)(callback_list->callback.moudel_des), strlen(callback_list->callback.moudel_des))))
            {
                LOGD("callback function desc=[%s] found, supported", callback_list->callback.moudel_des);
                return 1;
            }
        }
        LOGD("callback function desc=[%s] is not supported", desc);
    }

    return 0;
}

int chnl_fw_ptrFunc_add(char *desc, void **ptrFunc)
{
    struct list_head* ptrFunc_head;
    eng_fw_ptrFunc* ptrFunc_list = NULL;
    struct list_head* list_find;
    int find=0;
    int *pdest = NULL;

    LOGD("chnl_fw_ptrFunc_add: ptrFunc = %x, *ptrFunc = %x, ", ptrFunc, *ptrFunc);
    ptrFunc_head = chnl_get_callback_ptrFunc_head();
    if (ptrFunc_head != NULL)
    {
        list_for_each(list_find, ptrFunc_head)
        {
            ptrFunc_list = list_entry(list_find, eng_fw_ptrFunc, node);
            if ((0 != strlen(ptrFunc_list->fw_ptrFunc.func_des)) &&
            (0 == strncmp((const char *)desc, (const char *)(ptrFunc_list->fw_ptrFunc.func_des), strlen(ptrFunc_list->fw_ptrFunc.func_des))))
            {
                LOGD("fw function desc=[%s] exist, return", ptrFunc_list->fw_ptrFunc.func_des);
                return -1;
            }
        }
        LOGD("callback function desc=[%s] not find, add...", desc);

        eng_fw_ptrFunc *fw_ptr = (eng_fw_ptrFunc*)malloc(sizeof(eng_fw_ptrFunc));
        if (fw_ptr == NULL)
        {
            LOGE("%s malloc fail...",__FUNCTION__);
            return -1;
        }

        memset(fw_ptr,0,sizeof(eng_fw_ptrFunc));
        sprintf(fw_ptr->fw_ptrFunc.func_des, "%s", desc);
        fw_ptr->fw_ptrFunc.ptrFunc = (int)ptrFunc;

        list_add_tail(&fw_ptr->node, ptrFunc_head);
        chnl_callback(desc);
    }

    return 0;
}

void chnl_fw_ptrFunc_remove(char *desc)
{
    struct list_head* ptrFunc_head;
    eng_fw_ptrFunc* ptrFunc_list = NULL;
    struct list_head* list_find;
    int find=0;

    LOGD("chnl_fw_ptrFunc_remove");
    ptrFunc_head = chnl_get_callback_ptrFunc_head();
    if (ptrFunc_head != NULL)
    {
        list_for_each(list_find, ptrFunc_head)
        {
            ptrFunc_list = list_entry(list_find, eng_fw_ptrFunc, node);
            if ((0 != strlen(ptrFunc_list->fw_ptrFunc.func_des)) &&
            (0 == strncmp((const char *)desc, (const char *)(ptrFunc_list->fw_ptrFunc.func_des), strlen(ptrFunc_list->fw_ptrFunc.func_des))))
            {
                LOGD("fw function desc=[%s] exist, remove", ptrFunc_list->fw_ptrFunc.func_des);
                list_remove(&(ptrFunc_list->node));
                free(ptrFunc_list);
            }
        }
    }
}

int chnl_fw_ptrFunc_find(char *desc)
{
    struct list_head* ptrFunc_head;
    eng_fw_ptrFunc* ptrFunc_list = NULL;
    struct list_head* list_find;
    int find=0;

    LOGD("chnl_fw_ptrFunc_find");
    ptrFunc_head = chnl_get_callback_ptrFunc_head();
    if (ptrFunc_head != NULL)
    {
        list_for_each(list_find, ptrFunc_head)
        {
            ptrFunc_list = list_entry(list_find, eng_fw_ptrFunc, node);
            if ((0 != strlen(ptrFunc_list->fw_ptrFunc.func_des)) &&
            (0 == strncmp((const char *)desc, (const char *)(ptrFunc_list->fw_ptrFunc.func_des), strlen(ptrFunc_list->fw_ptrFunc.func_des))))
            {
                LOGD("fw function desc=[%s] find ", ptrFunc_list->fw_ptrFunc.func_des);
                return ptrFunc_list->fw_ptrFunc.ptrFunc;
            }
        }
    }

    return 0;
}
