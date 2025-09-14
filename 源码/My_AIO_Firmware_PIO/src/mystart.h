#include "lvgl.h"

lv_obj_t *mystart_src;

lv_obj_t *title_label;
lv_obj_t *author_label;
lv_obj_t *loading_label;

static lv_style_t default_style;
static lv_style_t title_style;
static lv_style_t label_style;
static lv_style_t info_style;

LV_FONT_DECLARE(lv_font_montserrat_20);
LV_FONT_DECLARE(lv_font_montserrat_24);
LV_FONT_DECLARE(lv_font_montserrat_30);

void mystart_gui_init(void)
{
    mystart_src = NULL;
    title_label = NULL;
    author_label = NULL;
    loading_label = NULL;

    lv_style_init(&default_style);
    lv_style_set_bg_color(&default_style, lv_color_hex(0x000000));

    lv_style_init(&title_style);
    lv_style_set_text_opa(&title_style, LV_OPA_COVER);
    lv_style_set_text_color(&title_style, lv_color_white());
    lv_style_set_text_font(&title_style, &lv_font_montserrat_30);

    lv_style_init(&label_style);
    lv_style_set_text_opa(&label_style, LV_OPA_COVER);
    lv_style_set_text_color(&label_style, lv_color_white());
    lv_style_set_text_font(&label_style, &lv_font_montserrat_24);

    lv_style_init(&info_style);
    lv_style_set_text_opa(&info_style, LV_OPA_COVER);
    lv_style_set_text_color(&info_style, lv_color_white());
    lv_style_set_text_font(&info_style, &lv_font_montserrat_20);
}

/*
 * 其他函数请根据需要添加
 */

void start_display_init(void)
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == mystart_src)
        return;

    lv_obj_clean(act_obj); // 清空此前页面

    // 本地的ip地址
    mystart_src = lv_obj_create(NULL);
    lv_obj_add_style(mystart_src, &default_style, LV_STATE_DEFAULT);

    title_label = lv_label_create(mystart_src);
    lv_obj_add_style(title_label, &title_style, LV_STATE_DEFAULT);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 60);
    lv_label_set_recolor(title_label, true); // 先得使能文本重绘色功能

    author_label = lv_label_create(mystart_src);
    lv_obj_add_style(author_label, &info_style, LV_STATE_DEFAULT);
    lv_obj_align(author_label, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_label_set_recolor(author_label, true); // 先得使能文本重绘色功能

    loading_label = lv_label_create(mystart_src);
    lv_obj_add_style(loading_label, &label_style, LV_STATE_DEFAULT);
    lv_obj_align(loading_label, LV_ALIGN_BOTTOM_MID, 0, -70);
    lv_label_set_recolor(loading_label, true); // 先得使能文本重绘色功能

    lv_scr_load(mystart_src);
}

void mystart_display(void)
{
    mystart_gui_init();
    start_display_init();

    lv_label_set_text(title_label, "#00ff00 Joey's\tCubic#");

    lv_label_set_text(author_label, "@Designed by #ff5500ff Joey");

    lv_label_set_text(loading_label, "#8067c9ff Loading...");
}