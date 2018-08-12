#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <cassert>

#include "mainform.h"
#include "mainframe.h"
#include "data_buffer.h"
#include "display/vga_color.h"

CMainForm * main_form;

CMainForm::CMainForm()
{
    view_num_ = 0;
    view_totl_ = MAIN_VIEW_TOTL; //子界面总数

    left = 0;
    top = main_frame->dis_top_;
    width = main_frame->width();
    height = main_frame->dis_height_;
    color = vgacolor(kVGA_Default);
    border.left_width = 0;
    border.top_width = 1;
    border.right_width = 0;
    border.bottom_width = 1;
    border.color = vgacolor(kVGA_Default);

    update = true;
    if (pqm_dis->disbufw()==480) {  //480*272
        tw_curve_w_ = 320;
    } else {    //320*240
        tw_curve_w_ = 160;
    }
    InitComponent();
    memset(&ra_data, 0, sizeof(raViewData));
}

//-----------------------------------------------------------------------
CMainForm::~CMainForm()
{
    delete tabctrl_main;
    delete chart_u;
    delete chart_i;
    delete label_urms;
    delete label_THDu;
    delete label_uw_title;
    delete label_iw_title;
    delete label_irms;
    delete label_THDi;
    delete labelset_cap_title;
    delete [] labelset_cap;
    delete chart_ra;
    delete labelset_event;
    delete label_event_title;
}

/*!
Description:Initialize component in mainform
*/
void CMainForm::InitComponent()
{
    //主界面选项标签类初始化
    tabctrl_main = new CTabControl(MAIN_VIEW_TOTL);
    tabctrl_main->top = 16;
    tabctrl_main->left = 15;
    //tabctrl_main->width = width;
    tabctrl_main->height = height;
    tabctrl_main->tab_position = tpLEFT;
    tabctrl_main->set_tab_start(8);
    tabctrl_main->border.left_width = 1;
    tabctrl_main->font.space = 2;
    tabctrl_main->set_tabs_title(0, "波形");
    tabctrl_main->set_tabs_title(1, "事件");
    //tabctrl_main->set_tabs_title(2, "电容");
    //tabctrl_main->set_tabs_title(2, "变压器");
    tabctrl_main->set_tabs_title(3, "γ α");

    InitWaveView();
    InitEventView();
    InitCapwView();
    Init_raView();
    InitTwView();

    //调试信息
    CLabel *plabel;
    label_debug1 = new CLabel("");
    plabel = label_debug1;
    plabel->left = tabctrl_main->left + left + 200;
    plabel->top = chart_i->top + 2;
    plabel->font.space = -1;
    plabel->set_visible(false);

    label_debug2 = new CLabel("");
    *label_debug2 = *label_debug1;
    plabel = label_debug2;
    plabel->top += 13;
    plabel->left -= 20;
}

int compare_hr(const void * arg1, const void * arg2)
{
    const HrWithIdx * phr1 = (HrWithIdx *)arg1;
    const HrWithIdx * phr2 = (HrWithIdx *)arg2;
    if (phr1->a < phr2->a) {
        return 1;
    } else if (phr1->a > phr2->a) {
        return -1;
    } else {
        return 0;
    }
}

/*!
Description:get border according harmonic be shown
            先找出可显示谐波中，alpha值最大的谐波。
            再找到与最大相邻且alpha值更大的谐波
    
    Input:      cnt -- numbers of harmonic that be shown
    Output:     max -- numbers of harmonic that greater than maximum harmonic
*/
void CMainForm::Get_raBorder(int &min_idx, int &max_idx, int cnt)
{
    printf("%s %s\n", __FILE__, __FUNCTION__);    
    int i, k;

    //search maximum  alpha value in ra_data.show_harm.
    int max = 0;
    for (i = 0; i < cnt; i++) {
        k = ra_data.show_harm[i];
        if (ra_data.thr[k - 2][0] > max) {
            max_idx = k;
            max = ra_data.thr[k - 2][0];
        }
    }
    //sort
    HrWithIdx hrbuf[24];
    for (i = 0; i < 24; i++) {
        hrbuf[i].a = ra_data.thr[i][0];
        hrbuf[i].idx = i + 2;
    }
    //printf("&%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",hrbuf[0].a,hrbuf[1].a,hrbuf[2].a,hrbuf[3].a,hrbuf[4].a,hrbuf[5].a,hrbuf[6].a,hrbuf[7].a,hrbuf[8].a,hrbuf[9].a,hrbuf[10].a,hrbuf[11].a);
    //printf("&&%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",hrbuf[12].a,hrbuf[13].a,hrbuf[14].a,hrbuf[15].a,hrbuf[16].a,hrbuf[17].a,hrbuf[18].a,hrbuf[19].a,hrbuf[20].a,hrbuf[21].a,hrbuf[22].a,hrbuf[23].a);
    qsort(hrbuf, 24, sizeof(HrWithIdx), compare_hr);    //sorted in descending order
    //printf("*%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",hrbuf[0].a,hrbuf[1].a,hrbuf[2].a,hrbuf[3].a,hrbuf[4].a,hrbuf[5].a,hrbuf[6].a,hrbuf[7].a,hrbuf[8].a,hrbuf[9].a,hrbuf[10].a,hrbuf[11].a);
    //printf("**%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",hrbuf[12].a,hrbuf[13].a,hrbuf[14].a,hrbuf[15].a,hrbuf[16].a,hrbuf[17].a,hrbuf[18].a,hrbuf[19].a,hrbuf[20].a,hrbuf[21].a,hrbuf[22].a,hrbuf[23].a);

    for (i = 0; i < 24; i++) {
        if (hrbuf[i].idx == max_idx) {
            if (i > 0) max_idx = hrbuf[i - 1].idx;
            break;
        }
    }
    min_idx = ra_data.show_harm[0];
    if (max_idx < min_idx) {
        min_idx = max_idx;
        max_idx = ra_data.show_harm[cnt - 1];
    } else if (max_idx < ra_data.show_harm[cnt - 1]) {
        max_idx = ra_data.show_harm[cnt - 1];
    }
}

