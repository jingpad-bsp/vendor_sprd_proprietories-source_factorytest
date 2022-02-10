#include "common.h"
#include "modem.h"

extern char s_cali_info[1024];
extern char s_cali_info1[1024];
extern char s_cali_info2[1024];
//Support cdma2000
extern char s_cali_info_cdma2000[1024];
extern char s_cali_info_nr[1024];
extern int getModemType();

//int version_change_page = 0;
extern char* test_modem_get_caliinfo(void);

/********************************************************************
 *  Function:str_replace()
 *********************************************************************/
void str_replace(char *p_result, char* p_source, char* p_seach,
        char *p_repstr) {
    int repstr_len = strlen(p_repstr);
    int search_len = strlen(p_seach);
    char *pos;
    int nLen = 0;
    do {
        pos = strstr(p_source, p_seach);
        if (pos == 0) {
            strcpy(p_result, p_source);
            break;
        }

        nLen = pos - p_source;
        memcpy(p_result, p_source, nLen);
        memcpy(p_result + nLen, p_repstr, repstr_len);
        p_source = pos + search_len;
        p_result = p_result + nLen + repstr_len;
    } while (pos);
}

char* strstr_ignorecase(char * inBuffer, char * inSearchStr) {
    char* currBuffPointer = inBuffer;
    while (*currBuffPointer != 0x00) {
        char* compareOne = currBuffPointer;
        char* compareTwo = inSearchStr;
        while (tolower(*compareOne) == tolower(*compareTwo)) {
            compareOne++;
            compareTwo++;
            if (*compareTwo == 0x00) {
                return (char*) currBuffPointer;
            }

        }
        currBuffPointer++;
    }
    return NULL;
}

