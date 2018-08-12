#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>

#include "chart.h"
#include "display/vga_color.h"

#define ALLOC_MEM(i,num) \
    series_type_ = new int[num];\
    data_value = new int*[num];\
    data_color = new int*[num];\
    data_yn_ = new char*[num];\
    for (i=0; i<num; i++) {\
        data_value[i] = new int[width_];\
        data_color[i] = new int[width_];\
        data_yn_[i] = new char[width_];\
    }

#define FREE_MEM(i, num) \
    for (i=0; i<num; i++) {\
        delete [] data_value[i];\
        delete [] data_color[i];\
        delete [] data_yn_[i];\
    }\
    delete [] data_value;\
    delete [] data_color;\
    delete [] data_yn_;\
    delete [] series_type_;

/* -----------------------------------------------------------------------------
Input:      num -- dataset number
----------------------------------------------------------------------------- */
CChart::CChart(int w, int h, int num)
{
    visible = true;
    left = 0;
    top = 0;
    width_ = w;
    height_ = h;
    //color = vgacolor(kVGA_Default);
    border.left_width = 1;
    border.top_width = 1;
    border.right_width = 1;
    border.bottom_width = 1;
    border.color = vgacolor(kVGA_Default);

    init_axis(&axis_x, width_);
    axis_x.labels_align = 1;
    init_axis(&axis_y, height_);

    bar_width = 3; //柱状图柱子的宽度
    y_scale_ = axis_y.ticks_spc; //Y轴每刻度对应的值
    x_dotnum = 0; //X轴点数，0=1对1
    start_point = 0;
    draw_range = width_;

    dataset_num_ = num;
    int i;
    ALLOC_MEM(i, num)
    for (i = 0; i < num; i++) {
        series_type_[i] = kSeriesQLine;
    }

    ClearData();
    update = true;
    wline_ = false;
    last_tick_ = false;
}

//-------------------------------------------------------------------------
CChart::~CChart()
{
    int i;
    FREE_MEM(i, dataset_num_)
}

//-------------------------------------------------------------------------
// 赋值函数
CChart & CChart::operator =(const CChart &other)
{
    //检查自赋值
    if(this == &other)
        return *this;

    width_ = other.width_;
    height_ = other.height_;
    set_sz(width_, height_);
    left = other.left;
    top = other.top;
    //color = other.color;
    memcpy(&border, &other.border, sizeof(SEdges));
    memcpy(&axis_x, &other.axis_x, sizeof(SAxis));
    memcpy(&axis_y, &other.axis_y, sizeof(SAxis));

    bar_width = other.bar_width; //柱状图柱子的宽度
    y_scale_ = other.y_scale_; //Y轴每刻度对应的值
    x_dotnum = other.x_dotnum; //X轴点数，0=1对1
    start_point = other.start_point;
    draw_range = other.draw_range;
    wline_ = other.wline_;
    for (int i = 0; i < dataset_num_; i++) {
        series_type_[i] = other.series_type_[i];
    }
    //返回本对象的引用
    return *this;
}

//-------------------------------------------------------------------------
void CChart::init_axis(SAxis * axis, int size)
{
    axis->visible = true;
    axis->color = vgacolor(kVGA_Default);
    axis->position = 500;
    axis->axis_border = 0;

    axis->ticks_spc = 300;
    axis->ticks_len = 6;
    axis->ticks_origin = true;
    axis->ticks_grid = true;
    axis->ticks_grid_border = 1;

    axis->minor_len = 3;
    axis->minor_num = 4;
    axis->minor_grid = false;
    axis->minor_grid_border = 1;

    axis->labels_visible = true;
    axis->labels_font.asc_size = 8;
    axis->labels_font.cn_size = 12;
    axis->labels_font.space = -2;
    axis->labels_font.color = vgacolor(kVGA_Default);
    axis->labels_align = 0;
    axis->labels_x = 0;
    axis->labels_y = 0;
    axis->start_value = 0;
    axis->scale_value = 20;
    axis->val_prec = 3;
    axis->val_decimal = 0;
    axis->val_unit = "";
    axis->labels_1st_minor = false;

    axis->title = "";
    axis->title_font.asc_size = 8;
    axis->title_font.cn_size = 12;
    axis->title_font.space = -2;
    axis->title_font.color = vgacolor(kVGA_Default);
    axis->title_x = 50;
    axis->title_y = 0;

    axis->top_margin = 0;
    axis->left_margin = 0;
    axis->bottom_margin = 0;
    axis->right_margin = 0;
    /*  axis->scale_min = -size/2;
        axis->scale_max = size/2;
        axis->scale_auto = false;
    */
}