/*!
Description:Show the view of event list

    Input:  type -- 0=draw, 1=refresh
*/
char evnt_info[10][48];
void CMainForm::ShowEventList(int type)
{
    if (curr_view_ != kMVEvent) return;
    int i, j, m;

    labelset_event->clear();
    m = data_buffer().GetEvntInfo(10, evnt_info);
    for (i = 0, j = m - 1; i < m; i++, j--) {
        labelset_event->set_txt(i, evnt_info[j]);
    }

    if (type) { //refresh
        label_event_title->refresh();
        labelset_event->refresh();
    } else {    //draw
        label_event_title->draw();
        labelset_event->draw();
        int x = label_event_title->left-2;
        int y = label_event_title->top+label_event_title->height+5;
        if (pqm_dis->disbufw()==480) {  //480*272
            pqm_dis->line(x, y, x+388, y, label_event_title->font.color, 0);
        } else {    //320*240
            pqm_dis->line(x, y, x+302, y, label_event_title->font.color, 0);
        }
    }
}

/*!
Description:Push HRu(a) into ra_data.hr_buf

    Called by:  SaveHarmonic
*/
void CMainForm::PushHRu(void *val)
{
    unsigned short huam_buf[3][kMaxHarmNum];
    memcpy(huam_buf, val, sizeof(short) * 3 * (kMaxHarmNum));

    int i, j, k;

    float base[3];  //基波幅值
    for (i = 0; i < 3; i++) {
        base[i] = unzip_int(huam_buf[i][1]);
    }
    float fi, max;
    k = ra_data.head;
    for (i = 0; i < 24; i++) {
        max = 0;
        for(j = 0; j < 3; j++) {
            if (!base[j]) continue;
            fi = unzip_int(huam_buf[j][i + 2]);
            fi /= base[j];
            if (max < fi) max = fi;
        }
        ra_data.hr_buf[i][k] = max * ra_data.thr[i][2] * 10 + 0.5;  //0.01%
        if (ra_data.hr_buf[i][kHRuBufNum] < ra_data.hr_buf[i][k]) {
            ra_data.hr_buf[i][kHRuBufNum] = ra_data.hr_buf[i][k];
        }
    }
    ra_data.head++;
    ra_data.head %= kHRuBufNum;
    if(view_num_ == 2) {
        DrawHRu();
    }
}

/*!
Description:Initialize component in wave view
*/
void CMainForm::InitWaveView()
{
    CLabel *plabel;
    CChart *pchart;

    int cht_w;
    if (pqm_dis->disbufw()==480) {  //480*272
        cht_w = 307; //刚好三个周波
    } else {    //320*240
        cht_w = 192;
    }
    //电压图表初始化
    chart_u = new CChart(cht_w, (height - 14) / 2 - 1);
    pchart = chart_u;
    pchart->left = width - cht_w;
    pchart->top = top + 1;
    pchart->border.left_width = 0;
    pchart->border.top_width = 0;
    pchart->border.right_width = 0;
    pchart->border.bottom_width = 0;
    //pchart->x_dotnum = 128; //设置显示点数
    pchart->axis_x.axis_border = 2; //坐标轴线型为dash
    pchart->axis_x.ticks_grid_border = 3; //刻度栅格的线型dot
    pchart->axis_x.ticks_origin = false; //不显示原点刻度
    float fi = 1000;
    if (pqm_dis->disbufw()==480) {  //480*272
        fi /= 6;
        pchart = chart_u;
        pchart->wline_ = true; //曲线加粗
        pchart->axis_y.labels_x = 14; //刻度标注位置
        pchart->axis_y.title_font.asc_size = 12;
        pchart->axis_y.labels_font.asc_size = 12;
        pchart->axis_y.title_font.space = 0;
        pchart->axis_y.labels_font.space = 0;
    } else {    //320*240
        fi /= 7.5;
        pchart->axis_x.labels_x = -7; //刻度标注位置
        pchart->axis_x.labels_y = 2;
        pchart->axis_y.labels_x = 6; //刻度标注位置
    }

    pchart->axis_x.ticks_spc = fi + 0.5; //刻度间距
    pchart->set_scale_val(0, 10); //刻度单位数值
    pchart->axis_x.minor_num = 1; //辅助刻度的数目
    pchart->axis_x.minor_grid = false; //不显示辅助刻度栅格
    pchart->axis_x.minor_len = 0; //不显示辅助刻度
    pchart->axis_x.ticks_len = 0; //不显示刻度
    pchart->axis_x.labels_visible = false; //不显示刻度标注

    pchart->axis_y.position = 0;
    pchart->axis_y.ticks_len = 3; //刻度长度
    pchart->axis_y.minor_len = 0; //不显示辅助刻度
    pchart->axis_y.labels_y = 6;
    pchart->axis_y.title = "(kV)";
    pchart->axis_y.title_x = -28;
    pchart->axis_y.title_y = 12; //标题的位置
    pchart->axis_y.ticks_spc = (1000 * 3 + 5) / 11; //刻度间距
    pchart->axis_y.val_prec = 4; //刻度标注的精度
    pchart->axis_y.val_decimal = 3;
    //chart_u->axis_y.labels_align = 1;

    //电流图表初始化
    chart_i = new CChart();
    *chart_i = *chart_u;
    pchart = chart_i;
    pchart->top += chart_u->height();
    pchart->border.top_width = 1;
    pchart->border.bottom_width = 1;
    pchart->axis_x.minor_len = 3; //辅助刻度长度
    pchart->axis_x.ticks_len = 5; //显示刻度长度
    pchart->axis_x.labels_visible = true; //显示刻度标注
    pchart->axis_x.title = "(ms)";
    pchart->axis_x.title_x = chart_i->width() - 27;
    pchart->axis_x.title_y = chart_i->height() + 13;
    pchart->axis_y.title = "(A)";
    pchart->axis_y.title_y = chart_i->height() + 10;
    //pchart->axis_y.ticks_spc = (1000*7+11)/22; //刻度间距
    pchart->axis_y.val_prec = 5; //刻度标注的精度
    pchart->axis_y.val_decimal = 1;
    pchart->axis_y.top_margin = -1;

    if (pqm_dis->disbufw()==480) {  //480*272
        pchart->axis_x.title_font.asc_size = 12;
        pchart->axis_x.labels_font.asc_size = 12;
        pchart->axis_x.title_font.space = 0;
        pchart->axis_x.labels_font.space = 0;
        pchart->axis_x.labels_x = -7;
        pchart->axis_x.labels_y = 1;
        pchart->axis_y.title_x = -22;
        pchart->axis_y.labels_x = 18; //刻度标注位置
        pchart->axis_y.left_margin = 48;
    } else {    //320*240
        pchart->axis_y.title_x = -18;
        pchart->axis_y.labels_x = 8; //刻度标注位置
    }

    //标注类初始化
    //电压部分
    label_urms = new CLabel("Urms:");
    plabel = label_urms;
    plabel->left = tabctrl_main->left + left + 4;
    label_THDu = new CLabel("THDu:");
    plabel = label_THDu;
    plabel->left = label_urms->left;

    label_uw_title = new CLabel("电 压");
    plabel = label_uw_title;
    plabel->left = tabctrl_main->left + left + 12;
    plabel->top = chart_i->top - 16;
    plabel->font.space = 1;
    plabel->font.color = vgacolor(kVGA_HiWhite);

    label_rcd_wave = new CLabel("开始录波... ");
    plabel = label_rcd_wave;
    plabel->set_visible(0);
    plabel->set_update(0);
    plabel->font.space = 1;
    plabel->font.color = vgacolor(kVGA_HiYellow);

    //电流部分
    label_irms = new CLabel("Irms:");
    plabel = label_irms;
    plabel->left = label_urms->left;

    label_THDi = new CLabel("THDi:");
    plabel = label_THDi;
    plabel->left = label_irms->left;

    label_iw_title = new CLabel("电 流");
    plabel = label_iw_title;
    plabel->left = label_uw_title->left;
    plabel->top = chart_i->top + 2;
    plabel->font.cn_size = 14;
    plabel->font.color = vgacolor(kVGA_HiWhite);
    
    if (pqm_dis->disbufw()==480) {  //480*272
        label_urms->font.asc_size = 12;
        label_THDu->font.asc_size = 12;
        label_THDu->font.space = 0;
        label_uw_title->font.cn_size = 14;
        label_urms->top = top + 28;
        label_THDu->top = label_urms->top + 18;
        label_irms->top = chart_i->top + 32;
        label_irms->font.asc_size = 12;
        label_irms->font.space = 0;
        label_THDi->top = label_irms->top + 18;
        label_THDi->font.asc_size = 12;
        label_THDi->font.space = 0;
        label_iw_title->font.space = 1;
        label_rcd_wave->left = left + 26;
        label_rcd_wave->top = label_THDi->top + 40;
    } else {    //320*240
        label_urms->font.space = -1;
        label_urms->top = top + 28;
        label_THDu->font.space = -1;
        label_THDu->top = label_urms->top + 15;
        label_irms->top = chart_i->top + 28;
        label_irms->font.space = -1;
        label_THDi->top = label_irms->top + 15;
        label_THDi->font.space = -1;
        label_rcd_wave->left = left + 22;
        label_rcd_wave->top = label_THDi->top + 36;
    }
}

