#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>

#include "../mainframe.h"
#include "data_buffer.h"
#include "display/vga_color.h"

CMainFrame * main_frame;

//-----------------------------------------------------------------------
CMainFrame::CMainFrame()
{
    left = 0;
    top = 0;
    width_ = pqm_dis->disbufw();
    height_ = pqm_dis->disbufh();
    color = vgacolor(kVGA_Default);
    border.left_width = 0;
    border.top_width = 0;
    border.right_width = 0;
    border.bottom_width = 0;
    border.color = vgacolor(kVGA_Default);

    update = true;
    InitComponent();
    dis_height_ = tabctrl_phs->height - 2;
    dis_top_ = tabctrl_view->top + 1;
}

//-----------------------------------------------------------------------
CMainFrame::~CMainFrame()
{
}

//-----------------------------------------------------------------------
void CMainFrame::InitComponent()
{
    CLabel *plabel;
    CTabControl *ptabctrl;
    int phs_tab_height = 15;
    int label_help_height = 16;
    
    //界面选项标签类初始化
    tabctrl_view = new CTabControl(3);
    ptabctrl = tabctrl_view;
    ptabctrl->top = 18;
    ptabctrl->left = 0;
    ptabctrl->width = width_;
    ptabctrl->height = height_ - ptabctrl->top - label_help_height - 4;
    ptabctrl->tab_position = tpTOP;
    ptabctrl->tab_height = 18;
    ptabctrl->border.top_width = 1;
    ptabctrl->font.cn_size = 14;
    ptabctrl->font.space = 0;
    ptabctrl->set_tabs_title(0, "主界面");
    ptabctrl->set_tabs_title(1, "谐 波");
    ptabctrl->set_tabs_title(2, "其 它");
    //ptabctrl->set_tabs_title(3, "查 询");

    //相位选项标签类初始化
    tabctrl_phs = new CTabControl(3);
    ptabctrl = tabctrl_phs;
    tabctrl_phs->top = tabctrl_view->top;
    ptabctrl->left = 0;
    ptabctrl->width = width_;
    ptabctrl->height = tabctrl_view->height - phs_tab_height;
    ptabctrl->tab_position = tpBOTTOM;
    ptabctrl->set_tab_start(32);
    ptabctrl->border.bottom_width = 1;
    ptabctrl->tab_height = phs_tab_height;
    ptabctrl->tab_vofst -= 1;
    ptabctrl->font.space = -1;

    if (data_buffer().sys_para()->connect_t()) {
        ptabctrl->set_tabs_title(0, "ＡＢ");
        ptabctrl->set_tabs_title(1, "ＢＣ");
        ptabctrl->set_tabs_title(2, "ＣＡ");
    } else {
        ptabctrl->set_tabs_title(0, "Ａ相");
        ptabctrl->set_tabs_title(1, "Ｂ相");
        ptabctrl->set_tabs_title(2, "Ｃ相");
    }

    //帮助提示标注
    label_help = new CLabel("");
    plabel = label_help;
    plabel->width = width_;
    plabel->height = label_help_height;
    plabel->top = height_ - plabel->height -1;
    plabel->left = left;
    plabel->font.cn_size = 14;
    plabel->font.asc_size = 16;
    plabel->auto_size = false;
    plabel->transparent = false;
    plabel->bgcolor = vgacolor(kVGA_Blue1);
    plabel->font.color = vgacolor(kVGA_White);
    plabel->font.space = 2;
    plabel->left_space = 5;
    plabel->top_space = 0;

    //系统提示标注
    label_sys = new CLabel("");
    *label_sys = *label_help;
    plabel = label_sys;
    plabel->font.cn_size = 14;
    plabel->font.asc_size = 12;
    plabel->font.space = 2;
    plabel->set_visible(false);

    label_clamp = new CLabel("电流钳量程:");
    plabel = label_clamp;
    plabel->left = left+width_-120;
    plabel->top = top;
    plabel->font.cn_size = 14;
    plabel->font.asc_size = 12;
    plabel->set_visible(true);

    //电池电量指示
    battery_indicator = new CBattery(12, 20);
    battery_indicator->set_position(plabel->left-40, 2);
    battery_indicator->set_direct(2);
#ifdef PQNet2xx_3xx
    battery_indicator->set_visible(false);
#endif

    //设备编号
    int i = 34;
    label_unitnum = new CLabel("");
    label_unitnum->left = left+8;
    label_unitnum->top = height_-i;
    label_unitnum->font.asc_size = 12;
    label_unitnum->font.space = 0;

    //频率
    label_frq = new CLabel("");
    label_frq->left = left+136;
    label_frq->top = height_-i;
    label_frq->font.asc_size = 12;
    label_frq->font.space = 0;

    //系统时间
    label_clock = new CLabel("");
    label_clock->left = label_frq->left+64;
    label_clock->top = height_-i;
    label_clock->font.asc_size = 12;
    label_clock->font.space = 0;
}

//-------------------------------------------------------------------------
void CMainFrame::draw()
{
    int i, wl, wr;
    //画边框
    for (i=0;i<border.left_width;i++) { //左边框, 上-->下
        pqm_dis->line(left-i,top,left-i,top+height_,border.color,0);
    }
    for (i=0;i<border.right_width;i++) { //右边框, 上-->下
        pqm_dis->line(left+width_+i-1,top,left+width_+i-1,top+height_,border.color,0);
    }
    wl = border.left_width>1?border.left_width-1:0;
    wr = border.right_width>1?border.right_width-1:0;
    for (i=0;i<border.top_width;i++) { //上边框, 左-->右
        pqm_dis->line(left-wl,top-i,left+width_+wr,top-i,border.color,0);
    }
    for (i=0;i<border.bottom_width;i++) { //下边框, 左-->右
        pqm_dis->line(left-wl,top+height_+i-1,left+width_+wr,top+height_+i-1,border.color,0);
    }

    tabctrl_view->draw();
    if (data_buffer().sys_para()->connect_t()) {
        tabctrl_phs->set_tabs_title(0, "ＡＢ");
        tabctrl_phs->set_tabs_title(1, "ＢＣ");
        tabctrl_phs->set_tabs_title(2, "ＣＡ");
    } else {
        tabctrl_phs->set_tabs_title(0, "Ａ相");
        tabctrl_phs->set_tabs_title(1, "Ｂ相");
        tabctrl_phs->set_tabs_title(2, "Ｃ相");
    }
    tabctrl_phs->draw();

    label_help->draw();
    label_clamp->draw();
    label_unitnum->draw();
    label_frq->draw();
    label_clock->draw();
    battery_indicator->draw();
    label_sys->draw();
}

//-------------------------------------------------------------------------
// ret value=1:refresh all
int CMainFrame::refresh()
{
    int retval = 0;
    if (update) {
        pqm_dis->clear();
        retval = 1;
        draw();
    } else {
        tabctrl_phs->refresh();
        label_clamp->refresh();
        label_frq->refresh();
        label_clock->refresh();
        label_help->refresh();
        battery_indicator->refresh();
        label_sys->refresh();
    }
    update = false;
    return retval;
}



