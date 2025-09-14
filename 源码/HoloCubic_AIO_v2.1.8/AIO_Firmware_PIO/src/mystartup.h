#include "lvgl.h"
#include "common.h"

lv_obj_t *my_startup_scr;

lv_obj_t *title_label;
lv_obj_t *myname_label;
lv_obj_t *forperson_label;

static lv_style_t default_style;
static lv_style_t title_style;
static lv_style_t label_style;

LV_FONT_DECLARE(lv_font_montserrat_20);
LV_FONT_DECLARE(lv_font_montserrat_24);
LV_FONT_DECLARE(lv_font_montserrat_30);

void mystart_gui_init(void)
{
    my_startup_scr = NULL;
    title_label = NULL;
    myname_label = NULL;
    forperson_label = NULL;

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
}


void start_display_init(void)
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == my_startup_scr)
        return;

    lv_obj_clean(act_obj); // 清空此前页面

    // 启动页标识
    my_startup_scr = lv_obj_create(NULL);
    lv_obj_add_style(my_startup_scr, &default_style, LV_STATE_DEFAULT);

    title_label = lv_label_create(my_startup_scr);
    lv_obj_add_style(title_label, &title_style, LV_STATE_DEFAULT);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 0);
    lv_label_set_recolor(title_label, true); //先得使能文本重绘色功能

    myname_label = lv_label_create(my_startup_scr);
    lv_obj_add_style(myname_label, &label_style, LV_STATE_DEFAULT);
    lv_label_set_recolor(myname_label, true); //先得使能文本重绘色功能
    lv_obj_align(myname_label, LV_ALIGN_BOTTOM_LEFT, 5, -100);

    forperson_label = lv_label_create(my_startup_scr);
    lv_obj_add_style(forperson_label, &label_style, LV_STATE_DEFAULT);
    lv_label_set_recolor(forperson_label, true); //先得使能文本重绘色功能
    lv_obj_align(forperson_label, LV_ALIGN_BOTTOM_MID, 0, -150);


    // lvgl8之前版本，由于使用了 LV_LABEL_LONG_SCROLL_CIRCULAR 所以 lv_obj_set_size 是不生效的
    // lvgl8之前版本，模式一旦设置 LV_LABEL_LONG_SCROLL_CIRCULAR
    // 宽度恒定等于当前文本的长度，所以下面先设置以下长度
    // lv_label_set_text(new_ver_label, "Please update your A");
    // lv_label_set_recolor(new_ver_label, true); //先得使能文本重绘色功能
    // lv_label_set_long_mode(new_ver_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    // lv_obj_align(new_ver_label, LV_ALIGN_CENTER, 0, 60);
    // lv_label_set_text_fmt(new_ver_label, "#ff0000 Is the latest version!");

    lv_scr_load(my_startup_scr);
}

void start_display(const char *myname, const char *forperson)
{
    start_display_init();

    lv_label_set_text(title_label, "#00ff00 SimonCubic#");

    lv_label_set_text_fmt(forperson_label, "for #ff0000 %s#", forperson);

    lv_label_set_text_fmt(myname_label, "Designed by %s#", myname);

    lv_scr_load(my_startup_scr);
    screen.routine();   //若之前有已经显示了相关内容需要刷新一遍
}

void mystrat_gui_del(void)
{
    if (NULL != my_startup_scr)
    {
        lv_obj_clean(my_startup_scr);
        my_startup_scr = NULL;
        title_label = NULL;
        forperson_label = NULL;
        myname_label = NULL;
    }

    // 手动清除样式，防止内存泄漏
    lv_style_reset(&default_style);
    lv_style_reset(&title_style);
    lv_style_reset(&label_style);
}