/*!
Description:Initialize component in capacitor warning view
*/
void CMainForm::InitCapwView()
{
    int i, j, k;

    //电容预警数据初始化
    labelset_cap_title = new CLabelSet(7);
    CLabelSet * plabelset = labelset_cap_title;
    //plabelset->border.top_width = 1;
    plabelset->left = tabctrl_main->left + 8;
    plabelset->top = top + 32;
    plabelset->allrefresh = true;
    if (pqm_dis->disbufw()==480) {  //480*272
        plabelset->width = 80;
        plabelset->height = 206;
        plabelset->font.space = 1;
        plabelset->font.asc_size = 16;
    } else {    //320*240
        plabelset->width = 64;
        plabelset->height = 186;
        plabelset->font.space = -1;
        plabelset->font.asc_size = 12;
    }

    plabelset->font.cn_size = 14;
    i = 0;
    plabelset->set_txt(i++, "寿命预警:");
    plabelset->set_txt(i++, "24h累积预警:");
    plabelset->set_txt(i++, "持续过电压1:");
    plabelset->set_txt(i++, "持续过电压2:");
    plabelset->set_txt(i++, "过电流预警:");
    plabelset->set_txt(i++, "过容限预警:");
    plabelset->set_txt(i++, "峰值过电压:");
    for (i = 0; i < 7; i++) {
        if (pqm_dis->disbufw()==480) {  //480*272
            plabelset->labels_[i].top = i * 24;
        } else {    //320*240
            plabelset->labels_[i].top = i * 22;
        }
        plabelset->labels_[i].font.space = plabelset->font.space;
        plabelset->labels_[i].font.asc_size = plabelset->font.asc_size;
        plabelset->labels_[i].font.cn_size = plabelset->font.cn_size;
        plabelset->labels_[i].self_font = true;
    }

    for (i = 0; i < 4; i++) {
        labelset_cap[i] = new CLabelSet(8);
    }
    plabelset = labelset_cap[0];
    plabelset->top = top + 8;
    if (pqm_dis->disbufw()==480) {  //480*272
        plabelset->left = labelset_cap_title->left + 104;
        plabelset->width = 36;
        plabelset->height = 192;
        plabelset->font.space = 1;
        plabelset->font.asc_size = 16;
        plabelset->font.cn_size = 14;
    } else {    //320*240
        plabelset->left = labelset_cap_title->left + 80;
        plabelset->width = 26;
        plabelset->height = 172;
        plabelset->font.space = -1;
        plabelset->font.asc_size = 12;
        plabelset->font.cn_size = 14;
    }
    plabelset->allrefresh = true;
    for (i = 1; i < 8; i++) { //date time
        plabelset->labels_[i].font.space = plabelset->font.space;
        plabelset->labels_[i].font.asc_size = plabelset->font.asc_size;
        plabelset->labels_[i].font.cn_size = plabelset->font.cn_size;
        plabelset->labels_[i].self_font = true;
    }

    for (i = 1; i < 4; i++) *labelset_cap[i] = *plabelset;

    if (pqm_dis->disbufw()==480) {  //480*272
        j = 168;
        k = 48;
    } else {    //320*240
        j = 126;
        k = 32;
    }

    labelset_cap[0]->width = j;
    labelset_cap[1]->left += j;
    for (i = 2; i < 4; i++) labelset_cap[i]->left = labelset_cap[1]->left + (i - 1) * k;
    i = 0;
    labelset_cap[i++]->set_txt(0, "更新时间");
    labelset_cap[i++]->set_txt(0, "A相");
    labelset_cap[i++]->set_txt(0, "B相");
    labelset_cap[i++]->set_txt(0, "C相");

    for (i = 0; i < 4; i++) {
        for (j = 1; j < 8; j++) {
            if (pqm_dis->disbufw()==480) {  //480*272
                labelset_cap[i]->labels_[j].top = j * 24;
            } else {    //320*240
                labelset_cap[i]->labels_[j].top = j * 22;
            }
        }
    }
}