//-------------------------------------------------------------------------
void CChart::set_visible(bool yn)
{
    int i, j;

    visible = yn;
    i = axis_x.labels_y;
    i += axis_x.labels_font.cn_size;
    j = 7;
    j -= axis_y.labels_font.space;
    j *= (axis_y.val_prec + 1);
    j -= axis_y.labels_x;
    if(!visible) {
        pqm_dis->clear(left, top, left + width_, top + height_);
        pqm_dis->clear(left, top + height_, left + width_, top + height_ + i);
        pqm_dis->clear(left - j, top, left, top + height_);
    }
    update = true;
    axisx_update = true;
    axisy_update = true;
}

//-------------------------------------------------------------------------
void CChart::refresh()
{
    if(!update || !visible) return;

    int i, j;
    bool x_up = false;
    bool y_up = false;
    if(axisx_update) {
        i = axis_x.labels_y;
        i += axis_x.labels_font.cn_size;
        pqm_dis->clear( left - axis_x.left_margin,
                        top + height_ - axis_x.top_margin,
                        left + width_ + axis_x.right_margin,
                        top + height_ + i + axis_x.bottom_margin );
        x_up = true;
    }
    if(axisy_update) {
        j = 7;
        j -= axis_y.labels_font.space;
        j *= (axis_y.val_prec + 1);
        j -= axis_y.labels_x;
        pqm_dis->clear( left - j - axis_y.left_margin,
                        top - axis_y.top_margin,
                        left + axis_y.right_margin,
                        top + height_ + axis_y.bottom_margin );
        y_up = true;
    }
    pqm_dis->clear(left, top, left + width_, top + height_);
    draw();
    pqm_dis->refresh(left, top, left + width_, top + height_);
    if(x_up) {
        pqm_dis->refresh( left - axis_x.left_margin,
                          top + height_ - axis_x.top_margin,
                          left + width_ + axis_x.right_margin,
                          top + height_ + i + axis_x.bottom_margin );
    }
    if(y_up) {
        pqm_dis->refresh( left - j - axis_y.left_margin,
                          top - axis_y.top_margin,
                          left + axis_y.right_margin,
                          top + height_ + axis_y.bottom_margin );
    }
}

//-------------------------------------------------------------------------
void CChart::draw()
{
    update = false;
    if(!visible) return;
    axisx_update = false;
    axisy_update = false;

    draw_frame();
    for (int i = 0; i < dataset_num_; i++) draw_data(i);
}

//-------------------------------------------------------------------------rrr
//绘制图标框架
void CChart::draw_frame()
{
    int i, wl, wr;

    //画边框
    for(i = 0; i < border.left_width; i++) { //左边框, 上-->下
        pqm_dis->line(left - i, top, left - i, top + height_, border.color, 0);
    }
    for(i = 0; i < border.right_width; i++) { //右边框, 上-->下
        pqm_dis->line(left + width_ + i - 1, top, left + width_ + i - 1, top + height_, border.color, 0);
    }
    wl = border.left_width > 1 ? border.left_width - 1 : 0;
    wr = border.right_width > 1 ? border.right_width - 1 : 0;
    for(i = 0; i < border.top_width; i++) { //上边框, 左-->右
        pqm_dis->line(left - wl, top - i, left + width_ + wr, top - i, border.color, 0);
    }
    for(i = 0; i < border.bottom_width; i++) { //下边框, 左-->右
        pqm_dis->line(left - wl, top + height_ + i - 1, left + width_ + wr, top + height_ + i - 1, border.color, 0);
    }

    //坐标轴
    if(axis_y.visible) {
        draw_axis(&axis_y, axis_x.position, 1);
    }
    if(axis_x.visible) {
        draw_axis(&axis_x, axis_y.position, 0);
    }
}


