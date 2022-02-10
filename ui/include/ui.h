#ifndef __UI_H__
#define __UI_H__

#include "minui.h"
#include "freetype.h"

typedef struct {
    unsigned char num;
    char* title;
    int (*func)(void);
}menu_info;

typedef struct menu{
    char* title;
    unsigned char item_total;  //item numbers of menu
    unsigned char item_select;  //high light the current select item
    unsigned char item_top_on_page;  //
    unsigned char item_end_on_page;  //
    unsigned char page_total;
    unsigned char page_current;
    menu_info* menu;
    unsigned char father;
    unsigned char father_item_select;
    unsigned char self;
}menu_list;

void ui_init();
int ui_wait_key(void);
int wait_headset_key(void);
int ui_text_visible();
void ui_clear_key_queue();
void ui_set_background(int icon);
void ui_fill_locked(void);
void ui_show_title(const char* title);
void ui_show_page(unsigned int, unsigned int);
int ui_show_text(int row, int col, const char* text);
int ui_handle_button(const char* left,const char* center,const char* right);
int ui_wait_button(const char* left,const char* center,const char* right);
void ui_show_button(const char* left,  const char* center,const char* right);
void ui_fill_screen(unsigned char r, unsigned char g, unsigned char b);
void ui_clear_rows(int start, int n);
void ui_clear_rows_cols(int row_start, int n1,int col_start,int n2);
void ui_set_color(int cl);
void ui_draw_line(int x1,int y1,int x2,int y2);
void ui_draw_line_mid(int x1,int y1,int x2,int y2);
void ui_push_result(int result);
int device_handle_key(int key_code);
int ui_getMaxItems();
int ui_getMaxRows();
int ui_getItemHeight();
int ui_getLineHeight();
int ui_getTitleHeight();
int ui_getButtonWidth();
int ui_getButtonHeight();
int ui_getCharSize();
void draw_menu(menu_list *m);
int menu_change(menu_list* m, int action);

#endif