/*!
Description:Initialize component in event list view
*/
void CMainForm::InitEventView()
{
    //Initialize event list title
    label_event_title = new CLabel("Start Time       Trg  Type   Val(V)  Dur(s)");
    label_event_title->top = top + 4;
    label_event_title->font.cn_size = 14;

    if (pqm_dis->disbufw()==480) {  //480*272
        label_event_title->left = tabctrl_main->left + 16;
        label_event_title->font.space = 2;
        label_event_title->font.asc_size = 16;
    } else {    //320*240
        label_event_title->left = tabctrl_main->left + 6;
        label_event_title->font.space = 0;
        label_event_title->font.asc_size = 12;
    }

    labelset_event = new CLabelSet(10);
    //labelset_event->border.top_width = 1;
    labelset_event->left = label_event_title->left - 2;
    labelset_event->height = 162;
    labelset_event->allrefresh = true;
    int k = 2;
    int cnt = labelset_event->get_count();
    for(int i = 0; i < cnt; i++) {
        labelset_event->labels_[i].top = k;
        if (pqm_dis->disbufw()==480) {  //480*272
            k += 19;
        } else {    //320*240
            k += 16;
        }
    }
    if (pqm_dis->disbufw()==480) {  //480*272
        labelset_event->top = label_event_title->top + 22;
        labelset_event->width = 392;
        labelset_event->font.asc_size = 16;
        labelset_event->font.space = 2;
    } else {    //320*240
        labelset_event->top = label_event_title->top + 19;
        labelset_event->width = 296;
        labelset_event->font.asc_size = 12;
        labelset_event->font.space = 0;
    }
}

/*!
Description:Initialize component in r-a view
*/
void CMainForm::Init_raView()
{
    int cht_w = width - tabctrl_main->left - 24;
    int cht_h = height - 32;
    chart_ra = new CChart(cht_w, cht_h, kDisHarmNum + 1);
    CChart * pchart = chart_ra;
    pchart->left = width - cht_w;
    pchart->top = top + 16;
    pchart->border.left_width = 0;
    pchart->border.top_width = 1;
    pchart->border.right_width = 0;
    pchart->border.bottom_width = 0;
    //pchart->x_dotnum = 128; //设置显示点数

    pchart->axis_x.position = 0;
    //pchart->axis_x.axis_border = 2; //坐标轴线型为dash
    pchart->axis_x.ticks_grid_border = 3; //刻度栅格的线型dot
    pchart->axis_x.ticks_origin = false; //不显示原点刻度
    //pchart->last_tick_ = true; //显示横坐标轴最后一个主刻度的标注
    pchart->axis_x.labels_x = -7; //刻度标注位置
    pchart->axis_x.labels_y = 2;
    pchart->axis_x.minor_num = 1; //辅助刻度的数目
    pchart->axis_x.minor_grid = false; //显示辅助刻度栅格
    pchart->axis_x.title_x = cht_w - 18;
    pchart->axis_x.title_y = cht_h + 12; //标题的位置
    pchart->axis_x.title = "(%)";
    pchart->axis_x.minor_len = 0; //不显示辅助刻度
    //pchart->axis_x.ticks_len = 0; //不显示刻度
    //pchart->axis_x.labels_visible = false; //不显示刻度标注

    pchart->axis_y.position = 0;
    pchart->axis_y.ticks_len = 3; //刻度长度
    pchart->axis_y.minor_len = 0; //不显示辅助刻度
    pchart->axis_y.labels_x = 6; //刻度标注位置
    pchart->axis_y.labels_y = 4;
    pchart->axis_y.title_x = -22;
    pchart->axis_y.title_y = 10; //标题的位置
    //pchart->axis_y.val_prec = 4; //刻度标注的精度
    //pchart->axis_y.val_decimal = 3;
    //chart_u->axis_y.labels_align = 1;

    pchart->set_series_type(kSeriesLine, 0); //设数列显示类型为曲线
    for (int i = 1; i < kMaxShowNum + 1; i++) { //设数列显示类型为打点
        pchart->set_series_type(kSeriesPoint, i);
    }
}

void CMainForm::draw()
{
    int i, wl, wr;

    //画边框
    for(i = 0; i < border.left_width; i++) { //左边框, 上-->下
        pqm_dis->line(left - i, top, left - i, top + height, border.color, 0);
    }
    for(i = 0; i < border.right_width; i++) { //右边框, 上-->下
        pqm_dis->line(left + width + i - 1, top, left + width + i - 1, top + height, border.color, 0);
    }
    wl = border.left_width > 1 ? border.left_width - 1 : 0;
    wr = border.right_width > 1 ? border.right_width - 1 : 0;
    for(i = 0; i < border.top_width; i++) { //上边框, 左-->右
        pqm_dis->line(left - wl, top - i, left + width + wr, top - i, border.color, 0);
    }
    for(i = 0; i < border.bottom_width; i++) { //下边框, 左-->右
        pqm_dis->line(left - wl, top + height + i - 1, left + width + wr, top + height + i - 1, border.color, 0);
    }

    ShowWave(0);
    ShowEventList(0);
    ShowEEWCap(0);

    tabctrl_main->draw();
    if (prdct_type_ >= kEEWNet300) { //Capacitor
        int x = tabctrl_main->left - 8;
        int y = tabctrl_main->top + 128;
        pqm_dis->line(x, y, x, y + 7, tabctrl_main->font.color, 0); //draw a line between γ & α
    }

    chart_ra->draw();
    DrawLegend();

    ShowEEWT(0);
    DrawTwCurve(0);

    label_debug1->draw();
    label_debug2->draw();
    update = false;
}