/*-------------------------------------------------------------------------
Description:画标尺(刻度、栅格及标注)
Input:      axis -- 轴参数
            type -- 轴类型，0=水平轴; 1=垂直轴
            pos -- 刻度位置
            sn -- 刻度序号
            sign -- 0=正半轴; 1=负半轴
*/
void CChart::draw_staff(SAxis * axis, int type, int pos, int sn, int sign)
{
    int i, k, j, m, n, sg, color;
    int x, y, w, v, h, lv, lh;
    float fi;
    char str[24];

    if(!sign) { //正半轴
        sg = 1;
    } else { //负半轴
        sg = -1;
    }
    //初始化与轴类型相关的参数
    lv = 0;
    lh = 0;
    v = 0;
    h = 0;
    if(!type) { //水平
        w = width_;
        i = (pos * w + sg * 499) / 1000;
        x = left + i;
        y = top + height_ - 1;
        lv = -height_;
        v = 1;
    } else { //垂直
        w = height_;
        i = (pos * w + sg * 499) / 1000;
        x = left;
        y = top + height_ - 1 - i;
        lh = width_;
        h = 1;
    }
    color = axis->color;

    k = axis->labels_x - h * axis->labels_font.asc_size * (axis->val_prec + 1);
    j = axis->labels_y + v * axis->labels_font.cn_size;

//--------- 主刻度 -----------------------
    //画刻度栅格
    if(axis->ticks_grid && sn != 0) { //显示刻度栅格
        pqm_dis->line(x, y, x + lh, y + lv, color, axis->ticks_grid_border);
    }
    if(sn != 0 || (axis->ticks_origin && sign == 0)) { //不为原点或需要显示原点刻度
        //画刻度
        if(axis->ticks_len) { //刻度长度不为0
            pqm_dis->line(x, y, x - h * axis->ticks_len, y + v * axis->ticks_len, color, 0);
        }
        //画刻度标注
        if(axis->labels_visible) { //显示刻度标注
            fi = axis->start_value + axis->scale_value * sn;
            i = float_fmt(fi, axis->val_prec, axis->val_decimal);
            if(!sn) { //原点
                i = axis->start_value;
            }
            if(axis->labels_align) { //左对齐
                sprintf(str, "%-*.*f%s", axis->val_prec + 1, i, fi, axis->val_unit);
            } else { //右对齐
                sprintf(str, "%*.*f%s", axis->val_prec + 1, i, fi, axis->val_unit);
            }
            pqm_dis->set_font(axis->labels_font.cn_size, axis->labels_font.asc_size);
            if((pos + axis->ticks_spc < 1010) || last_tick_ || type) //水平线最后一个刻度标注不显示了
                pqm_dis->puts(str, x + k, y + j, axis->labels_font.color, axis->labels_font.space);
        }
    }

//--------- 辅助刻度 -----------------------
    m = sg;
    i = sg * (axis->minor_num + 1) / 2;
    n = (m * axis->ticks_spc + i) / (axis->minor_num + 1);
    while(m >= -axis->minor_num && m <= axis->minor_num && (pos + n) > 0 && (pos + n) < 1000) {
        n = (n * w + sg * 499) / 1000;
        //画辅助刻度栅格
        if(axis->minor_grid && ((pos + axis->ticks_spc < 1000 - 20) || type)) { //显示辅助刻度栅格
            pqm_dis->line(x + v * n, y - h * n, x + v * n + lh, y - h * n + lv, color, axis->ticks_grid_border);
        }
        //画辅助刻度
        if(axis->minor_len && ((pos + axis->ticks_spc < 1000 - 20) || type)) { //辅助刻度的长度不为0
            pqm_dis->line(x + v * n, y - h * n, x + v * n - h * axis->minor_len,
                          y - h * n + v * axis->minor_len, color, 0);
        }
        m += sg;
        n = (m * axis->ticks_spc + i) / (axis->minor_num + 1);
    }

    //画第一个辅助刻度标注
    if(axis->labels_1st_minor && sn == 0) { //显示第一个辅助刻度标注
        n = (w * axis->ticks_spc + 499) / (axis->minor_num + 1) / 1000; //辅助刻度的位置
        n *= sg;
        fi = axis->scale_value / (axis->minor_num + 1);
        i = float_fmt(fi, axis->val_prec, axis->val_decimal);
        if(axis->labels_align) { //左对齐
            sprintf(str, "%-*.*f%s", axis->val_prec + 1, i, fi, axis->val_unit);
        } else { //右对齐
            sprintf(str, "%*.*f%s", axis->val_prec + 1, i, fi, axis->val_unit);
        }
        pqm_dis->set_font(axis->labels_font.cn_size, axis->labels_font.asc_size);
        pqm_dis->puts(str, x + n * v + k, y + n * h + j,
                      axis->labels_font.color, axis->labels_font.space);
    }
}