int test_cali_info_common(void) {
    int row = 2;
    char tmp[64] = { 0 }, gsm_cali[64][64] = { 0 }, wcdma_cali[64][64] = { 0 },
            lte_cali[64][64] = { 0 };
    display_content cali_content[128];
    char property[PROPERTY_VALUE_MAX];
    char property_wg[PROPERTY_VALUE_MAX];
    char* pcali, *pos1, *pos2;
    int gsm_num = 0, wcdma_num = 0, lte_num = 0;
    int len = 0, total_page = 0, cur_page = 1;
    int n = 0;
    int i;
    //add cdma2000
    char cdma2000_cali[64][64] = { 0 };
    int cdma2000_num = 0;
    //add 5g NR
    char nr_cali[64][64] = { 0 };
    int nr_cali_num = 0;

    ui_fill_locked();
    ui_show_title (MENU_CALI_INFO);

    pcali = s_cali_info;
    len = strlen(pcali);

    /*delete the "BIT",and replace the calibrated with cali */
    //GSM cali ino
    while (len > 0) {
        pos1 = strstr(pcali, ":");
        if (pos1 == NULL)
            break;

        pos1++;
        pos2 = strstr(pos1, "BIT");
        if (pos2 == NULL) {
            strcpy(tmp, pos1);
            len = 0;
        } else {
            memcpy(tmp, pos1, (pos2 - pos1));
            tmp[pos2 - pos1] = '\0';
            len -= (pos2 - pcali);
            pcali = pos2;
        }

        str_replace(gsm_cali[gsm_num], tmp, "calibrated", "cali");
        memset(tmp, 0, sizeof(tmp));

        if (strstr(gsm_cali[gsm_num], "Not"))
            cali_content[n].color = CL_RED;
        else
            cali_content[n].color = CL_GREEN;

        cali_content[n++].content = gsm_cali[gsm_num++];
    }
    cali_content[n++].content = NULL; //for separate different cali item

    //LTE cali ino
    //if ro.radio.modemtype = l, and persist.radio.modem.config = W_G,G ------is W modem, for pike2 7731e
    property_get(PROP_MODEM_LTE_ENABLE, property, "not_find");
    if (!strcmp(property, "1") || getModemType() == MODEM_TYPE_NR) {
        if (4 != test_modem_getlteconf()) {
            pcali = s_cali_info2;
            len = strlen(pcali);
            while (len > 0) {
                memset(tmp, 0, sizeof(tmp));
                pos1 = strstr(pcali, ":");
                if (pos1 != NULL) {
                    pos1++;
                    pos2 = strstr(pos1, "BIT");
                    if (pos2 == NULL) {
                        strcpy(tmp, pos1);
                        len = 0;
                    } else {
                        memcpy(tmp, pos1, (pos2 - pos1));
                        tmp[pos2 - pos1] = '\0';
                        len -= (pos2 - pcali);
                        pcali = pos2;
                    }
                    str_replace(lte_cali[lte_num], tmp, "BAND", "LTE BAND");
                } else {
                    //New text like:"+LTE BAND38, calibrate NOT PASS FT NOT PASS\n+LTE BAND1, calibrate NOT PASS FT NOT PASS"
                    pos2 = strstr(pcali, "\n");
                    if (pos2 == NULL) {
                        strcpy(tmp, pcali);
                        len = 0;
                    } else {
                        memcpy(tmp, pcali, (pos2 - pcali));
                        tmp[pos2 - pcali] = '\0';
                        len -= (pos2 - pcali);
                        pcali = pos2 + 1;
                    }
                    LOGD("lte tmp= %s", tmp);
                    memcpy(lte_cali[lte_num], tmp, sizeof(tmp));
                }

                if (strstr_ignorecase(lte_cali[lte_num], "Not"))
                    cali_content[n].color = CL_RED;
                else
                    cali_content[n].color = CL_GREEN;

                cali_content[n++].content = lte_cali[lte_num++];
            }
        }
    }
    cali_content[n++].content = NULL; //for separate different cali item

    //WCDMA cali ino
    pcali = s_cali_info1;
    len = strlen(pcali);
    while (len > 0) {
        pos1 = strstr(pcali, ":");
        if (pos1 == NULL)
            break;

        pos1++;
        pos2 = strstr(pos1, "BIT");
        if (pos2 == NULL) {
            strcpy(tmp, pos1);
            len = 0;
        } else {
            memcpy(tmp, pos1, (pos2 - pos1));
            tmp[pos2 - pos1] = '\0';
            len -= (pos2 - pcali);
            pcali = pos2;
        }
        str_replace(wcdma_cali[wcdma_num], tmp, "BAND", "WCDMA BAND");
        memset(tmp, 0, sizeof(tmp));

        if (strstr(wcdma_cali[wcdma_num], "Not"))
            cali_content[n].color = CL_RED;
        else
            cali_content[n].color = CL_GREEN;

        cali_content[n++].content = wcdma_cali[wcdma_num++];
    }
    cali_content[n++].content = NULL; //for separate different cali item

    //CDMA2000 cali ino
    if (32 == test_modem_getlteconf()) {
        pcali = s_cali_info_cdma2000;
        len = strlen(pcali);
        while (len > 0) {
            pos1 = strstr(pcali, ":");
            if (pos1 == NULL)
                break;

            pos1++;
            pos2 = strstr(pos1, "BIT");
            if (pos2 == NULL) {
                strcpy(tmp, pos1);
                len = 0;
            } else {
                memcpy(tmp, pos1, (pos2 - pos1));
                tmp[pos2 - pos1] = '\0';
                len -= (pos2 - pcali);
                pcali = pos2;
            }
            str_replace(cdma2000_cali[cdma2000_num], tmp, "BAND",
                    "CDMA2000 BAND");
            memset(tmp, 0, sizeof(tmp));

            if (strstr(cdma2000_cali[cdma2000_num], "Not"))
                cali_content[n].color = CL_RED;
            else
                cali_content[n].color = CL_GREEN;

            cali_content[n++].content = cdma2000_cali[cdma2000_num++];
        }
        cali_content[n++].content = NULL; //for separate different cali item
    }

    //5g nr cali ino
    if (getModemType() == MODEM_TYPE_NR) {
        pcali = s_cali_info_nr;
        len = strlen(pcali);
        while (len > 0) {
            memset(tmp, 0, sizeof(tmp));
            pos1 = strstr(pcali, ":");
            if (pos1 != NULL) {
                pos1++;
                pos2 = strstr(pos1, "BIT");
                if (pos2 == NULL) {
                    strcpy(tmp, pos1);
                    len = 0;
                } else {
                    memcpy(tmp, pos1, (pos2 - pos1));
                    tmp[pos2 - pos1] = '\0';
                    len -= (pos2 - pcali);
                    pcali = pos2;
                }
                str_replace(nr_cali[nr_cali_num], tmp, "BAND", "NR BAND");
            } else {
                //New text like:"+NR BAND38, calibrate NOT PASS FT NOT PASS\n+NR BAND1, calibrate NOT PASS FT NOT PASS"
                pos2 = strstr(pcali, "\n");
                if (pos2 == NULL) {
                    strcpy(tmp, pcali);
                    len = 0;
                } else {
                    memcpy(tmp, pcali, (pos2 - pcali));
                    tmp[pos2 - pcali] = '\0';
                    len -= (pos2 - pcali);
                    pcali = pos2 + 1;
                }
                LOGD("nr tmp= %s", tmp);
                memcpy(nr_cali[nr_cali_num], tmp, sizeof(tmp));
            }
            //memset(tmp, 0, sizeof(tmp));

            if (strstr_ignorecase(nr_cali[nr_cali_num], "Not"))
                cali_content[n].color = CL_RED;
            else
                cali_content[n].color = CL_GREEN;

            cali_content[n++].content = nr_cali[nr_cali_num++];
        }
        cali_content[n++].content = NULL; //for separate different cali item
    }

    total_page += ((((gsm_num % ui_getMaxRows()) > 0) ? 1 : 0)
            + (gsm_num / ui_getMaxRows()));
    total_page += ((((wcdma_num % ui_getMaxRows()) > 0) ? 1 : 0)
            + (wcdma_num / ui_getMaxRows()));
    total_page += ((((lte_num % ui_getMaxRows()) > 0) ? 1 : 0)
            + (lte_num / ui_getMaxRows()));
    //CDMA2000 cali ino
    if (32 == test_modem_getlteconf()) {
        total_page += ((((cdma2000_num % ui_getMaxRows()) > 0) ? 1 : 0)
                + (cdma2000_num / ui_getMaxRows()));
    }
    //5G nr cali ino
    if (getModemType() == MODEM_TYPE_NR) {
        total_page += ((((nr_cali_num % ui_getMaxRows()) > 0) ? 1 : 0)
                + (nr_cali_num / ui_getMaxRows()));
    }
    LOGD("cdma2000_num = %d", cdma2000_num);
    LOGD("gsm_num = %d,%d,%d", gsm_num, wcdma_num, lte_num);

    for (i = 0; i < n; i++) {
        ui_set_color(cali_content[i].color);
        row = ui_show_text(row, 0, cali_content[i].content);

        if ((row - 2) >= ui_getMaxRows() || i == (n - 1)
                || (row != 2 && cali_content[i].content == NULL)) {
            ui_show_page(cur_page, total_page);
            switch (ui_wait_button(NULL, TEXT_NEXT_PAGE, TEXT_GOBACK)) {
            case KEY_VOLUMEDOWN:
            case KEY_VIR_NEXT_PAGE:
                ui_set_color (CL_SCREEN_BG);
                gr_fill(0, 0, gr_fb_width(), gr_fb_height());
                ui_set_color (CL_TITLE_BG);
                ui_show_title (MENU_PHONE_INFO_TEST);
                cur_page++;
                row = 2;
                if (i == (n - 1)) //if is the last page, back to the first page
                        {
                    i = -1;
                    cur_page = 1;
                    LOGD("turn to the first page");
                } else
                    LOGD("turn to the next page");
                break;
            case KEY_POWER:
            case KEY_VIR_BACK:
            case KEY_BACK:
                LOGD("exist");
                return 0;
            default:
                i--;
                row--;
                LOGE("the key is unexpected!");
            }
        }
    }
    return 1;
}