//-------------------------------------------------------------------------
//type刷新类型，1=全部刷新
void CMainForm::refresh(int type)
{
    if(type) { //显示器全部刷新
        draw();
    } else if(update) { //窗口全部刷新
        pqm_dis->clear(left, top, left + width, top + height);
        draw();
        pqm_dis->refresh(left, top, left + width, top + height);
    } else {
        ShowEEWCap(1);
        ShowWave(1);
        ShowEventList(1);
        chart_ra->refresh();

        ShowEEWT(1);
        DrawTwCurve(1);

        tabctrl_main->refresh();
        label_debug1->refresh();
        label_debug2->refresh();
    }
    update = false;
}

void CMainForm::set_view_totl(int num)
{
    view_totl_ = num;
    if (num == 1) {
        tabctrl_main->left = 2;
        num = 0;
    } else {
        tabctrl_main->left = 15;
    }
    label_urms->left = tabctrl_main->left + left + 4;
    label_THDu->left = label_urms->left;
    label_irms->left = label_urms->left;
    label_THDi->left = label_irms->left;
    label_uw_title->left = tabctrl_main->left + left + 12;
    label_iw_title->left = label_uw_title->left;

    tabctrl_main->set_tab_num(num);
}

//切换主界面的子界面
void CMainForm::SwitchView()
{
    //设置界面选项标签
    chart_u->set_visible(false);
    chart_i->set_visible(false);
    label_urms->set_visible(false);
    label_THDu->set_visible(false);
    label_irms->set_visible(false);
    label_THDi->set_visible(false);
    label_uw_title->set_visible(false);
    label_iw_title->set_visible(false);
    chart_ra->set_visible(false);

    switch(curr_view_) {
        case kWave:     //波形
            chart_u->set_visible(true);
            chart_i->set_visible(true);
            label_urms->set_visible(true);
            label_THDu->set_visible(true);
            label_irms->set_visible(true);
            label_THDi->set_visible(true);
            label_uw_title->set_visible(true);
            label_iw_title->set_visible(true);
            break;
        case kMVEvent:     //事件
            break;
        case kEEWCap: //Capacitor
            break;
        case kEEWCap_ra:     //γ-α
            chart_ra->set_visible(true);
            SetLegendVal();
            DrawLegend();
            Draw_raCurve();
            DrawHRu();
            break;
        case kEEWTrnsfmr: //Transformer
            DrawTwCurve(0);
            break;
        default:
            break;
    }
}

/*!
switch view_num_

    Intput: val -- <0=increase; >0=decrease; 0=reset;
*/
void CMainForm::switch_view_num(int val)
{
    int tl = view_totl_ - 1;
    if (val > 0) {
        view_num_++;
        if (view_num_ > tl) view_num_ = 0;
    } else if (val < 0) {
        view_num_--;
        if (view_num_ < 0) view_num_ = tl;
    } else view_num_ = 0;
    tabctrl_main->set_tab_index(view_num_);

    switch (view_num_) {
        case 0: //Wave
            curr_view_ = kWave;
            break;
        case 1: //Event
            curr_view_ = kMVEvent;
            break;
        case 2: //Capacitor or Transformer
            if (prdct_type_ >= kEEWNet200 && prdct_type_ < kEEWNet300) { //Transformer
                curr_view_ = kEEWTrnsfmr;
            } else {        //Capacitor
                curr_view_ = kEEWCap;
            }
            break;
        case 3: //γ-α
            curr_view_ = kEEWCap_ra;
            break;
        default:
            break;
    }
}

