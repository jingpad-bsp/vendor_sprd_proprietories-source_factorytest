#ifndef __ENG_DIAG_MODULES_H__
#define __ENG_DIAG_MODULES_H__

#include "sprd_fts_type.h"
#include "sprd_fts_log.h"
#include "sprd_fts_diag.h"
#include "sprd_fts_list.h"
#include "sprd_fts_cb.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct eng_modules_info
{
    struct  list_head node;
    struct  eng_callback callback;
}eng_modules;

int eng_modules_load(struct list_head *head);

#ifdef __cplusplus
}
#endif

#endif
