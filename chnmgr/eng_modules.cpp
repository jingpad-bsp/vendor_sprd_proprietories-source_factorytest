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

#define MAX_LEN NAME_MAX //50

typedef void (*REGISTER_FUNC)(struct eng_callback *register_callback);
typedef void (*REGISTER_EXT_FUNC)(struct eng_callback *reg, int *num);
typedef void (*REGISTER_FW_FUNC)(struct fw_callback *reg);
//typedef int (*QUERYINTERFACE)(char *interfaceDesc, void** ptrFunc);

static const char *eng_modules_path = "/vendor/lib/npidevice";
static char callback_desc[100];

//Fix WRITE_TO_MAX
DYMIC_WRITETOPC_FUNC write_interface[WRITE_TO_MAX] = {
NULL,/*pc lte diag*/NULL,NULL,
NULL,NULL,NULL,
chnl_send_diag_interface,NULL,chnl_send_at_interface,/*send cmd so to so*/
NULL,NULL,NULL,NULL,NULL,NULL
};


eng_modules* get_eng_modules(struct eng_callback p, int type)
{
    LOGD("%s",__FUNCTION__);
    eng_modules *modules = (eng_modules*)malloc(sizeof(eng_modules));
    if (modules == NULL)
    {
        LOGE("%s malloc fail...",__FUNCTION__);
        return NULL;
    }

    if (type != -1 && type != p.type){
        LOGE("%s, type = %d, p.type= %d", __FUNCTION__, type, p.type);
     free(modules);
        return NULL;
    }

    memset(modules,0,sizeof(eng_modules));
    modules->callback.type = p.type;
    modules->callback.subtype = p.subtype;
    modules->callback.diag_ap_cmd = p.diag_ap_cmd;
    modules->callback.also_need_to_cp = p.also_need_to_cp;
    sprintf(modules->callback.at_cmd, "%s", p.at_cmd);
    modules->callback.eng_diag_func = p.eng_diag_func;
    modules->callback.eng_linuxcmd_func = p.eng_linuxcmd_func;
    modules->callback.eng_set_writeinterface_func = p.eng_set_writeinterface_func;
    return modules;
}

eng_callbacks* get_eng_callbacks(struct eng_callback_func p, int type)
{
    LOGD("%s",__FUNCTION__);
    eng_callbacks *callbacks = (eng_callbacks*)malloc(sizeof(eng_callbacks));
    if (callbacks == NULL)
    {
        LOGE("%s malloc fail...",__FUNCTION__);
        return NULL;
    }

    memset(callbacks,0,sizeof(eng_callbacks));

    sprintf(callbacks->callback.moudel_des, "%s", p.moudel_des);
    callbacks->callback.eng_cb = p.eng_cb;

    return callbacks;
}

eng_modules* get_eng_modules(struct eng_callback p)
{
    return get_eng_modules(p, -1);
}

eng_callbacks* get_eng_callbacks(struct eng_callback_func p)
{
    return get_eng_callbacks(p, -1);
}

int readFileList(const char *basePath, char **f_name)
{
    struct dirent *ptr;
    int filenum = 0;
    DIR *dir;

    if(basePath != NULL)
    {
        LOGD("open dir:%s", basePath);
        if ((dir = opendir(basePath)) == NULL)
        {
            LOGE("Open %s error...%s",basePath,dlerror());
            return 0;
        }

        while ((ptr = readdir(dir)) != NULL)
        {
            if(ptr->d_type == 8)
            {    ///file
                LOGD("d_name:%s/%s\n", basePath, ptr->d_name);
                f_name[filenum] = ptr->d_name;
                filenum++;
                LOGD("d_name:%s\n",*f_name);
            }
        }
        closedir(dir);
    }
    else
        LOGE("path is NULL");

    return filenum;
}

//call callback function at least 2 times
//first time, get the modules descript and save to callback list
int eng_callback_test(char *module_desc,const void** ptrFunc)
{
    if( module_desc != NULL)
    {
        LOGD("module_desc= %s", module_desc);
        strcpy(callback_desc, module_desc);
        return 0;
    }
    else
        LOGE("module_desc is NULL");

    return -1;
}