/*!
Description:Initialize component in transformer warning view
*/
void CMainForm::InitTwView()
{
    int i, j;

    // 变压器发热曲线图表初始化
    int cht_w = tw_curve_w_ + 2;
    int cht_h = height - 72;
    chart_t_heat = new CChart(cht_w, cht_h, kTHeatCurveNum);
    CChart * pchart = chart_t_heat;
    pchart->border.color = vgacolor(kVGA_HiGrey);
    pchart->left = tabctrl_main->left + 26;
    pchart->top = top + (height - cht_h - 17);
    pchart->border.left_width = 0;
    pchart->border.top_width = 1;
    pchart->border.right_width = 1;
    pchart->border.bottom_width = 0;
    //pchart->x_dotnum = 128; //设置显示点数

    pchart->axis_x.position = 0;
    //pchart->axis_x.axis_border = 2; //坐标轴线型为dash
    pchart->axis_x.ticks_grid_border = 3; //刻度栅格的线型dot
    pchart->axis_x.ticks_origin = false; //不显示原点刻度
    //pchart->last_tick_ = true; //显示横坐标轴最后一个主刻度的标注
    pchart->axis_x.labels_x = -7; //刻度标注位置
    pchart->axis_x.labels_y = 2;
    pchart->axis_x.minor_num = 1; //辅助刻度的数目
    pchart->axis_x.minor_grid = false; //显示辅助刻度栅格
    pchart->axis_x.title_x = 0;
    pchart->axis_x.title_y = cht_h + 12; //标题的位置
    pchart->axis_x.title = "(s)";
    pchart->axis_x.title_font.space = 0;
    pchart->axis_x.minor_len = 0; //不显示辅助刻度
    //pchart->axis_x.ticks_len = 0; //不显示刻度
    //pchart->axis_x.labels_visible = false; //不显示刻度标注
    pchart->set_last_tick(true); //显示横坐标轴最后一个主刻度的标注
    pchart->axis_x.start_value = 480; //起始坐标对应的值
    pchart->axis_x.labels_font.space = 0;
    pchart->axis_x.color = vgacolor(kVGA_HiGrey);

    pchart->axis_y.position = 0;
    //pchart->axis_x.axis_border = 2; //坐标轴线型为dash
    pchart->axis_y.ticks_grid_border = 3; //刻度栅格的线型dot
    pchart->axis_y.ticks_len = 3; //刻度长度
    pchart->axis_y.minor_len = 0; //不显示辅助刻度
    pchart->axis_y.labels_x = 6; //刻度标注位置
    pchart->axis_y.labels_y = 4;
    pchart->axis_y.title_x = -22;
    pchart->axis_y.title_y = 10; //标题的位置
    //pchart->axis_y.val_prec = 4; //刻度标注的精度
    pchart->axis_y.val_decimal = 1;
    //chart_u->axis_y.labels_align = 1;
    pchart->axis_y.left_margin = -8;
    pchart->axis_y.color = vgacolor(kVGA_HiGrey);

    for (int i = 0; i < kTHeatCurveNum; i++) {
        pchart->set_series_type(kSeriesLine, i);    //设数列显示类型为曲线
    }

    //变压器预警数据初始化
    for (i = 0; i < 3; i++) {
        labelset_trnsfmr[i] = new CLabelSet(kTHeatRecentNum);
    }
    CLabelSet * plabelset = labelset_trnsfmr[0];
    //plabelset->border.top_width = 1;
    plabelset->left = tabctrl_main->left + 24;
    plabelset->top = top + 1;
    plabelset->width = 40;
    plabelset->height = 45;
    plabelset->allrefresh = true;
    //plabelset->font.space = -1;
    plabelset->font.asc_size = 12;
    plabelset->font.cn_size = 14;
    for (i = 0; i < kTHeatCurveNum; i++) {
        plabelset->labels_[i].font.space = plabelset->font.space;
        plabelset->labels_[i].font.asc_size = plabelset->font.asc_size;
        plabelset->labels_[i].font.cn_size = plabelset->font.cn_size;
        plabelset->labels_[i].self_font = true;
    }

    for (i = 1; i < 3; i++) *labelset_trnsfmr[i] = *plabelset;
    labelset_trnsfmr[0]->width = 132;
    labelset_trnsfmr[1]->left += 150;
    labelset_trnsfmr[2]->left = labelset_trnsfmr[1]->left + 52;
    labelset_trnsfmr[2]->width = 64;

    for (i = 0; i < 3; i++) {
        for (j = 1; j < kTHeatRecentNum; j++) {
            labelset_trnsfmr[i]->labels_[j].top = labelset_trnsfmr[i]->labels_[j - 1].top + 15;
        }
    }

    //变压器最新发热数据初始化
    labelset_theat = new CLabelSet(kTHeatCurveNum);
    plabelset = labelset_theat;
    plabelset->left = chart_t_heat->left + chart_t_heat->width() + 8;
    plabelset->top = chart_t_heat->top + 24;
    plabelset->width = left + width - plabelset->left - 2;
    plabelset->height = 56;
    plabelset->allrefresh = true;
    plabelset->font.asc_size = 12;
    plabelset->font.cn_size = 14;
    for (i = 0; i < kTHeatCurveNum; i++) {
        plabelset->labels_[i].font.space = plabelset->font.space;
        plabelset->labels_[i].font.asc_size = plabelset->font.asc_size;
        plabelset->labels_[i].font.cn_size = plabelset->font.cn_size;
        plabelset->labels_[i].self_font = true;
    }
    for (i = 1; i < kTHeatCurveNum; i++) {
        plabelset->labels_[i].top = plabelset->labels_[i - 1].top + 20;
    }
    plabelset->set_labels_font(0 , vgacolor(kVGA_Orange));
    plabelset->set_labels_font(1 , vgacolor(kVGA_HiPurple));
    plabelset->set_labels_font(2 , vgacolor(kVGA_HiPurple));
}

/*!
Show the view of wave

    Input:  type -- 0=draw, 1=refresh
*/
void CMainForm::ShowWave(int type)
{
    if (curr_view_ != kWave) return;
    if (!type) {    //draw
        //电压、电流波形标题的背景区
        pqm_dis->rectangle(tabctrl_main->left, top + chart_u->height() - 16, tabctrl_main->left + 56,
                           top + chart_u->height() + 19, vgacolor(kVGA_Blue1));
        //划分电压与电流区的横线
        pqm_dis->line(tabctrl_main->left, top + chart_u->height() + 1, left + width - chart_u->width(),
                      top + chart_u->height() + 1, color, 0);
        //pqm_dis->setmode(DISOPXOR); //设写显存方式为异或
        //pqm_dis->line(left, top + chart_u->height() + 1, 72,
        //              top + chart_u->height() + 1, vgacolor(kVGA_Default), 0);
        pqm_dis->setmode(DISOPUN); //设写显存方式为覆盖

        //短竖线，用于延长电流坐标的Y轴
        pqm_dis->line(chart_u->left, top + 2 * chart_u->height(),
                      chart_u->left,  top + height, color, 0);
        label_uw_title->draw();
        label_iw_title->draw();

        label_urms->draw();
        label_THDu->draw();
        label_irms->draw();
        label_THDi->draw();

        chart_u->draw();
        chart_i->draw();
    } else {
        label_urms->refresh();
        label_THDu->refresh();
        label_irms->refresh();
        label_THDi->refresh();
        label_uw_title->refresh();
        label_iw_title->refresh();

        chart_u->refresh();
        chart_i->refresh();
    }
    //Manual record wave
    if(label_rcd_wave->get_update() && label_rcd_wave->get_visible()) {
        pqm_dis->clear(label_rcd_wave->left, label_rcd_wave->top,
                       label_rcd_wave->left + label_rcd_wave->width, label_rcd_wave->top + label_rcd_wave->height);
        label_rcd_wave->draw();
    }
}