/*-------------------------------------------------------------------------
Description:画坐标轴、标尺及标题
Input:      axis -- 轴
            pos -- 与本轴垂直轴的位置
            type -- 轴类型，0=水平轴; 1=垂直轴
*/
void CChart::draw_axis(SAxis * axis, int pos, int type)
{
    int i, j, lv, lh, x, y;

    //坐标轴
    //初始化与轴类型相关的参数
    lv = 0;
    lh = 0;
    if(!type) { //水平
        x = left;
        y = top + height_ - 1 - (height_ * axis->position + 499) / 1000;
        lh = width_;
    } else { //垂直
        x = left + (width_ * axis->position + 499) / 1000;
        y = top;
        lv = height_;
    }
    pqm_dis->line(x, y, x + lh, y + lv, axis->color, axis->axis_border);

    //pqm_dis->setmode(DISOPXOR); //设写显存方式为异或
    if(axis->ticks_spc) { //刻度间距不为0
        //正半轴的标尺
        i = pos;
        j = 0;
        while(i < 1000-2) {
            draw_staff(axis, type, i, j, 0);
            i += axis->ticks_spc;
            j ++;
        }
        //负半轴的标尺
        i = pos;
        j = 0;
        while(i > 10) {
            draw_staff(axis, type, i, j, 1);
            i -= axis->ticks_spc;
            j --;
        }
    }
    pqm_dis->setmode(DISOPUN); //设写显存方式为覆盖

    //轴标题
    pqm_dis->set_font(axis->title_font.cn_size, axis->title_font.asc_size);
    pqm_dis->puts(axis->title, left + axis->title_x, top + axis->title_y,
                  axis->title_font.color, axis->title_font.space);
}

//-------------------------------------------------------------------------
void CChart::ClearData()
{
    ClearData(0, dataset_num_);
}
void CChart::ClearData(int pos, int sz)
{
    if ( (pos+sz)>dataset_num_ ) return;
    for (int i = pos; i < pos+sz; i++) memset(data_yn_[i], 0, width_);
}

/*!
Description:add one show data to position(pos) of one groups(grp)

    Input:  grp -- which group to be set?
*/
void CChart::add_data(int pos, float data, int color, int grp)
{
    int j, m, wd;
    float fi;

    wd = draw_range;
    m = x_dotnum;
    if(!m) {
        m = wd;
    }
    if(pos >= m) return;
    fi = height_ * axis_y.ticks_spc;
    fi /= 1000;

    j = (pos + 1) * wd / m + start_point;
    data_yn_[grp][j] = 1;
    data_color[grp][j] = color;
    fi = data * fi / y_scale_;
    if(fi >= 0) {
        fi += 0.5;
    } else {
        fi -= 0.5;
    }
    data_value[grp][j] = (int)fi;
    update = true;
}

/*!
Description:添加一组显示数据
    Intput: pos - 起始位置(横坐标)
            data - 数据集(纵坐标)
            num - data 的数目
            grp -- which group to be set?
*/
void CChart::add_dataset(int pos, float *data, int num, int color, int grp)
{
    int i, j, m, wd;
    float fi, fj;

    wd = draw_range;
    m = x_dotnum;
    if(!m) {
        m = wd;
    }
    if(pos >= m) return;

    fi = height_ * axis_y.ticks_spc;
    fi /= 1000;
    i = 0;
    j = (pos + i) * wd / m + start_point;
    //printf("fi=%5.3f, num=%d,j=%d, width_=%d\n",fi, num,j,width_);
    while(i < num && j < width_) {
        data_yn_[grp][j] = 1;
        data_color[grp][j] = color;
        fj = data[i] * fi / y_scale_;
        if(fj >= 0) {
            fj += 0.5;
        } else {
            fj -= 0.5;
        }
        data_value[grp][j] = (int)fj;
        i ++;
        j = (pos + i) * wd / m + start_point;
    }
    update = true;
}