int eng_modules_load(struct list_head *head)
{
    REGISTER_FUNC eng_register_func = NULL;
    REGISTER_EXT_FUNC eng_register_ext_func = NULL;
    struct eng_callback register_callback;
    struct eng_callback_func register_callback_func;
    struct eng_callback register_arr[32];
    struct eng_callback_func register_callback_arr;
    struct eng_callback *register_arr_ptr = register_arr;
    REGISTER_FW_FUNC eng_register_fw_func;
    int register_num = 0;
    int register_cb_num = 0;
    int i = 0;
    char path[MAX_LEN] = " ";
    char lnk_path[MAX_LEN] = " ";
    int readsize = 0;
    DIR *dir;
    struct dirent *ptr;
    void *handler = NULL;
    eng_modules *modules;
    eng_modules *modules_ex;
    eng_callbacks *callbacks;

    struct fw_callback reg;
    reg.ptfQueryInterface = eng_callback_test;

    LOGD("load so and register test module...");

    INIT_LIST_HEAD(&head[0]);
    INIT_LIST_HEAD(&head[1]);
    INIT_LIST_HEAD(&head[2]);
    if ((dir = opendir(eng_modules_path)) == NULL)
    {
        LOGE("Open %s error...%s", eng_modules_path, dlerror());
        return 0;
    }

    while ((ptr = readdir(dir)) != NULL)
    {
        if (ptr->d_type == 8 || ptr->d_type == 10)          // file  , 10 == DT_LNK
        {
            snprintf(path, sizeof(path), "%s/%s", eng_modules_path, ptr->d_name);
            LOGD("find lib path: %s", path);

            if (ptr->d_type == 10)          //DT_LNK
            {
                memset(lnk_path, 0, sizeof(lnk_path));
                readsize = readlink(path, lnk_path, sizeof(lnk_path));
                LOGD("%s readsize:%d lnk_path:%s", path, readsize, lnk_path);

                if(readsize == -1)
                {
                    LOGE("ERROR! Fail to readlink!\n");
                    continue;
                }

                memset(path, 0, sizeof(path));
                strncpy(path, lnk_path, strlen(lnk_path));
            }

            if (access(path, R_OK) == 0)
            {
                handler = NULL;
                handler = dlopen(path, RTLD_LAZY);
                if (handler == NULL)
                {
                    LOGD("%s dlopen fail! %s", path, dlerror());
                }
                else
                {
                    eng_register_func = (REGISTER_FUNC)dlsym(handler, "register_this_module");
                    if (eng_register_func != NULL)
                    {
                        memset(&register_callback, 0, sizeof(struct eng_callback));
                        eng_register_func(&register_callback);
                        LOGD("%d:type:%d subtype:%d data_cmd:%d at_cmd:%s", i,
                             register_callback.type, register_callback.subtype,
                             register_callback.diag_ap_cmd, register_callback.at_cmd);

                        modules = get_eng_modules(register_callback);
                        if (modules == NULL)
                        {
                            LOGE("modules is NULL");
                            //continue;
                        }else{
                            list_add_tail(&modules->node, &head[0]);
                        }
                    }

                    eng_register_ext_func = (REGISTER_EXT_FUNC)dlsym(handler, "register_this_module_ext");
                    if (eng_register_ext_func != NULL)
                    {
                        memset(register_arr, 0, sizeof(register_arr));
                        eng_register_ext_func(register_arr_ptr, &register_num);
                        LOGD("register_num:%d", register_num);

                        for (i = 0; i < register_num; i++)
                        {
                            LOGD("%d:type:%d subtype:%d data_cmd:%d at_cmd:%s", i,
                                 register_arr[i].type, register_arr[i].subtype,
                                 register_arr[i].diag_ap_cmd, register_arr[i].at_cmd);

                            modules_ex = get_eng_modules(register_arr[i]);
                            if (modules_ex == NULL)
                            {
                                LOGE("modules is NULL");
                                continue;
                            }
                            if (NULL != modules_ex->callback.eng_set_writeinterface_func) {
                                LOGE("modules is not NULL");
                                modules_ex->callback.eng_set_writeinterface_func(write_interface);
                            }
                            list_add_tail(&modules_ex->node, &head[0]);
                        }
                    }

                    eng_register_fw_func = (REGISTER_FW_FUNC)dlsym(handler, "register_fw_function");
                    if (eng_register_fw_func != NULL)
                    {
                        eng_register_fw_func(&reg);

                        memset(&register_callback_arr, 0, sizeof(struct eng_callback_func));
                        register_callback_arr.eng_cb= eng_register_fw_func;
                        strcpy(register_callback_arr.moudel_des, callback_desc);
                        LOGD("found register_fw_function,desc=%s, register_cb_num=%d", callback_desc, register_cb_num++);

                        memset(callback_desc, 0, sizeof(callback_desc));

                        callbacks = get_eng_callbacks(register_callback_arr);

                        if (callbacks == NULL)
                        {
                            LOGE("callbacks is NULL");
                            //continue;
                        }else{
                            list_add_tail(&callbacks->node, &head[1]);
               }
                    }

                    if (eng_register_func == NULL && eng_register_ext_func == NULL && eng_register_fw_func == NULL)
                    {
                        dlclose(handler);
                        LOGD("%s dlsym fail! %s", path, dlerror());
                        continue;
                    }
                }
            }
            else
            {
                LOGE("%s is not allow to read!", path);
            }
        }
    }

    closedir(dir);

    return 0;
}