/*!
Description:Show the view of statistic of capacitor warning information
Input:      type -- 0=draw, 1=refresh
*/
void CMainForm::ShowEEWCap(int type)
{
    if (curr_view_ != kEEWCap) return;
    char str[32];
    int i, j, k, m;
    CLabelSet * plabelset;
    EEWFileHead * peew_head = data_buffer().eew_para()->c_fhead();    //get capacitor warnig file header

    //Display phase A,B,C data
    for (i = 1; i < 4; i++) {
        plabelset = labelset_cap[i];
        for (j = 0; j < 7; j++) {
            k = peew_head->eew_stat[j].warn_cnt[i - 1];
            sprintf(str, "%d", k);
            plabelset->set_txt(j + 1, str);
        }
    }
    //Display update time
    plabelset = labelset_cap[0];
    tm tmi;
    char tm_str[24];
    for (j = 0; j < 7; j++) {
        long tv_sec = peew_head->eew_stat[j].update_time.tv_sec;
        if (!tv_sec) continue;
        LocalTime(&tmi, &tv_sec);
        if (j == 1) { //24h warning
            strftime(str, 20, "%Y-%m-%d", &tmi);
        } else {
            strftime(str, 20, "%Y-%m-%d %H:%M:%S", &tmi);
        }
        plabelset->set_txt(j + 1, str);
    }

    j = peew_head->recent;
    for (i = 0; i < 7; i++) {
        //最近更新预警相别数据标黄
        k = peew_head->eew_stat[i].up_phs;
        if (k) {
            for (m = 1; m < 4; m++) {
                if (k == m) {
                    labelset_cap[m]->set_labels_font(i + 1, vgacolor(kVGA_HiYellow));
                } else {
                    labelset_cap[m]->set_labels_font(i + 1, vgacolor(kVGA_Default));
                }
            }
        }

        //最近更新预警数据对应时间标黄
        if (j & (1 << i)) {
            labelset_cap[0]->set_labels_font(i + 1, vgacolor(kVGA_HiYellow));
        } else {
            labelset_cap[0]->set_labels_font(i + 1, vgacolor(kVGA_Default));
        }
    }

    char capw = data_buffer().eew_para()->cap_life_warning();    //获取寿命预警超限状态
    if (capw) {
        labelset_cap_title->set_labels_font(0, vgacolor(kVGA_HiRed));
        tabctrl_main->tabs_fcolor[2] = vgacolor(kVGA_HiRed);
        for (i = 0; i < 3; i++) {
            if (capw >> i & 1) labelset_cap[i + 1]->set_labels_font(1, vgacolor(kVGA_HiRed));
        }
    }

    if (type) { //refresh
        labelset_cap_title->refresh();
        for (i = 0; i < 4; i++) {
            labelset_cap[i]->refresh();
        }
    } else {    //draw
        labelset_cap_title->draw();
        for (i = 0; i < 4; i++) {
            labelset_cap[i]->draw();
        }
    }
}

/*!
Description:Show the view of transformer warning information
Input:      type -- 0=draw, 1=refresh
*/
void CMainForm::ShowEEWT(int type)
{
    if (curr_view_ != kEEWTrnsfmr) return;
    char str[32];
    int i, k;
    CLabelSet * plabelset;
    EEWFileHead * peew_head = data_buffer().eew_para()->t_fhead();    //获取预警文件头

    //start time
    plabelset = labelset_trnsfmr[0];
    tm tmi;
    for (i = 0; i < kTHeatRecentNum; i++) {
        long tv_sec = peew_head->eew_stat[i].update_time.tv_sec;
        if (!tv_sec) {
            plabelset->set_labels_font(i , vgacolor(kVGA_BlackGrey));
            sprintf(str, "---------- --:--:--");
        } else {
            plabelset->set_labels_font(i , vgacolor(kVGA_Red));
            LocalTime(&tmi, &tv_sec);
            strftime(str, 20, "%Y-%m-%d %H:%M:%S", &tmi);
        }
        plabelset->set_txt(i, str);
    }

    //maximum value
    float fi;
    plabelset = labelset_trnsfmr[1];
    for (i = 0; i < kTHeatRecentNum; i++) {
        k = peew_head->eew_stat[i].warn_cnt[0];
        if (!k) {
            plabelset->set_labels_font(i , vgacolor(kVGA_BlackGrey));
            sprintf(str, "-.---");
        } else {
            plabelset->set_labels_font(i , vgacolor(kVGA_Red));
            fi = k;
            sprintf(str, "%5.3f", fi / 1000);
        }
        plabelset->set_txt(i, str);
    }

    //duration
    int hi, mi, si; //hour minute, second
    plabelset = labelset_trnsfmr[2];
    for (i = 0; i < kTHeatRecentNum; i++) {
        si = peew_head->eew_stat[i].warn_cnt[1] << 16;
        si += peew_head->eew_stat[i].warn_cnt[2];
        if (!si) {
            plabelset->set_labels_font(i , vgacolor(kVGA_BlackGrey));
            sprintf(str, "--:--:--");
        } else {
            plabelset->set_labels_font(i , vgacolor(kVGA_Red));
            hi = si / 3600;
            mi = (si % 3600) / 60;
            si = si - hi * 3600 - mi * 60;
            sprintf(str, "%02d:%02d:%02d", hi, mi, si);
        }
        plabelset->set_txt(i, str);
    }

    if (type) { //refresh
        for (i = 0; i < 3; i++) {
            labelset_trnsfmr[i]->refresh();
        }
    } else {    //draw
        for (i = 0; i < 3; i++) {
            labelset_trnsfmr[i]->draw();
        }
    }
}

int LegendColor[] = { vgacolor(kVGA_HiRed), vgacolor(kVGA_Orange), 
                      vgacolor(kVGA_HiYellow), vgacolor(kVGA_HiGreen),
                      vgacolor(kVGA_Cyan), vgacolor(kVGA_HiBlue), vgacolor(kVGA_HiPurple)
                    };
/*!
Description:Draw (a,r) at r-a chart
*/
void CMainForm::DrawHRu()
{
    if (curr_view_ != kEEWCap_ra) return;
    CChart * pchart = chart_ra;
    pchart->ClearData(1, ra_data.show_num);

    int i, j, k, n;
    float fi = pchart->scale_val(0); //获取每刻度对应的实际值
    fi *= 1000;
    fi /= pchart->axis_x.ticks_spc; //计算出可容纳的最大实际值. scale_val/(ticks_spc/1000)
    fi = pchart->get_dotnum() / fi; //每单位实际值对应的点数
    for (i = 0; i < kHRuBufNum + 1; i++) {
        for (j = 0; j < ra_data.show_num; j++) {
            k = ra_data.show_harm[j] - 2;
            n = ra_data.hr_buf[k][i];
            if (!n) continue;
            pchart->add_data(n * fi / 100.0 + 0.5, ra_data.thr[k][1] / 100.0, LegendColor[j], j + 1);
        }
    }
}

