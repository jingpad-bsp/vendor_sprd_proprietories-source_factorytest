#ifndef _CHNMGR_H_
#define _CHNMGR_H_

#include "eng_modules.h"

typedef enum
{
    CHNL_DIAG,
    CHNL_AT,
    CHNL_NONE,
}CHNL_TYPE;

#define ENGPC_TYPE_NATIVEMMI 0x39

int chnl_init();
struct list_head* chnl_get_call_list_head();
struct list_head* chnl_get_callback_list_head();
int chnl_send(CHNL_TYPE type, char *buf, int len, char *rsp, int rsp_len);
int chnl_is_support(char *cmd_or_desc);
int chnl_callback(char* desc);
int chnl_is_support_callback(char* desc);
int chnl_at_send(char *buf, int len, char *rsp, int rsp_len);
int chnl_diag_send(char *buf, int len, char *rsp, int rsp_len);
int chnl_fw_ptrFunc_find(char *desc);
void chnl_fw_ptrFunc_remove(char *desc);
int chnl_fw_ptrFunc_add(char *desc, void **ptrFunc);

int chnl_send_at_interface(char *rsp, int len);
int chnl_send_diag_interface(char *rsp, int len);

#endif