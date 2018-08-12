/*
 *  view_sxz 表示第x个选项的主界面显示程序
 *  view_sxz_key 表示第x个选项的主界面键盘处理程序
 *  view_sx_yz 表示第x个选项的界面下第y个选项的显示程序
 *  view_sx_yz_key 表示第x个选项的界面下第y个选项的键盘处理程序
 *
 *  view_syxz 表示在系统界面第x个选项的主界面显示程序
 *  view_syxz_key 表示在系统界面第x个选项的主界面键盘处理程序
 *  view_syx_yz 表示在系统界面第x个选项的界面下第y个选项的显示程序
 *  view_syx_yz_key 表示在系统界面第x个选项的界面下第y个选项的键盘处理程序
 *
 */

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <stdio.h>
#include <unistd.h>

#include "viewset.h"
#include "viewset_menu.h"
#include "../thread/pthread_mng.h"
#include "../device/keyboard.h"
#include "time_cst.h"
#include "../data/data_buffer.h"
#include "display/vga_color.h"

/*------------------------------------------------------------------------------
Description:Edit integer number.
Input:      nst -- 当前编辑的是第几个字符;
            str -- 编辑的字符串的地址；
            limtnum -- 字符个数限值
            sign -- true=signed, false=unsigned
*/
int CSetView::edit_num(int ktype, int nst, char *str, int limtnum, bool sign)
{
    int i, curw;
    curw = 7 + font.space; //光标的宽度，亦即光标的间距

    if (nst < limtnum) {
        if (ktype > 47 && ktype < 59 || ktype == KEY_dot) {
            if (sign && nst == 0 && ktype == 48) ktype = '-';
            str[nst] = ktype;
            str[++nst] = 0;
            cursorx += curw;
        }
    }
    if (ktype == KEY_PHS) { // Cursor move to left and delete char before cursor.
        if (nst) {
            nst--;
            i = nst;
            while (str[i]) {
                str[i] = str[i + 1];
                i++;
            }
            cursorx -= curw;
        }
    } else if (ktype == KEY_OTH) { // Cursor move to right.
        if (str[nst]) {
            nst++;
            cursorx += curw;
        }
    }
    return nst;
}

void CSetView::show_cursor()
{
    int color, y;
    if (!showcursor)
        return;
    color = vgacolor(kVGA_Default);
    y = cursory - font.cn_size;//-1; 2007-5-28
    y = y < 1 ? 1 : y;
    pqm_dis->rectangle_xor(cursorx, y, cursorx + 7 + font.space,
                           cursory, color);
    pqm_dis->refresh(cursorx, y, cursorx + 7 + font.space, cursory);
}

//-------------------------------------------------------------------------
void CSetView::IniMdfyTime(tm *ptm)
{
    time_t timei = time(NULL);
    LocalTime(ptm, &timei);

    sprintf(key_buf[0], "%i", ptm->tm_year + 1900);
    sprintf(key_buf[1], "%i", ptm->tm_mon + 1);
    sprintf(key_buf[2], "%i", ptm->tm_mday);
    sprintf(key_buf[3], "%i", ptm->tm_hour);
    sprintf(key_buf[4], "%i", ptm->tm_min);
    sprintf(key_buf[5], "%i", ptm->tm_sec);

    for (int i = 0; i < 6; i++) {
        buf_pt[i] = 0;
    }
    buf_sel_ = 0;
}

/* -----------------------------------------------------------------------------
Description:Select index of list
Input:      ktype -- code of keyboard
            nst -- number of option be selected
            limtnum -- total number of options
----------------------------------------------------------------------------- */
int CSetView::select_lst(int ktype, int nst, int limtnum)
{
    if (ktype == KEY_8) {
        if (nst > 0)
            nst--;
        else
            nst = limtnum - 1;
    } else if (ktype == KEY_2) {
        nst ++;
        nst %= limtnum;
    }
    return nst;
}

//-------------------------------------------------------------------------
//Description: 校验时间的正确性，把其修正到正确范围
void CSetView::verify_time(tm * ptm)
{
    sscanf(key_buf[0], "%hui", &ptm->tm_year);
    sscanf(key_buf[1], "%hui", &ptm->tm_mon);
    sscanf(key_buf[2], "%hui", &ptm->tm_mday);
    sscanf(key_buf[3], "%hui", &ptm->tm_hour);
    sscanf(key_buf[4], "%hui", &ptm->tm_min);
    sscanf(key_buf[5], "%hui", &ptm->tm_sec);
    if (ptm->tm_year > 2037) ptm->tm_year = 2037;
    if (ptm->tm_year < 1971) ptm->tm_year = 1971;
    if (ptm->tm_mon > 12) ptm->tm_mon = 12;
    if (ptm->tm_mday > month_day[ptm->tm_mon - 1]) {
        ptm->tm_mday = month_day[ptm->tm_mon - 1];
    }
    if (ptm->tm_hour > 23) ptm->tm_hour = 23;
    if (ptm->tm_min > 59) ptm->tm_min = 59;
    if (ptm->tm_sec > 59) ptm->tm_sec = 59;
    ptm->tm_year -= 1900;
    ptm->tm_mon -= 1;
}