//-------------------------------------------------------------------------
//绘制显示数据
void CChart::draw_data(int grps)
{
    int i, x, y, y0, x1, y1, stpt;

    //pqm_dis->setmode(DISOPXOR); //设写显存方式为异或

    if (series_type_[grps] == kSeriesQLine) { //快速曲线图
        y0 = top + height_ - 1 - axis_x.position * height_ / 1000;
        x = left + 1;
        y = y0;
        stpt = 1;
        for (i = 0; i < width_ - 1; i++) {
            x1 = x;
            y1 = y;
            x = left + i + 1;
            if (!data_yn_[grps][i]) {
                stpt = 1;
                continue;
            }
            y = y0 - data_value[grps][i];
            if (y < top) {
                y = top;
                continue;
            }
            if (y > top + height_ - 1) {
                y = top + height_;
                continue;
            }
            if((y - y1) < 2 && (y - y1) > -2 && (x - x1) < 2 && (x - x1) > -2 || stpt) {
                if (!wline_) {
                    pqm_dis->set_pixel(x, y, data_color[grps][i]);
                } else {
                    pqm_dis->set_wpixel(x, y, data_color[grps][i]);
                }
                stpt = 0;
            } else {
                if (!wline_) {
                    pqm_dis->line(x1, y1, x, y, data_color[grps][i], 0);
                    pqm_dis->set_pixel(x, y, data_color[grps][i]);
                } else {
                    pqm_dis->wline(x1, y1, x, y, data_color[grps][i], 0);
                    pqm_dis->set_wpixel(x, y, data_color[grps][i]);
                }
            }
        }
    } else if (series_type_[grps] == kSeriesLine) { //通用曲线图
        y0 = top + height_ - 1 - axis_x.position * height_ / 1000;
        i = 0;
        while(!data_yn_[grps][i] && i < width_) {
            i++;
        }
        x = left + i + 1;
        y = y0 - data_value[grps][i];
        i--;
        while (i < width_) {
            i++;
            if (!data_yn_[grps][i]) continue;
            x1 = x;
            y1 = y;
            x = left + i + 1;
            y = y0 - data_value[grps][i];
            //超出显示范围，则不显示
            if (y < top) {
                y = top;
                continue;
            }
            if (y > top + height_ - 1) {
                y = top + height_;
                continue;
            }
            if (!wline_) {
                pqm_dis->line(x1, y1, x, y, data_color[grps][i], 0);
                pqm_dis->set_pixel(x, y, data_color[grps][i]);
            } else {
                pqm_dis->wline(x1, y1, x, y, data_color[grps][i], 0);
                pqm_dis->set_wpixel(x, y, data_color[grps][i]);
            }
        }
    } else if (series_type_[grps] == kSeriesBar) {  //柱状图
        y0 = top + height_ - 1 - axis_x.position * height_ / 1000;
        for (i = 0; i < width_; i++) {
            if(!data_yn_[grps][i]) continue;
            x = left + i;
            y = y0 - data_value[grps][i];
            if(y < top) y = top;
            if(y >= top + height_) y = top + height_ - 1;
            pqm_dis->wline(x, y0, x, y, data_color[grps][i], bar_width);
        }
    } else if (series_type_[grps] == kSeriesPoint) { //打点图
        y0 = top + height_ - 1 - axis_x.position * height_ / 1000;
        for (i = 0; i < width_; i++) {
            if(!data_yn_[grps][i]) continue;
            x = left + i;
            y = y0 - data_value[grps][i];
            if(y < top) y = top;
            if(y >= top + height_) y = top + height_ - 1;
            pqm_dis->rectangle(x - 1, y - 1, x + 2, y + 2, data_color[grps][i]);
        }
    }
    pqm_dis->setmode(DISOPUN); //设写显存方式为覆盖
}

//-------------------------------------------------------------------------
//设置宽窄
void CChart::set_sz(int w, int h)
{
    width_ = w;
    height_ = h;
    int i;
    FREE_MEM(i, dataset_num_)
    ALLOC_MEM(i, dataset_num_)
    ClearData(0, dataset_num_);
}

/*!
Description: 设置单位刻度对应的标注值

    Input:  xy -- 0=X轴, 1=Y轴
*/
void CChart::set_scale_val(int xy, float val)
{
    if(xy) {
        axis_y.scale_value = val;
        axisy_update = true;
    } else {
        axis_x.scale_value = val;
        axisx_update = true;
    }
    update = true;
}

//-------------------------------------------------------------------------
//设置刻度标注的起始值
// xy=0:X轴；xy=1:Y轴
void CChart::set_start_val(int xy, float val)
{
    if(xy) {
        axis_y.start_value = val;
        axisy_update = true;
    } else {
        axis_x.start_value = val;
        axisx_update = true;
    }
    update = true;
}

//-------------------------------------------------------------------------
//获取每页显示点的数目
int CChart::get_dotnum()
{
    int i;
    i = x_dotnum;
    if(!i) i = width_;
    return i;
}





