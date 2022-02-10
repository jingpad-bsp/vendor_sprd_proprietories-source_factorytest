#ifndef _FACTORY_H
#define _FACTORY_H

/***************Result Show************/
#define PCBATXTPATH   "/mnt/vendor/PCBAtest.txt"
#define PHONETXTPATH    "/mnt/vendor/wholephonetest.txt"

typedef struct mmitest_result
{
    unsigned char type_id;
    unsigned char function_id;
    unsigned char eng_support;
    unsigned char pass_faild;   //0:not test,1:pass,2:fail
}mmi_result;

typedef struct mmi_show_data
{
    unsigned char id;
    char* name;
    int (*func)(void);
    mmi_result* mmi_result_ptr;
    char title[64];
}mmi_show_data;

#endif