/*!
Description:Draw transformer heat curve
Input:      type -- 0=draw, 1=refresh
*/
void CMainForm::DrawTwCurve(int type)
{
    if (curr_view_ != kEEWTrnsfmr) return;
    CChart *pchart = chart_t_heat;
    int i, k, n;

    k = 4;
    pchart->set_scale_val(0, -120); //设置X轴每刻度对应的标注值
    pchart->axis_x.ticks_spc = (1000 - 2) / k + 0.5; //刻度间距,相对于总宽度的占比，1/1000

    float *pdisbuf[3];  //[0-2]:P_Lr,I_Th_pu, I_rms_pu
    n = data_buffer().GetTwBuffer(tw_curve_w_, pdisbuf);
    float max = 0;
    for (i = 0; i < n; i++) {
        if (max < pdisbuf[0][i]) max = pdisbuf[0][i];
    }
    max *= 1000;
    if (max < 1200) max = 1200;
    max /= (200 - 10);
    pchart->set_scale_val(1, 0.2);  //设置Y轴每刻度对应的标注值
    pchart->axis_y.ticks_spc = 1000 / max + 0.5;   //刻度间距
    pchart->set_y_scale(0.2);     //设置每刻度对应的实际值

    pchart->ClearData();
    int pos;
    if (n > 0) {
        if (n > tw_curve_w_) n = tw_curve_w_;
        pos = tw_curve_w_ - n;
        pchart->add_dataset(pos, pdisbuf[2], n, vgacolor(kVGA_Purple), 0);
        pchart->add_dataset(pos, pdisbuf[1], n, vgacolor(kVGA_Cyan), 1);
        pchart->add_dataset(pos, pdisbuf[0], n, vgacolor(kVGA_Orange), 2);
    }

    //Show last update transformer heat data
    char str[32];
    EEWTData * peew_tdata = data_buffer().eew_para()->p_eew_t_data();  //get last transformer heat data
    CLabelSet * plabelset = labelset_theat;
    sprintf(str, "P_ll-pu: %5.3f", peew_tdata->P_Lr);
    plabelset->set_txt(0, str);
    sprintf(str, "I_th-pu: %5.3f", peew_tdata->I_Th_pu);
    plabelset->set_txt(1, str);
    sprintf(str, "I_rms-pu:%5.3f", peew_tdata->I_rms_pu);
    plabelset->set_txt(2, str);

    if (type) { //refresh
        chart_t_heat->refresh();
        labelset_theat->refresh();
    } else {    //draw
        chart_t_heat->draw();
        labelset_theat->draw();
    }
}

/* -----------------------------------------------------------------------------
Description:Draw Legend of chart_ra
----------------------------------------------------------------------------- */
void CMainForm::DrawLegend()
{
    if (curr_view_ != kEEWCap_ra) return;
    int x = left + 48;
    int y = top + 8;
    int color = vgacolor(kVGA_White);
    char stri[3];
    for (int i = 0; i < ra_data.show_num; i++) {
        int k = ra_data.show_harm[i];
        sprintf(stri, "%d", k);
        pqm_dis->rectangle(x - 1, y - 1, x + 2, y + 2, LegendColor[i]);
        pqm_dis->puts(stri, x + 6, y + 4, color, 1);
        x += 32;
    }
}

/* -----------------------------------------------------------------------------
Description:get harmonic be shown, then set value of legend
----------------------------------------------------------------------------- */
void CMainForm::SetLegendVal()
{
    int i, k;
    long li = data_buffer().eew_para()->show_harmonic();
    int cnt = 0;
    memset(ra_data.show_harm, 0, sizeof(int) * kMaxShowNum);
    for (i = 0; i < 32; i++) {
        if (li >> i & 1) {
            ra_data.show_harm[cnt] = i + 2;
            cnt++;
        }
        if (cnt >= kMaxShowNum) break;
    }
    ra_data.show_num = cnt;

    //Add two harmonic that alpha value >max && <min
    memcpy(ra_data.thr, data_buffer().eew_para()->p_eew_ra_thr(), sizeof(short) * 24 * 3);
    int min, max;
    Get_raBorder (min, max, cnt);
    ra_data.show_harm[kMaxShowNum] = min;
    ra_data.show_harm[kMaxShowNum + 1] = max;
    printf("min=%d, max=%d\n", min, max);
}

/*!
Description:display r-a view
*/
void CMainForm::Draw_raCurve()
{
    CChart *pchart = chart_ra;
    int i, k;
    int a_max, r_max;   //maxium of alpha,gamma
    pchart->ClearData();

    a_max = r_max = 0;
    for (i = ra_data.show_harm[kMaxShowNum] - 2; i < ra_data.show_harm[kMaxShowNum + 1] - 1; i++) {
        if (a_max < ra_data.thr[i][0]) a_max = ra_data.thr[i][0];
        if (r_max < ra_data.thr[i][1]) r_max = ra_data.thr[i][1];
    }
    float fi = r_max;
    fi /= 98;   //100-1 最顶部留白
    if (fi < 28) k = 5;
    else k = 10;
    pchart->set_scale_val(1, k);
    pchart->axis_y.ticks_spc = 1000 * k / fi + 0.5; //刻度间距
    pchart->set_y_scale(k);

    fi = a_max;
    fi /= 98;   //100-1 最右留白
    if (fi < 13) k = 2;
    else k = 5;
    pchart->set_scale_val(0, k); //设置每刻度对应的实际值
    pchart->axis_x.ticks_spc = 1000 * k / fi + 0.5; //刻度间距,相对于总宽度的占比，1/1000

    float data;
    float fj = pchart->get_dotnum();    //获取总点数
    fi = fj / fi;   //每单位实际值对应的点数
    int color = vgacolor(kVGA_BlackGrey);    //COLOR_HI_YELLOW;
    for (i = ra_data.show_harm[kMaxShowNum] - 2; i < ra_data.show_harm[kMaxShowNum + 1] - 1; i++) {
        fj =  ra_data.thr[i][0] * fi / 100 + 0.5;
        data = ra_data.thr[i][1];
        pchart->add_data(fj, data / 100, color, 0);
    }
}