/*!
Description:Show power on&off time
*/
void CSetView::Show01Time()
{
    time_t tm_ary[10];  //Power on&off time

    num_onoff = data_buffer().GetPower01Time(tm_ary, 10);

    int x = left_;
    int y = top_ + (font.cn_size + font.space) * 12 / 10;
    clear();
    show_frame();
    pqm_dis->puts(" 序 号  最近10次开关机时间", x, y, font.color, font.space);
    int n, stp;
    if (num_onoff >= buf_pt[0] * 8 + 8) n = 8;
    else n = num_onoff - buf_pt[0] * 8;
    stp = (top_ + height - y - font.space - 2) / 8;
    y += (font.cn_size + font.space) * 11 / 10;
    tm tmi;
    int i, j;
    char str[64];
    for (i = 0; i < n; i++) {
        j = (buf_pt[0] * 8 + i);
        LocalTime(&tmi, &tm_ary[j]);
        int m = sprintf(str, " %2d%s  ", j + 1, NoYes[i % 2]);
        strftime(&str[m], 20, "%Y-%m-%d, %H:%M:%S", &tmi);
        pqm_dis->puts(str, x, y + stp * i, font.color, font.space);
    }
    pqm_dis->refresh(left_, top_, left_ + width, top_ + height);
}

/* -----------------------------------------------------------------------------
Description:Convert bit of val to string. Example:0x69 >>> 0,3,5,6 + offset
Input:      val, offset
Output:     str
----------------------------------------------------------------------------- */
void CSetView::Bit2Str(char *str, long val, int offset)
{
    str[0] = 0;
    char stri[4];
    int cnt = 0;
    for (int i = 0; i < 32; i++) {
        if (val >> i & 1) {
            sprintf(stri, "%d,", i + offset);
            strcat(str, stri);
            cnt++;
        }
        if (cnt >= 7) break;
    }
    if (cnt) str[strlen(str) - 1] = 0;
}

/*!
Description:Convert between frequency save space & index

    Input:  type -- 0 = convert from save space to index
                    1 = convert from index to save space
    Retrun: type=0:index, type=1:save space
*/
int CSetView::Freq2Indx(int data, int type)
{
    if (type) {
        switch (data) {
            case 3:
                return 1;
            case 10:
                return 2;
            case 60:
                return 3;
            case 180:
                return 4;
            case 600:
                return 5;
            default:
                return 0;
        }
    } else {
        switch (data) {
            case 1:
                return 3;
            case 2:
                return 10;
            case 3:
                return 60;
            case 4:
                return 180;
            case 5:
                return 600;
            default:
                return 1;
        }
    }

}

/*!
If setting be locked

    Retrun: true=locked, false=unlocked
*/
bool CSetView::Locked()
{
    if (ps_err_cnt_ == 0) {
        time_t tm = time(NULL);
        if (tm - lock_t_ < 60) {
            return true;
        } else {
            ps_err_cnt_ = 5;
        }
    }
    return false;
}

/*!
Show audit log record list

    Input:  type -- 0=warning, 1=alarm
*/
void CSetView::ShowAuditEvent(int type, int x0, int y0)
{
    clear();
    show_frame();

    AuditEvntInf inf[10];
    int n = 0;
    if (type == 0) {
        //n = usr_mng().GetWarnEvt(10, inf);
        for (int i = 0; i < n; i++) {
            pqm_dis->puts(inf[i].str, x0 + 4, i * 16 + y0 + 16, font.color, font.space - 2);
        }
    } else {
        //n = usr_mng().GetAlarmEvt(10, inf);
        for (int i = 0; i < n; i++) {
            pqm_dis->puts(inf[i].str, x0 + 4, i * 16 + y0 + 16, font.color, font.space - 2);
        }
        //usr_mng().reset_audit_alarm();
    }

    refresh();
}

/*!
*/
void CSetView::RunIniSys(int type)
{
    int x0 = pop_box_.x0;
    int y0 = pop_box_.y0;
    int x1 = pop_box_.x1;
    int y1 = pop_box_.y1;
    pqm_dis->clear(x0, y0, x1, y1);
    pqm_dis->rectframe(x0, y0, x1, y1, vgacolor(kVGA_HiBlue));
    pqm_dis->puts("开始...", x0 + 20, y0 + 20, font.color, font.space);
    pqm_dis->refresh(x0, y0, x1, y1);
    
    if (ResetDefaultPara==type) {
        messageq_gui().PushCtrlSig(data_idx_, 8, 0);   //restore to default parameter
        para_update_ = true;
        daram_update = true;
        eew_update_ = 2;
        sleep(1);
    } else if (SysIni==type) {  //initailize system
        messageq_gui().PushCtrlSig(data_idx_, 8, 1);
        sleep(5);
    }

    pqm_dis->clear(x0+20, y0, x1, y0+24);
    pqm_dis->puts("完成!", x0 + 20, y0 + 20, font.color, font.space);
    pqm_dis->refresh(x0+20, y0, x1, y0+24);
    sleep(1);
}



