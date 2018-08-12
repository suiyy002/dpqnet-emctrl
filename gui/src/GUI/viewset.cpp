/* -----------------------------------------------------------------------------
	view_set
		|---- view_init
		|		`---- view_sxxini
		`-------- view_sxx
	view_set_key
		|---- view_sxxini_key
		`---- view_sxx_key
----------------------------------------------------------------------------- */

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <stdio.h>
#include <unistd.h>

#include "viewset.h"
#include "viewset_menu.h"
#include "../thread/pthread_mng.h"
#include "../device/device.h"
#include "../data/data_buffer.h"
#include "../Version.h"
#include "utility.h"
#include "time_cst.h"
#include "display/vga_color.h"

//-------------------------------------------------------------------------
CSetView::CSetView(int w, int h)
{
    left_ = 0;
    top_ = 0;
    width = w;
    height = h;
	if (pqm_dis->disbufw()==480) {  //480*272
        font.asc_size = 16;
        font.cn_size = 14;
        font.space = 2;
    } else {    //320*240
        font.asc_size = 12;
        font.cn_size = 12;
        font.space = 1;
    }

    font.color = vgacolor(kVGA_Default);
    color = vgacolor(kVGA_Blue);
    frame_visible = true; //是否显示边框

    ini_para();
    showcursor = false;

    phrm_rcd_buf = new char*[8];

    int i;
    char **cpi = phrm_rcd_buf;
    for (i = 0;i < 8;i++) {
        *cpi = NULL;
        cpi ++;
    }

    //设置GPS对时的相关参数
    if (data_buffer().sys_para()->gps_single_type() == 0 
        || data_buffer().sys_para()->gps_single_type() == 2) {
        i = data_buffer().sys_para()->gps_pulse_type();
        if (i > 1) i = 0;
        data_buffer().SetGpsParam(i, data_buffer().sys_para()->proof_time_intr(i));
    } else if (data_buffer().sys_para()->gps_single_type() == 1
         || data_buffer().sys_para()->gps_single_type() == 3) { //使能脉冲不起作用
        data_buffer().SetGpsParam(2, 0);
    }
    ps_err_cnt_ = 5;
}

//-------------------------------------------------------------------------
CSetView::~CSetView()
{

}

//-------------------------------------------------------------------------
void CSetView::ini_para()
{
    int num1, num2;
    curr_cmd_ = VW_LOGIN;
    force_exit_ = false;
    para_update_ = false;
    netpara_update = false;
    ntppara_update = false;
    eew_update_ = 0;
    comm_update = false;
    daram_update = false;
    socket_update = false;
    m_serial_prtcl_update = false;
    m_socket_prtcl_update = false;
    gps_pulse_update = false;
    srand((unsigned)time(NULL));
    num1 = rand() % (10 - 1) + 1;
    num2 = rand() % 1000000;
    rand_num = num1 * 1000000 + num2;

    for (int i = 0;i < 6;i++) {
        key_buf[i][0] = 0;
        buf_pt[i] = 0;
    }
    
    const SSetMenu *psetmenu = &ViewLOGINMenu;
    menu_x_ = left_ + (width - psetmenu->width * (font.cn_size + font.space)) / 2;
    Item_x_[0] = menu_x_ + psetmenu->pmenu_item[0].val_x * (font.cn_size + font.space);
    Item_y_[0] = top_ + (height - font.cn_size - font.space)/2;
    cursorx = Item_x_[0];
    cursory = Item_y_[0];
    if (psetmenu->pmenu_item[0].tag&0x2) showcursor = true;
    nd_rfrsh_ = true;
    pop_box_.x0 = left_ + width/10;
    pop_box_.y0 = top_ + height/4;
    pop_box_.x1 = pop_box_.x0 + width*8/10;
    pop_box_.y1 = pop_box_.y0 + height*2/4;
    reset_stack();
}

/*------------------------------------------------------------------------------
Description:设置界面显示初始化
Input:      type - 界面类型
*/
void CSetView::view_init(int type)
{
    int i, stp;
    const SSetMenu *psetmenu;
    int lines = 7; //当前界面按照几行来排版
    int hide_nm = 0; //隐藏菜单项的数目
    menu_num_ = 0;
    have_title_ = false;
    switch (type) {
        case VW_LOGIN:
            psetmenu = &ViewLOGINMenu;
            lines = 6;
            break;
        case VW_S0:
            hide_nm = 3;
            psetmenu = &ViewS0Menu;
            if (EQUIP_TYPE<kEEWNet200) {
                psetmenu->pmenu_item[5].tag |= 0x11;
            } else {
                psetmenu->pmenu_item[5].tag &= 0xffee;
            }
            break;
        case VW_SY0:
            hide_nm = 5;
            psetmenu = &ViewSy0Menu;
            break;
        case VW_AUDIT:
            hide_nm = 3;
            psetmenu = &ViewAuditMenu;
            break;
            //----------- 2nd layer menu ---------------------------------------
        case VW_SteadyPara:
            psetmenu = &ViewSteadyMenu;
            break;
        case VW_LINE:
            psetmenu = &ViewLineMenu;
            break;
        case VW_COMM:
            psetmenu = &ViewCommMenu;
            break;
        case VW_OTHERSET:
            psetmenu = &ViewOtherMenu;
            break;
        case VW_CUSTOM:
            psetmenu = &ViewCustomMenu;
            break;
        case VW_Fluctuation:
            psetmenu = &ViewFluctuationMenu;
            break;
        case VW_InterHarm:
            psetmenu = &ViewInterHarmMenu;
            break;
        case VW_AlmEnable:
            psetmenu = &ViewAlmEnableMenu;
            break;
        case VW_EEW:
            psetmenu = &ViewEEWMenu;
            if (EQUIP_TYPE==kEEWNet200) {
                psetmenu->pmenu_item[0].tag |= 0x10;
                psetmenu->pmenu_item[1].tag |= 0x10;
                psetmenu->pmenu_item[2].tag |= 0x10;
                psetmenu->pmenu_item[3].tag |= 0x10;
                psetmenu->pmenu_item[4].tag &= 0xffee;
            } else {
                psetmenu->pmenu_item[0].tag &= 0xffee;
                psetmenu->pmenu_item[1].tag &= 0xffee;
                psetmenu->pmenu_item[2].tag &= 0xffee;
                psetmenu->pmenu_item[3].tag &= 0xffee;
                psetmenu->pmenu_item[4].tag |= 0x10;
            }
            break;
        case VW_HIDE:
            psetmenu = &ViewHideMenu;
            break;
        case VW_VERIFY:
            psetmenu = &ViewVerifyMenu;
            break;
        case VW_SYSPARA:
            hide_nm = 2;
            psetmenu = &ViewSysparaMenu;
            break;
        case VW_INIT:
            hide_nm = 1;
            psetmenu = &ViewSysInitMenu;
            break;
        case VW_SYS_HIDE:
            psetmenu = &ViewSysHideMenu;
            break;
        case VW_SYS_HIDE2:
            psetmenu = &ViewSysHide2Menu;
            break;
        case VW_RecWavePara:
            psetmenu = &ViewRecWaveMenu;
            break;
        case VW_ShortRMSVVR: //VW_TranstSet:
            psetmenu = &ViewShortVVRMenu;
            break;
        case VW_ShortRMSIVR:    //VW_CurTranstSet:
            psetmenu = &ViewShortIVRMenu;
            break;
        case VW_SteadyTrigPara:
            psetmenu = &ViewStdyTrgMenu;
            break;
        case VW_GpsParamSet:
            psetmenu = &ViewGPSMenu;
            break;
        case VW_CvtModify:
            psetmenu = &ViewCVTMenu;
            break;
        case VW_SetResRatio:
            psetmenu = &ViewResRatioMenu;
            break;
        case VW_VoltLinearity:
            psetmenu = &ViewLinearityMenu;
            break;
        case VW_RcdSpaceSaveType:
            psetmenu = &ViewRcdSpaceMenu;
            break;
        case VW_SeeCharacterDCValue:
            psetmenu = &ViewDCValueMenu;
            break;
        case VW_SetHarmLimit:
            psetmenu = &ViewSetHlmtMenu;
            break;
        case VW_CvtModifyK:
            psetmenu = &ViewCVTKMenu;
            break;
        case VW_HmCurrLimit:
            psetmenu = &ViewCLimitMenu;
            have_title_ = true;
            ptitle = "谐波阶次    允许值";
            tiltle_height_ = (font.cn_size + font.space)*12/10;
            break;
        case VW_AuditWarn:
            ShowAuditEvent(0, left_, top_);
            return;
        case VW_AuditAlarm:
            ShowAuditEvent(1, left_, top_);
            return;
        case VW_raDisHarm:
            psetmenu = &ViewraDisHmMenu;
            break;
        case VW_raThresh:
            psetmenu = &ViewraThrMenu;
            break;        
        case VW_CapThresh:
            psetmenu = &ViewCapThrMenu;
            break;
        case VW_raParam:
            psetmenu = &ViewraParamMenu;
            break;
        case VW_TrnsfmrParam:
            psetmenu = &ViewTrnsfmrMenu;
            break;
        default:
            return;
    }
    item_totl_ = psetmenu->totl;
    i = 8;
    if (item_totl_) {
        if (item_totl_ > buf_pt[6]*i + i) {
            menu_num_ = i;
        } else {
            menu_num_ = item_totl_ - buf_pt[6] * i;
        }
        lines = 8;
    }
    
    if (!menu_num_)  menu_num_ = psetmenu->count;
    int y;
    int h = height;
    int t = top_;
    if (have_title_) {
        h -= tiltle_height_;
        t += tiltle_height_;
    }
    if (lines==8) {
        stp = h * 10 / (8 * 10 + 4);
        y = t + stp * 1.2;
    } else if (menu_num_ -hide_nm > 6) {
        stp = h * 10 / ((menu_num_ -hide_nm) * 10 + 4); //分为n+0.4等份
        y = t + stp * 1.2;
    } else {
        stp = (h-8) / lines;
        y = top_ + stp * 1.8;
    }
    int len = psetmenu->width;
    menu_x_ = left_ + (width - len * (font.cn_size + font.space)) / 2;
    clear();
    show_frame();
    if (have_title_) {
        pqm_dis->puts(ptitle, menu_x_, t-font.space, font.color, font.space);
    }
    for (i = 0;i < menu_num_;i++) {
        menu2cmd_[i] = psetmenu->pmenu_item[i].cmd;
        menu_tag_[i] = psetmenu->pmenu_item[i].tag;
        Item_x_[i] = menu_x_ + psetmenu->pmenu_item[i].val_x * (font.cn_size + font.space);
        Item_y_[i] = y + stp * i;
        if ( menu_tag_[i]&0x1 ) continue;  //该菜单项不显示
        if ( menu_tag_[i]&0x10 ) font.color = vgacolor(kVGA_HiGrey); //COLOR_GREY;  //该菜单项Disable
        view_sxxini(menu2cmd_[i], psetmenu->pmenu_item[i].name, i + 1, menu_x_, Item_x_[i], Item_y_[i]);
        font.color = vgacolor(kVGA_Default);
    }
    refresh();
}

/*------------------------------------------------------------------------------
Description:设置界面显示处理
*/
void CSetView::view_set()
{
    pqm_dis->set_font(font.cn_size, font.asc_size);
    int x, y, stp, i;
    if (curr_cmd_ < VW_GUI_DILIMITER) { //独立的显示界面
        view_init(curr_cmd_);
        return;     //essential
    }
    if (!nd_rfrsh_) return;
    x = curr_x_;
    y = curr_y_;
    switch (curr_cmd_) {
        case ShowOcTime:
            Show01Time();
            break;
        case ModifyPasswd:
            for (i = 0;i < 3;i++) {
                view_sxx(curr_cmd_, x, cursor_top[i], i);
            }
            break;
        default:
            view_sxx(curr_cmd_, x, y);
            break;
    }
}

/*-------------------------------------------------------------------------
Description:设置界面键盘处理
Return:     1=正常退出, 2=登录错误, 3=force exit 
*/
int CSetView::view_set_key(int ktype)
{
    WorkNode * tosave;
    int i;
    int retb = 0;
    //----- 显示界面处理 ----------------------------
    if (curr_cmd_ < VW_GUI_DILIMITER) {
        if (ktype == KEY_ESC) {
            switch (curr_cmd_) {
                case VW_S0:
                case VW_SY0: {
                    if (para_update_) {

                    }
                    retb = 1;
                    break;
                }
                case VW_AUDIT:
                    retb = 1;
                    break;
                case VW_LOGIN:
                    retb = 2;
                    break;
                case VW_raDisHarm:
                    data_buffer().eew_para()->set_show_harmonic(ra_show_hr_);
                default:
                    curr_cmd_ = popcmd();
                    break;
            }
        } else if (ktype == KEY_PHS) {  //page up
            switch (curr_cmd_) {
                case VW_raDisHarm:
                case VW_raThresh:
                case VW_SetHarmLimit:
                case VW_CvtModifyK:
                case VW_HmCurrLimit:
                    if (buf_pt[6] > 0) buf_pt[6]--;
                    break;
                default:;
            }
        } else if (ktype == KEY_OTH) {  //Page down
            switch (curr_cmd_) {
                case VW_raDisHarm:
                case VW_raThresh:
                case VW_SetHarmLimit:
                case VW_CvtModifyK:
                case VW_HmCurrLimit:
                    if (buf_pt[6]*8 + 8 < item_totl_) buf_pt[6]++;
                    break;
                default:;
            }
        } else if (ktype > KEY_0 && (ktype - KEY_0) <= menu_num_) {
            i = ktype - KEY_1;
            if ( !(menu_tag_[i]&0x10)) { //该菜单项Enable,且可设置
                curr_sn_ = i;
                curr_x_ = cursorx = Item_x_[i];
                curr_y_ = cursory = Item_y_[i];
                view_sxxini_key(menu2cmd_[i], ktype);
                if (menu_tag_[i]&0x2) showcursor = true;  //显示光标
            }
        }
        if (retb) {
            showcursor = false;
        }
        return retb; //essential
    }
    //----- 不切换界面 --------------------------------
    int x = curr_x_;
    view_sxx_key(curr_cmd_, ktype, x);
    if (force_exit_) {
        retb = 3;
    }
    if (retb) showcursor = false;
    return retb;
}

/* -----------------------------------------------------------------------------
Description:显示设置选项
Input:      type,设置项类型
        	name, 菜单或参数项名称
        	row, 指示当前设置项在第几行
        	x1, 当前界面菜单的起始横坐标
        	x2, 当前界面各菜单项设置值的起始横坐标
			y, 当前界面各菜单项的纵坐标
----------------------------------------------------------------------------- */
void CSetView::view_sxxini(int type, const char* name, int row, int x1, int x2, int y)
{
    char *pstr = "";
    char str[36], str1[8];
    float fi, fj, fk;
    long li;
    unsigned int ui;
    signed char ch;
    short si;
    int x = x2;
    switch (type) {
        case VW_RecWavePara:
            if (!trnst_valid_) { //不具有暂态功能
                return;
            }
            break;
        case VoltWarp:
            sprintf(str, "%-4.1f /-%2.1f", data_buffer().sys_para()->Volt_warp(0),
                    -data_buffer().sys_para()->Volt_warp(1));
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case TransEnable:
            ui = data_buffer().sys_para()->transt_monitor();
            sprintf(str, "%s /%s /%s", YesNo[ui&0x1], YesNo[(ui>>1)&0x1], YesNo[(ui>>2)&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case TransCurEnable:
            ui = data_buffer().sys_para()->transt_i_monitor();
            sprintf(str, "%s /%s /%s", YesNo[ui&0x1], YesNo[(ui>>1)&0x1], YesNo[(ui>>2)&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case FluctuationEna:
            ui = data_buffer().sys_para()->fluct_enable();
            sprintf(str, "%s /%s /%s", YesNo[ui&0x1], YesNo[(ui>>1)&0x1], YesNo[(ui>>2)&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case GpsSingleType:
            sprintf(str, "%s ", kGpsSingleType[data_buffer().sys_para()->gps_single_type()]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case GpsPulseType:
            sprintf(str, "%s ", Gps_Pulse_Type[data_buffer().sys_para()->gps_pulse_type()&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kRecWaveFormat:
            sprintf(str, "%s ", RcdWvSavFmt[data_buffer().equip_para()->rcd_wv_fmt()]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case SetBTimeIntr:
            sprintf(str, "%-3d", data_buffer().sys_para()->b_time_intr());
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case SetProofTimeIntr:
            sprintf(str, "%-3d/%d", data_buffer().sys_para()->proof_time_intr(0),
                    data_buffer().sys_para()->proof_time_intr(1));
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case StartCurEn:
            ui = data_buffer().sys_para()->start_cur_en();
            sprintf(str, "%s /%s /%s", YesNo[ui&0x1], YesNo[(ui>>1)&0x1], YesNo[(ui>>2)&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case StartCurLimit:
            fi = data_buffer().sys_para()->sc_limit();
            sprintf(str, "%-5.1f", fi / 10);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case FluctuationDb:
            fi = data_buffer().sys_para()->fluct_db(); 
            sprintf(str, "%5.3f", fi / 1000);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kInterHarmDb:
            fi = data_buffer().sys_para()->inter_limit(); 
            sprintf(str, "%4.1f", fi / 10);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case TransTbEnable:
            ui = data_buffer().sys_para()->transt_tb_enable();
            sprintf(str, "%s /%s /%s", YesNo[ui&0x1], YesNo[(ui>>1)&0x1], YesNo[(ui>>2)&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case TransLimit:
            fi = data_buffer().sys_para()->vvr_limit(0);
            fj = data_buffer().sys_para()->vvr_limit(1);
            fk = data_buffer().sys_para()->vvr_limit(2);
            sprintf(str, "%-5.1f/%4.1f/%4.1f", fi / 10, fj / 10, fk/10);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case TransCurLimit:
            fi = data_buffer().sys_para()->ivr_limit(0);
            fj = data_buffer().sys_para()->ivr_limit(1);
            sprintf(str, "%-6.1f/%2.1f", fi / 10, fj / 10);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case TransTbLimit:
            fi = data_buffer().sys_para()->tb_high_limit();  ///转成实型才能进行除法运算
            fj = data_buffer().sys_para()->tb_low_limit();
            sprintf(str, "%-6.1f/%2.1f", fi / 10, fj / 10); ///小于5,2 时按5,2 算，大于时按实际的算。
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case TransManualRec:
            ui = data_buffer().sys_para()->manual_rec_enable();
            sprintf(str, "%s", YesNo[ui&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case CvtModifyEn:
            ui = data_buffer().sys_para()->cvt_modify_en();
            sprintf(str, "%s", YesNo[ui&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case TransEndNum:
            sprintf(str, "%d", data_buffer().sys_para()->transt_end_num());
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kAuditLogSize:
            sprintf(str, "%d", data_buffer().equip_para()->audtlog_size());
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case ImbalanceLimit:
            fi = data_buffer().sys_para()->unbalance_thr(0);
            fj = data_buffer().sys_para()->unbalance_thr(1);
            sprintf(str, "%-6.1f/%2.1f", fi / 10, fj / 10);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case NegativeILimit:
            fi = data_buffer().sys_para()->neg_sequence_Ithr();
            sprintf(str, "%3.1f A", fi / 10);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kHarmonic10Cyc:
            fi = data_buffer().equip_para()->harmnm10cyc();
            sprintf(str, "%3.1f", fi / 10);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case FrequecyLimit:
            fi = data_buffer().sys_para()->freq_limit(0);
            fj = data_buffer().sys_para()->freq_limit(1);
            sprintf(str, "%-6.2f/%2.2f", fi / 100, fj / 100);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kCapLifeThr:
            fi = data_buffer().eew_para()->life_thr();
            si = data_buffer().eew_para()->life_thr_cnt();
            sprintf(str, "%-5.2f /%d次", fi / 1000, si);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kCapVolt1Thr:
            fi = data_buffer().eew_para()->volt1_thr();
            si = data_buffer().eew_para()->voltdur1_thr();
            sprintf(str, "%-5.2f /%d秒", fi / 1000, si);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kCapVolt2Thr:
            fi = data_buffer().eew_para()->volt2_thr();
            si = data_buffer().eew_para()->voltdur2_thr();
            sprintf(str, "%-5.2f /%d秒", fi / 1000, si);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kCapCurrThr:
            fi = data_buffer().eew_para()->curr_thr();
            sprintf(str, "%-5.2f", fi / 1000);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kCapCapThr:
            fi = data_buffer().eew_para()->cap_thr();
            sprintf(str, "%-5.2f", fi / 1000);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kCapPeakThr:
            fi = data_buffer().eew_para()->peakv_thr();
            sprintf(str, "%-5.2f", fi / 1000);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kCapGamaThr:
            fi = data_buffer().eew_para()->Th();
            sprintf(str, "%-5.2f", fi / 1000);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kTrnsfmrPec:
            fi = data_buffer().eew_para()->Pec_rpu();
            sprintf(str, "%-5.2f", fi / 1000);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kTransformerMs:
            fi = data_buffer().eew_para()->Ms();
            sprintf(str, "%-5.2fMVA", fi / 100);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kTrnsfmrIn:
            fi = data_buffer().eew_para()->In();
            sprintf(str, "%-5.2fA", fi / 100);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kCapNmnlMc:
            fi = data_buffer().eew_para()->Mc();
            sprintf(str, "%-5.2fkvar", fi / 100);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kCapNmnlUc:
            sprintf(str, "%dV", data_buffer().eew_para()->Uc());
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kCapReactancex:
            fi = data_buffer().eew_para()->x();
            sprintf(str, "%-5.2f%%", fi / 100);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kImpedanceBeta:
            fi = data_buffer().eew_para()->beta();
            sprintf(str, "%-5.2f°", fi / 100);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kCapCharctHr:
            Bit2Str(str, data_buffer().eew_para()->charct_hr(), 2);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case VW_raDisHarm:
            Bit2Str(str, ra_show_hr_, 2);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case PstLimit:
            fi = data_buffer().sys_para()->pst_limit();
            fj = data_buffer().sys_para()->plt_limit();
            sprintf(str, "%-5.2f /%3.2f", fi/100, fj/100);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case ShortCap:
            sprintf(str, "%4.1f MVA", data_buffer().sys_para()->Short_cap());
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case UserCap:
            sprintf(str, "%4.1f MVA", data_buffer().sys_para()->User_cap());
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case SuppCap:
            sprintf(str, "%4.1f MVA", data_buffer().sys_para()->Supp_cap());
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case CLimitValue: { //查询电流限值子界面
            //if (row>1) {
                ui = buf_pt[6] * 8 + row + 1;
                //printf("buf");//for debug
                //fi = harmfunc->hrmi_limit(ui);
                fi = data_buffer().GetHrmILimit(ui);
                sprintf(str, "%2i次:     %5.2fA", ui, fi);
                pqm_dis->puts(str, x, y, font.color, font.space);
            //}
            return;
        }
        case kRAThreshold: { //γ-α阈值查询子界面
            //if (row>1) {
                ui = buf_pt[6] * 8 + row - 1;
                //printf("buf");//for debug
                fi = data_buffer().eew_ra_thr(ui, 0);
                fj = data_buffer().eew_ra_thr(ui, 1);
                fk = data_buffer().eew_ra_thr(ui, 2);
                sprintf(str, "%2i次: %5.2f%%  %5.2f  %5.3f ", ui+2, fi/100, fj/100, fk/1000);
                pqm_dis->puts(str, x, y, font.color, font.space);
            //}
            return;
        }
        case kraDisHmSub: { //γ-α显示谐波界面
            ui = buf_pt[6] * 8 + row -1;
            sprintf(str1, "%d次:", ui + 2);
            pstr = str1;
            if (ra_show_hr_&(1<<ui)) {
                pqm_dis->puts("√", x, y, font.color, font.space);
            }
            break;
        }
        case CvtModifyKSub: {   //设置CVT修正系数子界面
            ui = buf_pt[6] * 8 + row;
            sprintf(str1, "%d次:", ui + 1);
            fi = data_buffer().sys_para()->cvt_modify(ui-1);
            sprintf(str, "%-6.3f", fi / 1000);
            pstr = str1;
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        }
        case SetHarmLimitSub: { //设置谐波限值子界面
            ui = buf_pt[6] * 8 + row - 1;
            if (!ui) { //第1个值为THDu
                sprintf(str1, "%s", "THDu:");
                fi = data_buffer().sys_para()->harm_ulimit(ui);
                sprintf(str, "%-6.1f%%", fi / 10);
            } else {
                sprintf(str1, "%d次:", ui+1);
                fi = data_buffer().sys_para()->harm_ulimit(ui);
                fj = data_buffer().sys_para()->harm_ilimit(ui-1);
                sprintf(str, "%-6.1f%% /%3.1f A", fi / 10, fj / 10);
            }
            pstr = str1;
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        }
        case PTScale:
            sprintf(str, "%-7i/%i", data_buffer().GetRatio(0),
                    data_buffer().GetRatio(1));
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case CTScale:
            sprintf(str, "%-6i/%i", data_buffer().GetRatio(2),
                    data_buffer().GetRatio(3));
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case ZeroInputThr:
            sprintf(str, "%-2iV /%imA", data_buffer().equip_para()->zero_thr(0),
                    data_buffer().equip_para()->zero_thr(1));
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case VoltLvl:
            sprintf(str, "%s ", ULevSet[data_buffer().sys_para()->CUlevel()]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kUserName:
            sprintf(str, "%s ",  data_buffer().usr_name());
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case FreqRcdSpace:
            sprintf(str, "%d秒", data_buffer().sys_para()->freq_rcd_space());
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kFreqEvalTm:
            pqm_dis->puts(FreqEvalTmSlct[data_buffer().equip_para()->freq_evaltm()], x, y, font.color, font.space);
            break;
        case kClockSource:
            sprintf(str, "%s ", ClockSourceSlct[data_buffer().equip_para()->clock_src()]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            if (data_buffer().equip_para()->clock_src()) {   //System
                ViewHideMenu.pmenu_item[5].tag &= 0xffef;
            } else {
                ViewHideMenu.pmenu_item[5].tag |= 0x10;
            }
           break;
        case AdSampleRate:
            sprintf(str, "%s ", AdSmplRtSlct[data_buffer().equip_para()->ad_sample_rate()]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kComtradeSvPath:
            sprintf(str, "%s ", kCmtrdSvPathSlct[data_buffer().equip_para()->cmtrd_sv_path()]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kRecWaveSmplRt:
            sprintf(str, "%s ", WvSmplRtSlct[data_buffer().equip_para()->wav_sample_rate()]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case ConnectType:
            sprintf(str, "%s ", CnctTpSlct[data_buffer().sys_para()->connect_t()&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case HrmRcdSaveType:
            sprintf(str, "%s ", HrmRcdTypeSet[data_buffer().sys_para()->hrm_save_type()&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case FreqSaveType:
            sprintf(str, "%s ", HrmRcdTypeSet[data_buffer().sys_para()->freq_save_type()&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case VoltWarpRcdSpace:
            sprintf(str, "%d秒", data_buffer().sys_para()->voltdv_rcd_space());
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case VoltWarpSaveType:
            sprintf(str, "%s ", HrmRcdTypeSet[data_buffer().sys_para()->voltdv_save_type()&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case ImbalanceRcdSpace:
            sprintf(str, "%d秒", data_buffer().sys_para()->unbalance_rcd_space());
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case ImbalanceSaveType:
            sprintf(str, "%s ", HrmRcdTypeSet[data_buffer().sys_para()->unbalance_save_type()&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case SetLimitType:
            sprintf(str, "%s ", LimitTypeSet[data_buffer().sys_para()->limit_type()&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kInterHmGroup:
            sprintf(str, "%s ", kIntrHarmGrpt[data_buffer().sys_para()->inharm_type()&1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case CurrClampType:
            if (!data_buffer().equip_para()->current_clamp_en()&0x1) {
                return;
            }
            sprintf(str, "%s ", CClampTypeSet[data_buffer().equip_para()->current_clamp_type()]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case LCMDelayTime:
            sprintf(str, "%i分钟", data_buffer().equip_para()->lcm_dely_time());
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case TranstRcdTime:
            if (!trnst_valid_) { //不具有暂态功能
                return;
            }
            sprintf(str, "%i秒[%d]", data_buffer().equip_para()->transt_rcd_time(), 
                        data_buffer().equip_para()->transt_max() - 3);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kCapW24hThr:
            sprintf(str, "%d秒", data_buffer().eew_para()->w24h_thr());
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case HarmRcdSpace:
            sprintf(str, "%d秒", data_buffer().sys_para()->harm_rcd_space());
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kHarmRecSvMax:
            sprintf(str, "%d", data_buffer().equip_para()->harm_rec_svmax());
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case UnitNum:
            sprintf(str, "%i", data_buffer().equip_para()->device_num());
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kMdfyEvnStm:
            sprintf(str, "%d", data_buffer().equip_para()->modify_evn_stm());
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case ResetHysTime:
            sprintf(str, "%i", data_buffer().equip_para()->reset_hyst_time());
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kFilterCP:
            sprintf(str, "%i", data_buffer().equip_para()->fltr_cpx());
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kTimeDiff:
            sprintf(str, "%i", data_buffer().equip_para()->time_diff());
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case BaudRateSet:
            sprintf(str, "%6li ", data_buffer().equip_para()->BaudRate(0));
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case PhsAdjEnable:
            ui = data_buffer().equip_para()->phs_adj_enable();
            sprintf(str, "%s /%s /%s", YesNo[ui&0x1], YesNo[(ui>>1)&0x1], YesNo[(ui>>2)&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case VorC_AdjEnable:
            ui = data_buffer().equip_para()->vc_adj_enable();
            sprintf(str, "%s /%s ", YesNo[ui&0x1], YesNo[(ui>>1)&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kHarmTrgrEnable:
            sprintf(str, "%s /%s ", YesNo[data_buffer().sys_para()->trigger_enable(2)], 
                            YesNo[data_buffer().sys_para()->trigger_enable(3)]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kUnblcTrgrEnable:
            sprintf(str, "%s /%s ", YesNo[data_buffer().sys_para()->trigger_enable(4)], 
                            YesNo[data_buffer().sys_para()->trigger_enable(5)]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case DeviceSn:
            sprintf(str, "%li", data_buffer().equip_para()->device_sn());
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case SmpVltScale:
            sprintf(str, "%5.5f", data_buffer().equip_para()->sample_pt());
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case SmpCurScale:
            sprintf(str, "%6.6f", data_buffer().equip_para()->sample_ct());
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kSysTimeError:
            sprintf(str, "%d", data_buffer().equip_para()->time_err());
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case VALineCoef:
        case VBLineCoef:
        case VCLineCoef:
            li = data_buffer().equip_para()->v_line_coef(type-VALineCoef);
            sprintf(str, "%d", li);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case VAResRatio:
        case VBResRatio:
        case VCResRatio:
            fi = data_buffer().equip_para()->v_res_ratio(type-VAResRatio);
            sprintf(str, "%5.4f", fi / 10000);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case CAResRatio:
        case CBResRatio:
        case CCResRatio:
            fi = data_buffer().equip_para()->c_res_ratio(type-CAResRatio);
            sprintf(str, "%5.4f", fi / 10000);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case VADCValue:
        case VBDCValue:
        case VCDCValue:
        case CADCValue:
        case CBDCValue:
        case CCDCValue:
            si = data_buffer().equip_para()->character_dc(type-VADCValue);
            sprintf(str, "%d", si);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case VorCDatum:
            fi = data_buffer().equip_para()->v_datum();
            fj = data_buffer().equip_para()->c_datum();
            sprintf(str, "%-5.2f /%4.2f", fi / 100, fj / 100);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case TranstTolTime:
            if (!trnst_valid_) { //不具有暂态功能
                return;
            }
            sprintf(str, "%i分钟[%d]", data_buffer().equip_para()->transt_tol_time(), 
                        data_buffer().equip_para()->transt_max() - 3);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case ModifyTime:
            IniMdfyTime(&time_set_);
            strftime(str, 20, "%Y-%m-%d,%H:%M:%S", &time_set_);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case SocketServerPort:
            sprintf(str, "%d", data_buffer().equip_para()->socket_server_port());
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case IPAddrSet:
            data_buffer().equip_para()->get_ip(str);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case NTPServerIP:
            data_buffer().equip_para()->get_ntp_ip(str);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case NetMaskSet:
            data_buffer().equip_para()->get_nmask(str);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case GateWaySet:
            data_buffer().equip_para()->get_gateway(str);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case SetMacAddr:
            data_buffer().equip_para()->get_mac_addr(str);
            ui = sscanf( str, "%x:%x:%x:%x:%x:%x", &hwaddr[0], &hwaddr[1], &hwaddr[2],
                    &hwaddr[3], &hwaddr[4], &hwaddr[5] );
            if (ui != 6) {
                sprintf(str, "%s", "NULL");
                hwaddr[4] = 0;
                hwaddr[5] = 0;
            } else {
                sprintf(str, "%-3d:%d", hwaddr[4], hwaddr[5]);
            }
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kTimeZone:
            sprintf(str, "%3s  /%s", TimeZoneSelect[12-data_buffer().equip_para()->timezone()], 
                        GPSTZTypeSelect[data_buffer().equip_para()->timezone(1)]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kStdyTrgrEnable:
            ui = data_buffer().sys_para()->trigger_enable(0);
            sprintf(str, "%s", YesNo[ui&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kFreqTrgrEnable:
            ui = data_buffer().sys_para()->trigger_enable(1);
            sprintf(str, "%s", YesNo[ui&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kFixFreqSmpl:
            ui = data_buffer().equip_para()->fix_smpl();
            sprintf(str, "%s", YesNo[ui&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kHeartbeat61850:
            ui = data_buffer().equip_para()->hrtbt_61850();
            sprintf(str, "%s", YesNo[ui&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kVoltDvTrgrEnable:
            ui = data_buffer().sys_para()->trigger_enable(6);
            sprintf(str, "%s", YesNo[ui&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case DebugEnable:
            ui = data_buffer().equip_para()->debug_enable();
            sprintf(str, "%s", YesNo[ui&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case PstMdfyEnable:
            ui = data_buffer().equip_para()->Pst_mdfy_enable();
            sprintf(str, "%s", YesNo[ui&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case PstEnable:
            ui = data_buffer().equip_para()->pst_enable();
            sprintf(str, "%s", YesNo[ui&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kSecurityEn:
            ui = data_buffer().equip_para()->security_en();
            sprintf(str, "%s", YesNo[ui&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kSignalSimuEn:
            ui = data_buffer().equip_para()->signl_simu_en();
            sprintf(str, "%s", YesNo[ui&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case SaveWaveEnable:
            ui = data_buffer().equip_para()->save_wave_en();
            sprintf(str, "%s", YesNo[ui&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kInterHarmEna:
            ui = data_buffer().sys_para()->inter_enable();
            sprintf(str, "%s", YesNo[ui&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kHarmAlmEna:
            ui = data_buffer().equip_para()->alarm_enable(kAlmHarmEn);
            sprintf(str, "%s", YesNo[ui==0?0:1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kFreqAlmEna:
            ui = data_buffer().equip_para()->alarm_enable(kAlmFreqEn);
            sprintf(str, "%s", YesNo[ui==0?0:1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kPstAlmEna:
            ui = data_buffer().equip_para()->alarm_enable(kAlmPstEn);
            sprintf(str, "%s", YesNo[ui==0?0:1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kUnblcAlmEna:
            ui = data_buffer().equip_para()->alarm_enable(kAlmUnblcEn);
            sprintf(str, "%s", YesNo[ui==0?0:1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kVoltDvAlmEna:
            ui = data_buffer().equip_para()->alarm_enable(kAlmVoltdvEn);
            sprintf(str, "%s", YesNo[ui==0?0:1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kHarmRecSvEn:
            ui = data_buffer().equip_para()->harm_rec_sven();
            sprintf(str, "%s", YesNo[ui&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case Aggregation10min:
            ui = data_buffer().equip_para()->aggrgt_time_en();
            sprintf(str, "%s", YesNo[ui&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case CurrClampEnable:
            ui = data_buffer().equip_para()->current_clamp_en();
            sprintf(str, "%s", YesNo[ui&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case AdjustZeroEnable:
            ui = data_buffer().equip_para()->real_wave_zero();
            sprintf(str, "%s", NoYes[ui&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        default:
            break;
    }
    x = x1;
    if (row) {
        sprintf(str, "%i.%s%s", row, name, pstr);
    } else {
        sprintf(str, " %s%s", name, pstr);
    }
    pqm_dis->puts(str, x, y, font.color, font.space);
}

/*------------------------------------------------------------------------------
Description:初始化设置项对应参数的中间变量及光标位置
Input:     type - 设置项类型
      ktype - 键盘输入码，表示哪个按键被按下。范围[KEY_1,KEY_9]
*/
void CSetView::view_sxxini_key(int type, int ktype)
{
    int i;
    int x, y;
    float fi, fj, fk;
    unsigned int ui;
    short si;
    int row = ktype - KEY_0;
    buf_sel_ = 0;
    for (i = 0;i < 6;i++) {
        buf_pt[i] = 0;
    }
    buf_sel_sz_ = 1; ///表示每一选项有几个设置项
//------------- 初始化命令参数数据 -------------------------------------------
    switch (type) {
        case kraDisHmSub:
            ui = buf_pt[6] * 8 + row - 1;
            ra_show_hr_ ^= (1<<ui);
            eew_update_ = 1;
            return;
        case VW_RecWavePara:
            if (trnst_valid_) { //具有暂态功能
                break;
            }
        case VW_RESERVE1:
        case VW_SYS_RSV1:
        case VW_SYS_RSV2:
        case VW_SYS_RSV3:
        case VW_SYSPARA_RSV1:
        case VW_SYSPARA_RSV2:
            return;
        case VoltWarp:
            sprintf(key_buf[0], "%3.1f", data_buffer().sys_para()->Volt_warp(0));
            sprintf(key_buf[1], "%3.1f", -data_buffer().sys_para()->Volt_warp(1));
            buf_sel_sz_ = 2;
            break;
        case TransEnable: //暂态监测使能
            ui = data_buffer().sys_para()->transt_monitor();
            buf_sel_sz_ = 3;
            buf_pt[0] = ui & 0x1;
            buf_pt[1] = (ui >> 1) & 0x1;
            buf_pt[2] = (ui >> 2) & 0x1;
            break;
        case TransCurEnable: //暂态电流监测使能
            ui = data_buffer().sys_para()->transt_i_monitor();
            buf_sel_sz_ = 3;
            buf_pt[0] = ui & 0x1;
            buf_pt[1] = (ui >> 1) & 0x1;
            buf_pt[2] = (ui >> 2) & 0x1;
            break;
        case FluctuationEna: //Fluctuation enable
            ui = data_buffer().sys_para()->fluct_enable();
            buf_sel_sz_ = 3;
            buf_pt[0] = ui & 0x1;
            buf_pt[1] = (ui >> 1) & 0x1;
            buf_pt[2] = (ui >> 2) & 0x1;
            break;
        case TransLimit: //暂态门限值
            fi = data_buffer().sys_para()->vvr_limit(0);
            fj = data_buffer().sys_para()->vvr_limit(1);
            fk = data_buffer().sys_para()->vvr_limit(2);
            sprintf(key_buf[0], "%2.1f", fi / 10);
            sprintf(key_buf[1], "%2.1f", fj / 10);
            sprintf(key_buf[2], "%2.1f", fk / 10);
            buf_sel_sz_ = 3;
            break;
        case TransCurLimit: //暂态电流门限值
            fi = data_buffer().sys_para()->ivr_limit(0);
            fj = data_buffer().sys_para()->ivr_limit(1);
            sprintf(key_buf[0], "%2.1f", fi / 10);
            sprintf(key_buf[1], "%2.1f", fj / 10);
            buf_sel_sz_ = 2;
            break;
        case GpsSingleType:
            buf_pt[0] = data_buffer().sys_para()->gps_single_type();
            break;
        case GpsPulseType:
            buf_pt[0] = data_buffer().sys_para()->gps_pulse_type();
            break;
        case kRecWaveFormat:
            buf_pt[0] = data_buffer().equip_para()->rcd_wv_fmt();
            break;
        case SetProofTimeIntr: //GPS-PULSE对时间隔
            sprintf(key_buf[0], "%d", data_buffer().sys_para()->proof_time_intr(0));
            sprintf(key_buf[1], "%d", data_buffer().sys_para()->proof_time_intr(1));
            buf_sel_sz_ = 2;
            showcursor = false;
            break;
        case StartCurEn:
            ui = data_buffer().sys_para()->start_cur_en();
            buf_sel_sz_ = 3;
            buf_pt[0] = ui & 0x1;
            buf_pt[1] = (ui >> 1) & 0x1;
            buf_pt[2] = (ui >> 2) & 0x1;
            break;
        case StartCurLimit:
            fi = data_buffer().sys_para()->sc_limit();
            sprintf(key_buf[0], "%3.1f", fi / 10);
            break;
        case FluctuationDb:
            fi = data_buffer().sys_para()->fluct_db();
            sprintf(key_buf[0], "%5.3f", fi / 1000);
            break;
        case kInterHarmDb:
            fi = data_buffer().sys_para()->inter_limit();
            sprintf(key_buf[0], "%4.1f", fi / 10);
            break;
        case TransTbEnable:
            ui = data_buffer().sys_para()->transt_tb_enable();
            buf_sel_sz_ = 3;
            buf_pt[0] = ui & 0x1;
            buf_pt[1] = (ui >> 1) & 0x1;
            buf_pt[2] = (ui >> 2) & 0x1;
            break;
        case TransTbLimit:
            fi = data_buffer().sys_para()->tb_high_limit();
            fj = data_buffer().sys_para()->tb_low_limit();
            sprintf(key_buf[0], "%2.1f", fi / 10);
            sprintf(key_buf[1], "%2.1f", fj / 10);
            buf_sel_sz_ = 2;
            break;
        case TransManualRec:
            ui = data_buffer().sys_para()->manual_rec_enable();
            buf_pt[0] = ui & 0x1;
            break;
        case CvtModifyEn:
            ui = data_buffer().sys_para()->cvt_modify_en();
            buf_pt[0] = ui & 0x1;
            break;
        case TransEndNum:
            sprintf(key_buf[0], "%d", data_buffer().sys_para()->transt_end_num());
            break;
        case kAuditLogSize:
            sprintf(key_buf[0], "%d", data_buffer().equip_para()->audtlog_size());
            break;
        case ImbalanceLimit: //三相不平衡
            fi = data_buffer().sys_para()->unbalance_thr(0);
            fj = data_buffer().sys_para()->unbalance_thr(1);
            sprintf(key_buf[0], "%2.1f", fi / 10);///把数据保存到key_buf[0]和key_buf[1]中了。
            sprintf(key_buf[1], "%2.1f", fj / 10);
            buf_sel_sz_ = 2; ///选项有两个设置项
            break;
        case NegativeILimit: //Negative sequence current
            fi = data_buffer().sys_para()->neg_sequence_Ithr();
            sprintf(key_buf[0], "%3.1f", fi / 10);
            break;
        case kHarmonic10Cyc:
            fi = data_buffer().equip_para()->harmnm10cyc();
            sprintf(key_buf[0], "%3.1f", fi / 10);
            break;
        case FrequecyLimit: //频率限值
            fi = data_buffer().sys_para()->freq_limit(0);
            fj = data_buffer().sys_para()->freq_limit(1);
            sprintf(key_buf[0], "%2.2f", fi / 100);
            sprintf(key_buf[1], "%2.2f", fj / 100);
            buf_sel_sz_ = 2;
            break;
        case kCapLifeThr: //电容器寿命预警阈值
            fi = data_buffer().eew_para()->life_thr();
            ui = data_buffer().eew_para()->life_thr_cnt();
            sprintf(key_buf[0], "%-5.2f", fi / 1000);
            sprintf(key_buf[1], "%d", ui);
            buf_sel_sz_ = 2;
            break;
        case kCapVolt1Thr: //持续过电压预警1阈值
            fi = data_buffer().eew_para()->volt1_thr();
            ui = data_buffer().eew_para()->voltdur1_thr();
            sprintf(key_buf[0], "%-5.2f", fi / 1000);
            sprintf(key_buf[1], "%d", ui);
            buf_sel_sz_ = 2;
            break;
        case kCapVolt2Thr: //持续过电压预警2阈值
            fi = data_buffer().eew_para()->volt2_thr();
            ui = data_buffer().eew_para()->voltdur2_thr();
            sprintf(key_buf[0], "%-5.2f", fi / 1000);
            sprintf(key_buf[1], "%d", ui);
            buf_sel_sz_ = 2;
            break;
        case kCapCurrThr: //过电流预警阈值
            fi = data_buffer().eew_para()->curr_thr();
            sprintf(key_buf[0], "%-5.2f", fi / 1000);
            break;
        case kCapCapThr: //过容限预警阈值
            fi = data_buffer().eew_para()->cap_thr();
            sprintf(key_buf[0], "%-5.2f", fi / 1000);
            break;
        case kCapPeakThr: //峰值过电压阈值
            fi = data_buffer().eew_para()->peakv_thr();
            sprintf(key_buf[0], "%-5.2f", fi / 1000);
            break;
        case kCapGamaThr: //γ 阈值 Th
            fi = data_buffer().eew_para()->Th();
            sprintf(key_buf[0], "%-5.2f", fi / 1000);
            break;
        case kTrnsfmrPec: //pu of Eddy Current loss
            fi = data_buffer().eew_para()->Pec_rpu();
            sprintf(key_buf[0], "%-5.2f", fi / 1000);
            break;
        case kTransformerMs: //主变短路容量 Ms
            fi = data_buffer().eew_para()->Ms();
            sprintf(key_buf[0], "%-5.2f", fi / 100);
            break;
        case kTrnsfmrIn: //I_N of transformer A
            fi = data_buffer().eew_para()->In();
            sprintf(key_buf[0], "%-5.2f", fi / 100);
            break;
        case kCapNmnlMc:    //电容额定容量 Mc
            fi = data_buffer().eew_para()->Mc();
            sprintf(key_buf[0], "%-5.2f", fi / 100);
            break;
        case kCapNmnlUc:    //电容额定电压 Uc
            sprintf(key_buf[0], "%d", data_buffer().eew_para()->Uc());
            break;
        case kCapReactancex:    //串联电抗器电抗率
            fi = data_buffer().eew_para()->x();
            sprintf(key_buf[0], "%-5.2f", fi / 100);
            break;
        case kImpedanceBeta:    //电源阻抗夹角β
            fi = data_buffer().eew_para()->beta();
            sprintf(key_buf[0], "%-5.2f", fi / 100);
            break;
        case PstLimit: //闪变限值
            fi = data_buffer().sys_para()->pst_limit();
            fj = data_buffer().sys_para()->plt_limit();
            sprintf(key_buf[0], "%3.2f", fi / 100);
            sprintf(key_buf[1], "%3.2f", fj / 100);
            buf_sel_sz_ = 2;
            break;
        case ShortCap:
            sprintf(key_buf[0], "%3.1f", data_buffer().sys_para()->Short_cap());
            break;
        case UserCap:
            sprintf(key_buf[0], "%3.1f", data_buffer().sys_para()->User_cap());
            break;
        case SuppCap:
            sprintf(key_buf[0], "%3.1f", data_buffer().sys_para()->Supp_cap());
            break;
        case CvtModifyKSub:
            ui = buf_pt[6] * 8 + row;
            fi = data_buffer().sys_para()->cvt_modify(ui-1);
            sprintf(key_buf[0], "%5.3f", fi / 1000);
            break;
        case SetHarmLimitSub:
            ui = buf_pt[6] * 8 + row - 1;
            if (!ui) { //第1个值为THDu
                fi = data_buffer().sys_para()->harm_ulimit(0);
                sprintf(key_buf[0], "%3.1f", fi / 10);
            } else {
                fi = data_buffer().sys_para()->harm_ulimit(ui);
                fj = data_buffer().sys_para()->harm_ilimit(ui-1);
                sprintf(key_buf[0], "%3.1f", fi / 10);
                sprintf(key_buf[1], "%3.1f", fj / 10);
                buf_sel_sz_ = 2;
            }
            break;
        case VW_raThresh:
            buf_pt[6] = 0;
            break;
        case VW_HmCurrLimit:
            data_buffer().RefreshHrmlmt();
            //harmfunc->calc_user_cur();
            buf_pt[6] = 0;
            break;
        case VW_raDisHarm:
        case VW_CvtModifyK:
        case VW_SetHarmLimit:
            buf_pt[6] = 0;
            break;
        case ZeroInputThr:  //零输入信号判断阈值
            sprintf(key_buf[0], "%i", data_buffer().equip_para()->zero_thr(0));
            sprintf(key_buf[1], "%i", data_buffer().equip_para()->zero_thr(1));
            buf_sel_sz_ = 2;
            break;
        case PTScale:
            sprintf(key_buf[0], "%i", data_buffer().GetRatio(0));
            sprintf(key_buf[1], "%i", data_buffer().GetRatio(1));
            buf_sel_sz_ = 2;
            break;
        case CTScale:
            if (data_buffer().equip_para()->current_clamp_en()&0x1) {
                buf_pt[1] = data_buffer().equip_para()->current_clamp_ratio();
            }
            sprintf(key_buf[0], "%i", data_buffer().GetRatio(2));
            sprintf(key_buf[1], "%i", data_buffer().GetRatio(3));
            buf_sel_sz_ = 2;
            break;
        case VoltLvl:
            buf_pt[0] = data_buffer().sys_para()->CUlevel();
            break;
        case kUserName:
            buf_pt[0] = data_buffer().latest();
            break;
        case kClockSource:
            buf_pt[0] = data_buffer().equip_para()->clock_src();
            break;
        case AdSampleRate:
            buf_pt[0] = data_buffer().equip_para()->ad_sample_rate();
            break;
        case kComtradeSvPath:
            buf_pt[0] = data_buffer().equip_para()->cmtrd_sv_path();
            break;
        case kRecWaveSmplRt:
            buf_pt[0] = data_buffer().equip_para()->wav_sample_rate();
            break;
        case ConnectType:
            buf_pt[0] = data_buffer().sys_para()->connect_t();
            break;
        case HrmRcdSaveType:
            buf_pt[0] = data_buffer().sys_para()->hrm_save_type();
            break;
        case kTimeZone:
            buf_pt[0] = 12-data_buffer().equip_para()->timezone();
            buf_pt[1] = data_buffer().equip_para()->timezone(1);
            buf_sel_sz_ = 2;
            break;
        case SetLimitType:
            buf_pt[0] = data_buffer().sys_para()->limit_type();
            break;
        case kInterHmGroup:
            buf_pt[0] = data_buffer().sys_para()->inharm_type();
            break;
        case CurrClampType:
            buf_pt[0] = data_buffer().equip_para()->current_clamp_type();
            break;
        case UnitNum:
            sprintf(key_buf[0], "%i", data_buffer().equip_para()->device_num());
            break;
        case kMdfyEvnStm:
            sprintf(key_buf[0], "%i", data_buffer().equip_para()->modify_evn_stm());
            break;
        case ResetHysTime:
            sprintf(key_buf[0], "%i", data_buffer().equip_para()->reset_hyst_time());
            break;
        case kFilterCP:
            sprintf(key_buf[0], "%i", data_buffer().equip_para()->fltr_cpx());
            break;
        case kTimeDiff:
            sprintf(key_buf[0], "%i", data_buffer().equip_para()->time_diff());
            break;
        case LCMDelayTime:
            sprintf(key_buf[0], "%i", data_buffer().equip_para()->lcm_dely_time());
            break;
        case kCapW24hThr:
            sprintf(key_buf[0], "%i", data_buffer().eew_para()->w24h_thr());
            break;
        case HarmRcdSpace:
            sprintf(key_buf[0], "%i", data_buffer().sys_para()->harm_rcd_space());
            break;
        case FreqRcdSpace:
            buf_pt[0] = Freq2Indx(data_buffer().sys_para()->freq_rcd_space(), 1);
            break;
        case kFreqEvalTm:
            buf_pt[0] = data_buffer().equip_para()->freq_evaltm();
            break;
        case FreqSaveType:
            buf_pt[0] = data_buffer().sys_para()->freq_save_type();
            break;
        case VoltWarpRcdSpace:
            sprintf(key_buf[0], "%i", data_buffer().sys_para()->voltdv_rcd_space());
            break;
        case VoltWarpSaveType:
            buf_pt[0] = data_buffer().sys_para()->voltdv_save_type();
            break;
        case ImbalanceRcdSpace:
            sprintf(key_buf[0], "%i", data_buffer().sys_para()->unbalance_rcd_space());
            break;
        case ImbalanceSaveType:
            buf_pt[0] = data_buffer().sys_para()->unbalance_save_type();
            break;
        case TranstRcdTime:
            if (!trnst_valid_) { //不具有暂态功能
                return;
            }
            sprintf(key_buf[0], "%i", data_buffer().equip_para()->transt_rcd_time());
            set_confirm_ok_cnt_ = 0;
            break;
        case kHarmRecSvMax:
            sprintf(key_buf[0], "%d", data_buffer().equip_para()->harm_rec_svmax());
            set_confirm_ok_cnt_ = 0;
            break;
        case BaudRateSet:
            sprintf(key_buf[0], "%li", data_buffer().equip_para()->BaudRate(0));
            break;
        case PhsAdjEnable: //相别校准使能
            ui = data_buffer().equip_para()->phs_adj_enable();
            buf_sel_sz_ = 3;
            buf_pt[0] = ui & 0x1;
            buf_pt[1] = (ui >> 1) & 0x1;
            buf_pt[2] = (ui >> 2) & 0x1;
            break;
        case VorC_AdjEnable: //电压电流校准使能
            ui = data_buffer().equip_para()->vc_adj_enable();
            buf_sel_sz_ = 2;
            buf_pt[0] = ui & 0x1;
            buf_pt[1] = (ui >> 1) & 0x1;
            break;
        case kHarmTrgrEnable: //谐波触发使能
            buf_sel_sz_ = 2;
            buf_pt[0] = data_buffer().sys_para()->trigger_enable(2);
            buf_pt[1] = data_buffer().sys_para()->trigger_enable(3);
            break;
        case kUnblcTrgrEnable: //不平衡触发使能
            buf_sel_sz_ = 2;
            buf_pt[0] = data_buffer().sys_para()->trigger_enable(4);
            buf_pt[1] = data_buffer().sys_para()->trigger_enable(5);
            break;
        case kStdyTrgrEnable:
            buf_pt[0] = data_buffer().sys_para()->trigger_enable(0);
            break;
        case kFreqTrgrEnable:
            buf_pt[0] = data_buffer().sys_para()->trigger_enable(1);
            break;
        case kFixFreqSmpl:
            buf_pt[0] = data_buffer().equip_para()->fix_smpl()&0x1;
            break;
        case kHeartbeat61850:
            buf_pt[0] = data_buffer().equip_para()->hrtbt_61850()&0x1;
            break;
        case kVoltDvTrgrEnable:
            buf_pt[0] = data_buffer().sys_para()->trigger_enable(6);
            break;
        case DebugEnable:
            buf_pt[0] = data_buffer().equip_para()->debug_enable();
            break;
        case PstMdfyEnable:
            buf_pt[0] = data_buffer().equip_para()->Pst_mdfy_enable();
            break;
        case PstEnable:
            buf_pt[0] = data_buffer().equip_para()->pst_enable();
            break;
        case kSecurityEn:
            buf_pt[0] = data_buffer().equip_para()->security_en();
            break;
        case kSignalSimuEn:
            buf_pt[0] = data_buffer().equip_para()->signl_simu_en();
            break;
        case SaveWaveEnable:
            buf_pt[0] = data_buffer().equip_para()->save_wave_en();
            break;
        case kInterHarmEna:
            buf_pt[0] = data_buffer().sys_para()->inter_enable();
            break;
        case kHarmAlmEna:
            buf_pt[0] = data_buffer().equip_para()->alarm_enable(kAlmHarmEn);
            break;
        case kFreqAlmEna:
            buf_pt[0] = data_buffer().equip_para()->alarm_enable(kAlmFreqEn);
            break;
        case kPstAlmEna:
            buf_pt[0] = data_buffer().equip_para()->alarm_enable(kAlmPstEn);
            break;
        case kUnblcAlmEna:
            buf_pt[0] = data_buffer().equip_para()->alarm_enable(kAlmUnblcEn);
            break;
        case kVoltDvAlmEna:
            buf_pt[0] = data_buffer().equip_para()->alarm_enable(kAlmVoltdvEn);
            break;
        case kHarmRecSvEn:
            buf_pt[0] = data_buffer().equip_para()->harm_rec_sven();
            break;
        case Aggregation10min:
            buf_pt[0] = data_buffer().equip_para()->aggrgt_time_en();
            break;
        case CurrClampEnable:
            buf_pt[0] = data_buffer().equip_para()->current_clamp_en();
            break;
        case AdjustZeroEnable:
            buf_pt[0] = data_buffer().equip_para()->real_wave_zero();
            break;
        case DeviceSn:
            sprintf(key_buf[0], "%li", data_buffer().equip_para()->device_sn());
            break;
        case SmpVltScale:
            sprintf(key_buf[0], "%5.5f", data_buffer().equip_para()->sample_pt());
            break;
        case SmpCurScale:
            sprintf(key_buf[0], "%6.6f", data_buffer().equip_para()->sample_ct());
            break;
        case kSysTimeError:
            sprintf(key_buf[0], "%d", data_buffer().equip_para()->time_err());
            break;
        case VALineCoef:
        case VBLineCoef:
        case VCLineCoef:
            ui = data_buffer().equip_para()->v_line_coef(type-VALineCoef);
            sprintf(key_buf[0], "%d", ui);
            break;
        case VAResRatio:
        case VBResRatio:
        case VCResRatio:
            fi = data_buffer().equip_para()->v_res_ratio(type-VAResRatio);
            sprintf(key_buf[0], "%5.4f", fi / 10000);
            break;
        case CAResRatio:
        case CBResRatio:
        case CCResRatio:
            fi = data_buffer().equip_para()->c_res_ratio(type-CAResRatio);
            sprintf(key_buf[0], "%5.4f", fi / 10000);
            break;
        case VorCDatum://电压电流基准值
            fi = data_buffer().equip_para()->v_datum();
            sprintf(key_buf[0], "%5.2f", fi / 100);
            fi = data_buffer().equip_para()->c_datum();
            sprintf(key_buf[1], "%4.2f", fi / 100);
            buf_sel_sz_ = 2;
            break;
        case AutoAdj:
            messageq_gui().PushCtrlSig(data_idx_, 3, 1); //start calibrate precision
            break;
        case GetCharacterDCValue:
            messageq_gui().PushCtrlSig(data_idx_, 4, 1); //start get dc component.
            break;
        case TranstTolTime:
            if (!trnst_valid_) { //不具有暂态功能
                return;
            }
            sprintf(key_buf[0], "%i", data_buffer().equip_para()->transt_tol_time());
            set_confirm_ok_cnt_ = 0;
            break;
        case ModifyTime:
            buf_sel_sz_ = 6;
            break;
        case ModifyPasswd: {
            clear();
            show_frame();
            buf_sel_sz_ = 3;
            x = menu_x_+ (font.cn_size+font.space)*2;
            y = top_ + (font.cn_size+font.space)*2;
            int stp = (font.cn_size+font.space)*1.5;
            for (i = 0;i < buf_sel_sz_;i++) {
                strcpy(key_buf[i], "");
                Item_y_[i] = y + stp * i;
                pqm_dis->puts(MdfyPassVW[i], x, Item_y_[i], font.color, font.space);
            }
            cursorx = curr_x_ = x+(font.cn_size+font.space)*6;
            cursory = y;
            refresh();
            break;
        }
        case SetBTimeIntr: //GPS-B对时间隔
            sprintf(key_buf[0], "%d", data_buffer().sys_para()->b_time_intr());
            break;
        case SocketServerPort:
            sprintf(key_buf[0], "%d", data_buffer().equip_para()->socket_server_port());
            break;
        case IPAddrSet:
            data_buffer().equip_para()->get_ip(key_buf[0]);
            break;
        case NTPServerIP:
            data_buffer().equip_para()->get_ntp_ip(key_buf[0]);
            break;
        case NetMaskSet:
            data_buffer().equip_para()->get_nmask(key_buf[0]);
            break;
        case GateWaySet:
            data_buffer().equip_para()->get_gateway(key_buf[0]);
            break;
        case SetMacAddr:
            sprintf(key_buf[0], "%d", hwaddr[4]);
            sprintf(key_buf[1], "%d", hwaddr[5]);
            buf_sel_sz_ = 2;
            break;
        case VW_EEW:
            ra_show_hr_ = data_buffer().eew_para()->show_harmonic();
            break;
        default:
            break;
    }
    pushcmd(curr_cmd_);
    curr_cmd_ = type;
//------------- 初始化光标的位置 -------------------------------------------
    for (i = 0;i < 8;i++) {
        cursor_left[i] = cursorx;
        cursor_top[i] = cursory;
    }
    switch (type) {
        case SetProofTimeIntr:
        case SetMacAddr:
            cursor_left[1] = cursor_left[0] + 4 * (7 + font.space);
            break;
        case ZeroInputThr:
            cursor_left[1] = cursor_left[0] + 5 * (7 + font.space);
            break;
        case PstLimit:
        case kCapLifeThr:
        case kCapVolt1Thr:
        case kCapVolt2Thr:
        case VoltWarp:
        case VorCDatum: //电压电流基准值
        case CTScale:
        case TransCurLimit:
        case ImbalanceLimit:
        case FrequecyLimit:
        case TransTbLimit:
            cursor_left[1] = cursor_left[0] + 7 * (7 + font.space);
            break;
        case PTScale:
            cursor_left[1] = cursor_left[0] + 8 * (7 + font.space);
            break;
        case SetHarmLimitSub:
            cursor_left[1] = cursor_left[0] + 9 * (7 + font.space);
            break;
        case kTimeZone:
            cursor_left[1] = cursor_left[0] + 6 * (7 + font.space);
            break;
            break;
        case PstMdfyEnable: //闪变修正使能
        case PstEnable:
        case kSecurityEn:
        case kSignalSimuEn:
        case SaveWaveEnable:
        case kInterHarmEna:
        case kHarmAlmEna:
        case kFreqAlmEna:
        case kPstAlmEna:
        case kUnblcAlmEna:
        case kVoltDvAlmEna:
        case kHarmRecSvEn:
        case Aggregation10min:
        case kStdyTrgrEnable:
        case kFreqTrgrEnable:
        case kFixFreqSmpl:           
        case kHeartbeat61850:           
        case kVoltDvTrgrEnable:
        case DebugEnable:
        case StartCurEn:
        case TransTbEnable:
        case TransEnable:
        case TransCurEnable:
        case FluctuationEna:
        case PhsAdjEnable: //相别校准使能
        case VorC_AdjEnable: //电压电流校准使能
        case kHarmTrgrEnable: //谐波触发使能
        case kUnblcTrgrEnable: //谐波触发使能
        case CurrClampEnable: //电流钳使能
        case AdjustZeroEnable: //主板侧实时波形过零点调节使能
            for (i = 1;i < buf_sel_sz_;i++) {
                cursor_left[i] = cursor_left[i-1]
                        + font.cn_size + font.space + 2 * (7 + font.space);
            }
            break;
        case TransLimit:
            cursor_left[1] = cursor_left[0] + 6 * (7 + font.space);
            cursor_left[2] = cursor_left[1] + 5 * (7 + font.space);
            break;
        case ModifyTime:
            cursor_left[1] = cursor_left[0] + 5 * (7 + font.space);
            cursor_left[2] = cursor_left[1] + 3 * (7 + font.space);
            cursor_left[3] = cursor_left[2] + 3 * (7 + font.space);
            cursor_left[4] = cursor_left[3] + 3 * (7 + font.space);
            cursor_left[5] = cursor_left[4] + 3 * (7 + font.space);
            break;
        case ModifyPasswd:
            cursor_top[1] = Item_y_[1];
            cursor_top[2] = Item_y_[2];
        default:
            break;
    }
}

/*------------------------------------------------------------------------------
Description:对某一设置项的参数进行设置时的显示
Input:      type,设置项类型
            x,y, 设置项对应参数的显示坐标
            ext, 对有特殊特性的设置项增加的扩展参数
*/
void CSetView::view_sxx(int type, int x, int y, int ext)
{
    char str[32];
    int i, j;
    int dis_type = 0;
    int slcwd = 0;//选择框的宽度
    //pqm_dis->rectangle(x-2, y-13, left_+width-1, y+2, color); //清除本行中要更新的空间
    pqm_dis->clear(x - 2, y - font.cn_size - 1, left_ + width - 1, y + 2); //清除本行中要更新的空间
    switch (type) {
    //--------- 编辑框--单个 ---------------------------------------------------
        case SetBTimeIntr:
        case SocketServerPort:
        case UnitNum:
        case kMdfyEvnStm:
        case ResetHysTime:
        case kFilterCP:
        case kTimeDiff:
        case IPAddrSet:
        case NTPServerIP:
        case NetMaskSet:
        case GateWaySet:
        case DeviceSn:
        case SmpVltScale:
        case SmpCurScale:
        case kSysTimeError:
        case VALineCoef:
        case VBLineCoef:
        case VCLineCoef:
        case VAResRatio:
        case VBResRatio:
        case VCResRatio:
        case CAResRatio:
        case CBResRatio:
        case CCResRatio:
        case kCapCurrThr:
        case kCapCapThr:
        case kCapPeakThr:
        case kCapGamaThr:
        case kTrnsfmrPec:
        case TransEndNum:
        case kAuditLogSize:
            sprintf(str,  "%s", key_buf[0]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kCapW24hThr:
        case HarmRcdSpace:
        case VoltWarpRcdSpace:
        case ImbalanceRcdSpace:
            sprintf(str,  "%s秒", key_buf[0]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case LCMDelayTime:
            sprintf(str,  "%s分钟", key_buf[0]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kCapNmnlMc:
            sprintf(str,  "%s kvar", key_buf[0]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kCapNmnlUc:
            sprintf(str,  "%s V", key_buf[0]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kCapReactancex:
            sprintf(str,  "%s %%", key_buf[0]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kImpedanceBeta:
            sprintf(str,  "%s°", key_buf[0]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case ShortCap:
        case UserCap:
        case SuppCap:
        case kTransformerMs:
            sprintf(str,  "%s MVA", key_buf[0]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kTrnsfmrIn:
        case NegativeILimit:
            sprintf(str,  "%s A", key_buf[0]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kInterHarmDb:
            sprintf(str,  "%-4s", key_buf[0]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kHarmonic10Cyc:
        case StartCurLimit:
        case FluctuationDb:
            sprintf(str,  "%-5s", key_buf[0]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case CvtModifyKSub:
        case BaudRateSet:
            sprintf(str,  "%-6s", key_buf[0]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case CvtModifyEn:
        case TransManualRec:
        case kStdyTrgrEnable:
        case kFreqTrgrEnable:
        case kFixFreqSmpl:
        case kHeartbeat61850:
        case kVoltDvTrgrEnable:
        case DebugEnable:
        case PstMdfyEnable:
        case PstEnable:
        case kSecurityEn:
        case kSignalSimuEn:
        case SaveWaveEnable:
        case kInterHarmEna:
        case kHarmRecSvEn:
        case Aggregation10min:
        case CurrClampEnable:
            sprintf(str, "%s", YesNo[buf_pt[0]&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kHarmAlmEna:
        case kFreqAlmEna:
        case kPstAlmEna:
        case kUnblcAlmEna:
        case kVoltDvAlmEna:
            sprintf(str, "%s", YesNo[buf_pt[0]==0?0:1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case AdjustZeroEnable:
            sprintf(str, "%s", NoYes[buf_pt[0]&0x1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
    //--------- 编辑框--多个 ---------------------------------------------------
        case SetMacAddr:
            sprintf(str,  "%-3s:%s", key_buf[0], key_buf[1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case SetProofTimeIntr:
            sprintf(str,  "%-3s/%s", key_buf[0], key_buf[1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case VoltWarp:
            sprintf(str,  "%-4s /-%s", key_buf[0], key_buf[1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case PstLimit:
        case VorCDatum: //电压电流基准值
        case kCapLifeThr:
        case kCapVolt1Thr:
        case kCapVolt2Thr:
            sprintf(str,  "%-5s /%s", key_buf[0], key_buf[1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case ZeroInputThr:
            sprintf(str,  "%-2sV /%smA", key_buf[0], key_buf[1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case TransLimit:
            sprintf(str,  "%-5s/%-4s/%s", key_buf[0], key_buf[1], key_buf[2]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case CTScale:
        case TransCurLimit:
        case ImbalanceLimit:
        case FrequecyLimit:
        case TransTbLimit:
            sprintf(str,  "%-6s/%s", key_buf[0], key_buf[1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case PTScale:
            sprintf(str,  "%-7s/%s", key_buf[0], key_buf[1]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case VorC_AdjEnable: //电压电流校准使能
            sprintf(str, "%s /%s", YesNo[buf_pt[0]], YesNo[buf_pt[1]]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kHarmTrgrEnable: //谐波触发使能
            sprintf(str, "%s /%s", YesNo[buf_pt[0]], YesNo[buf_pt[1]]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kUnblcTrgrEnable: //不平衡触发使能
            sprintf(str, "%s /%s", YesNo[buf_pt[0]], YesNo[buf_pt[1]]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case TransEnable:
        case TransCurEnable:
        case TransTbEnable:
        case FluctuationEna:
        case StartCurEn:
        case PhsAdjEnable: //相别校准使能
            sprintf(str, "%s /%s /%s", YesNo[buf_pt[0]], YesNo[buf_pt[1]], YesNo[buf_pt[2]]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case ModifyTime:
            sprintf(str,  "%-4s-%-2s-%-2s %-2s:%-2s:%-2s", key_buf[0], key_buf[1],
                    key_buf[2], key_buf[3], key_buf[4], key_buf[5]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
    //--------- 选择框--单个 ---------------------------------------------------
        case GpsSingleType:
            pqm_dis->puts(kGpsSingleType[buf_pt[0]], x, y, font.color, font.space);
            slcwd = 5*(font.cn_size+font.space);
            break;
        case GpsPulseType:
            pqm_dis->puts(Gps_Pulse_Type[buf_pt[0]], x, y, font.color, font.space);
            slcwd = 4*(font.cn_size+font.space);
            break;
        case kRecWaveFormat:
            pqm_dis->puts(RcdWvSavFmt[buf_pt[0]], x, y, font.color, font.space);
            slcwd = 6*(font.cn_size+font.space);
            break;
        case VoltLvl:
            pqm_dis->puts(ULevSet[buf_pt[0]], x, y, font.color, font.space);
            slcwd = 5*(7+font.space);
            break;
        case kUserName:
            pqm_dis->puts(data_buffer().usr_name(buf_pt[0]), x, y, font.color, font.space);
            slcwd = 9*(7+font.space);
            break;
        case FreqRcdSpace:
            pqm_dis->puts(FreqSaveSpc[buf_pt[0]], x, y, font.color, font.space);
            slcwd = 2*(font.cn_size+font.space);
            break;
        case kFreqEvalTm:
            pqm_dis->puts(FreqEvalTmSlct[buf_pt[0]], x, y, font.color, font.space);
            slcwd = 2*(font.cn_size+font.space);
            break;
        case kClockSource:
            pqm_dis->puts(ClockSourceSlct[buf_pt[0]], x, y, font.color, font.space);
            slcwd = 7*(7+font.space);
            break;
        case AdSampleRate:
            pqm_dis->puts(AdSmplRtSlct[buf_pt[0]], x, y, font.color, font.space);
            slcwd = 8*(7+font.space);
            break;
        case kComtradeSvPath:
            pqm_dis->puts(kCmtrdSvPathSlct[buf_pt[0]], x, y, font.color, font.space);
            slcwd = 17*(7+font.space);
            break;
        case kRecWaveSmplRt:
            pqm_dis->puts(WvSmplRtSlct[buf_pt[0]], x, y, font.color, font.space);
            slcwd = 8*(7+font.space);
            break;
        case ConnectType:
            pqm_dis->puts(CnctTpSlct[buf_pt[0]], x, y, font.color, font.space);
            slcwd = 3*(7+font.space);
            break;
        case HrmRcdSaveType:
        case FreqSaveType:
        case VoltWarpSaveType:
        case ImbalanceSaveType:
            pqm_dis->puts(HrmRcdTypeSet[buf_pt[0]], x, y, font.color, font.space);
            slcwd = 4*(font.cn_size+font.space);
            break;
        case SetLimitType:
            pqm_dis->puts(LimitTypeSet[buf_pt[0]], x, y, font.color, font.space);
            slcwd = 4*(font.cn_size+font.space);
            break;
        case kInterHmGroup:
            pqm_dis->puts(kIntrHarmGrpt[buf_pt[0]], x, y, font.color, font.space);
            slcwd = 10*(font.cn_size+font.space);
            break;
        case CurrClampType:
            if (!data_buffer().equip_para()->current_clamp_en()&0x1)
                return;
            pqm_dis->puts(CClampTypeSet[buf_pt[0]], x, y, font.color, font.space);
            slcwd = 14*(7+font.space);
            break;
    //--------- 选择框--多个 ---------------------------------------------------
        case kTimeZone:
            sprintf(str, "%3s  /%s", TimeZoneSelect[buf_pt[0]], GPSTZTypeSelect[buf_pt[1]]);
            pqm_dis->puts(str, x, y, font.color, font.space);
            j = cursorx;
            if (buf_sel_==0) slcwd = 3*(7+font.space);
            else slcwd = 5*(7+font.space);
            break;
    //--------- 复杂界面 -------------------------------------------------------
        case SysIni:
        case ResetDefaultPara:
            x = pop_box_.x0;
            y = pop_box_.y0;
            pqm_dis->clear(x, y, pop_box_.x1, pop_box_.y1);
            pqm_dis->rectframe(x, y, pop_box_.x1, pop_box_.y1, vgacolor(kVGA_HiBlue));
            dis_type = 1;
            switch(type) {
                case SysIni:
                    pqm_dis->puts("本操作将清除所有记录并复位所有参数！", x+8, y + 20, font.color, font.space);
                    break;
                case ResetDefaultPara:
                    pqm_dis->puts("本操作将复位所有参数！", x+8, y + 20, font.color, font.space);
                    break;
            }
            pqm_dis->puts("按'设置'键继续，按任意键取消", x+8, y + 40, font.color, font.space);
            break;
        case TranstTolTime:
        case TranstRcdTime:
        case kHarmRecSvMax:
            if (!set_confirm_ok_cnt_) {
                switch(type) {
                    case TranstTolTime:
                        sprintf(str,  "%s分钟", key_buf[0]);
                        break;
                    case TranstRcdTime:
                        sprintf(str,  "%s秒", key_buf[0]);
                        break;
                    case kHarmRecSvMax:
                        sprintf(str,  "%s", key_buf[0]);
                        break;
                    default:
                        str[0]=0;
                        break;
                }
                if (str[0]) pqm_dis->puts(str, x, y, font.color, font.space);
            } else {
                dis_type = 1;
                x = pop_box_.x0;
                y = pop_box_.y0;
                pqm_dis->clear(x, y, pop_box_.x1, pop_box_.y1);
                pqm_dis->rectframe(x, y, pop_box_.x1, pop_box_.y1, vgacolor(kVGA_HiBlue));
                switch(type) {
                    case TranstTolTime:
                    case TranstRcdTime:
                        pqm_dis->puts("本操作将删除所有的暂态记录！", x+8, y + 20, font.color, font.space);
                        break;
                    case kHarmRecSvMax:
                        pqm_dis->puts("本操作将删除所有的谐波记录！", x+8, y + 20, font.color, font.space);
                        break;
                }
                pqm_dis->puts("按'设置'键继续，按任意键取消", x+8, y + 40, font.color, font.space);
            }
            break;
        case SetHarmLimitSub:
            j = buf_pt[6] * 8 + curr_sn_;
            if (!j) {
                sprintf(str,  "%-6s%%", key_buf[0]);
            } else {
                sprintf(str,  "%-6s%% /%s A", key_buf[0], key_buf[1]);
            }
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case kPassword:
            for (i = 0;i < buf_pt[0];i++) {
                str[i] = '*';
            }
            str[i] = 0;
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case ResetPassword:
            messageq_gui().PushCtrlSig(data_idx_, 5);    //Reset password
            x += 20;
            pqm_dis->puts("OK!", x, y, font.color, font.space);
            curr_cmd_ = popcmd();
            break;
        case AutoAdj:
            if (data_buffer().clbrt_stat(&i)) {
                sprintf(str, "开始...%d", i);
            } else {
                sprintf(str, "结束！");
                curr_cmd_ = popcmd();
                para_update_ = true;
                daram_update = true;
            }
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        case GetCharacterDCValue:
            j = data_buffer().getdc_stat(&i);
            if (j == 1) {
                sprintf(str, "开始...%i", i);
                pqm_dis->puts(str, x, y, font.color, font.space);
            } else
                if (j == 0) {
                    sprintf(str, "完成！");
                    pqm_dis->puts(str, x, y, font.color, font.space);
                    curr_cmd_ = popcmd();
                    para_update_ = true;
                    daram_update = true;
                } else {
                    sprintf(str, "超时！！", i);
                    pqm_dis->puts(str, x, y, font.color, font.space);
                    curr_cmd_ = popcmd();
                }
            break;
        case ModifyPasswd:
            for (i = 0;i < buf_pt[ext];i++) {
                str[i] = '*';
            }
            str[i] = 0;
            pqm_dis->puts(str, x, y, font.color, font.space);
            break;
        default:
            return;
    }
    if (slcwd) {    //绘选择框
        if (type!=kTimeZone) j = x;
        pqm_dis->rectangle_xor(x - 2, y - font.cn_size - 1, left_ + width - 1, y + 2, color);
        pqm_dis->draw_icon(j + slcwd, y - font.cn_size + 2, ICON_ARROWD, vgacolor(kVGA_HiGrey), 1);
        pqm_dis->rectframe(j - 2, y - font.cn_size - 1, j + slcwd + 11, y + 2, vgacolor(kVGA_HiGrey));
        pqm_dis->line(j + slcwd , y - font.cn_size, j + slcwd, y + 1, vgacolor(kVGA_HiGrey), 0);
    }
    if (dis_type == 1) {
        pqm_dis->refresh(x, y, x + width*8/10, y + height*2/4);
    } else {
        pqm_dis->refresh(x - 2, y - font.cn_size - 1 , left_ + width, y + 2);
    }
}

/*------------------------------------------------------------------------------
Description:具体设置项的键盘输入处理
Input:      type,设置项类型
            ktype, 键盘输入码，表示哪个按键被按下
            ext, 对有特殊特性的设置项增加的扩展参数
*/
void CSetView::view_sxx_key(int type, int ktype, int ext)
{
    float fi, fj;
    int i, j;
    unsigned int ui;
    long li, lj;
    char stri[32];
    unsigned short k;
    nd_rfrsh_ = true;
    if (type==TranstRcdTime||type==TranstTolTime||type==kHarmRecSvMax) {
        if (ktype!=KEY_SET && set_confirm_ok_cnt_==1) {
            ktype = KEY_ESC;
        }
    }
    if (type==SysIni||type==ResetDefaultPara) {
        if (ktype!=KEY_SET && set_confirm_ok_cnt_==0) {
            ktype = KEY_ESC;
        }
    }
    if (ktype == KEY_SET) {
        para_update_ = true;
        sscanf(key_buf[0], "%f", &fi);
        sscanf(key_buf[0], "%d", &i);
        switch (type) {
            case kPassword: {
                i = data_buffer().CheckPasswd(key_buf[0]);
                if (i==0) {
                    ps_err_cnt_ = 5;
                    if (strcmp(data_buffer().usr_name(), "admin")==0) {
                        curr_cmd_ = VW_SY0;
                    } else if (strcmp(data_buffer().usr_name(), "auditor")==0) {
                        curr_cmd_ = VW_AUDIT;
                    } else {
                        curr_cmd_ = VW_S0;
                    }
                    showcursor = false;
                    nd_rfrsh_ = true;
                } else {
                    force_exit_ = true;
                    if (--ps_err_cnt_<=0) {
                        time(&lock_t_);
                    }
                }
                para_update_ = true;
                return;
            }
            case SetHarmLimitSub:
                j = buf_pt[6] * 8 + curr_sn_;
                if (fi > 6000) fi = 6000;
                data_buffer().sys_para()->set_harm_ulimit(j, (unsigned short)(fi * 10 + 0.001));
                if (j > 0) { //不是THDu
                    sscanf(key_buf[1], "%f", &fi);
                    if (fi > 6000) fi = 6000;
                    data_buffer().sys_para()->set_harm_ilimit(j-1, (unsigned short)(fi * 10 + 0.001));
                }
                break;
            case CvtModifyKSub:
                j = buf_pt[6] * 8 + curr_sn_;
                data_buffer().sys_para()->set_cvt_modify(j, (unsigned short)(fi * 1000 + 0.001));
                break;
            case VoltWarp:
                data_buffer().sys_para()->set_Volt_warp(0, fi);
                sscanf(key_buf[1], "%f", &fi);
                data_buffer().sys_para()->set_Volt_warp(1, -fi);
                break;
            case ShortCap:
                data_buffer().sys_para()->set_Short_cap(fi);
                break;
            case UserCap:
                data_buffer().sys_para()->set_User_cap(fi);
                break;
            case SuppCap:
                data_buffer().sys_para()->set_Supp_cap(fi);
                break;
            case TransEnable:
                ui = buf_pt[0];
                ui |= buf_pt[1] << 1;
                ui |= buf_pt[2] << 2;
                data_buffer().sys_para()->set_transt_monitor(ui);
                daram_update = true;
                break;
            case TransCurEnable:
                ui = buf_pt[0];
                ui |= buf_pt[1] << 1;
                ui |= buf_pt[2] << 2;
                data_buffer().sys_para()->set_transt_i_monitor(ui);
                daram_update = true;
                break;
            case FluctuationEna:
                ui = buf_pt[0];
                ui |= buf_pt[1] << 1;
                ui |= buf_pt[2] << 2;
                data_buffer().sys_para()->set_fluct_enable(ui);
                daram_update = true;
                break;
            case TransLimit:
                if (fi > 999)
                    fi = 999;//最大允许值为5000
                if (fi < 100)
                    fi = 100;//最小允许值为100
                data_buffer().sys_para()->set_vvr_limit(0, (unsigned short)(fi * 10 + 0.001));
                sscanf(key_buf[1], "%f", &fi);
                if (fi > 100)
                    fi = 100;//最大允许值为100
                data_buffer().sys_para()->set_vvr_limit(1, (unsigned short)(fi * 10 + 0.001));
                sscanf(key_buf[2], "%f", &fi);
                if (fi > 100)
                    fi = 100;//最大允许值为100
                data_buffer().sys_para()->set_vvr_limit(2, (unsigned short)(fi * 10 + 0.001));
                daram_update = true;
                break;
            case TransCurLimit:
                if (fi > 6000) fi = 6000;//最大允许值为6000
                data_buffer().sys_para()->set_ivr_limit(0, (unsigned short)(fi * 10 + 0.001));
                sscanf(key_buf[1], "%f", &fj);
                if (fj > fi) fj = fi;
                data_buffer().sys_para()->set_ivr_limit(1, (unsigned short)(fj * 10 + 0.001));
                daram_update = true;
                break;
            case GpsSingleType:
                data_buffer().sys_para()->set_gps_single_type(buf_pt[0]);
                gps_pulse_update = true;
                break;
            case GpsPulseType:
                data_buffer().sys_para()->set_gps_pulse_type(buf_pt[0]);
                gps_pulse_update = true;
                break;
            case kRecWaveFormat:
                data_buffer().equip_para()->set_rcd_wv_fmt(buf_pt[0]);
                break;
            case SetBTimeIntr:
                data_buffer().sys_para()->set_b_time_intr(i);
                break;
            case SetProofTimeIntr:
                data_buffer().sys_para()->set_proof_time_intr(0, i);
                sscanf(key_buf[1], "%d", &i);
                data_buffer().sys_para()->set_proof_time_intr(1, i);
                gps_pulse_update = true;
                break;
            case StartCurEn:
                ui = buf_pt[0];
                ui |= buf_pt[1] << 1;
                ui |= buf_pt[2] << 2;
                data_buffer().sys_para()->set_start_cur_en(ui);
                daram_update = true;
                break;
            case StartCurLimit:
                if (fi > 999.9)
                    fi = 999.9;//最大允许值为999
                if (fi < 0)
                    fi = 0;//最小允许值为0
                data_buffer().sys_para()->set_sc_limit((unsigned short)(fi * 10));
                daram_update = true;
                break;
            case FluctuationDb:
                if (fi > 9.99)
                    fi = 9.99;//最大允许值为9999
                if (fi < 0)
                    fi = 0;//最小允许值为0
                data_buffer().sys_para()->set_fluct_db((unsigned short)(fi * 1000));
                daram_update = true;
                break;
            case kInterHarmDb:
                if (fi > 99.9)
                    fi = 99.9;//最大允许值为999
                data_buffer().sys_para()->set_inter_limit((unsigned short)(fi * 10));
                break;
            case TransTbEnable:
                ui = buf_pt[0];
                ui |= buf_pt[1] << 1;
                ui |= buf_pt[2] << 2;
                data_buffer().sys_para()->set_transt_tb_enable(ui);
                daram_update = true;
                break;
            case TransTbLimit:
                if (fi > 5000)
                    fi = 5000;//最大允许值为5000
                if (fi < 0)
                    fi = 0;//最小允许值为0
                data_buffer().sys_para()->set_tb_high_limit((unsigned short)(fi * 10 + 0.001));
                sscanf(key_buf[1], "%f", &fi);
                if (fi > 100)
                    fi = 100;//最大允许值为100
                data_buffer().sys_para()->set_tb_low_limit((unsigned short)(fi * 10 + 0.001));
                daram_update = true;
                break;
            case TransManualRec:
                data_buffer().sys_para()->set_manual_rec_enable(buf_pt[0]);
                daram_update = true;
                break;
            case CvtModifyEn:
                data_buffer().sys_para()->set_cvt_modify_en(buf_pt[0]);
                daram_update = true;
                break;
            case TransEndNum:
                if (i < 10) i = 10;
                else if (i > 100) i = 100;
                i = (i + 2) / 5 * 5;
                data_buffer().sys_para()->set_transt_end_num(i);
                daram_update = true;
                break;
            case kAuditLogSize:
                if (i < 1) i = 1;
                else if (i > 200) i = 200;
                data_buffer().equip_para()->set_audtlog_size(i);
                break;
            case ImbalanceLimit:
                if (fi > 6000) fi = 6000;//最大允许值为6000%
                data_buffer().sys_para()->set_unbalance_thr(0, (unsigned short)(fi * 10 + 0.001));
                
                sscanf(key_buf[1], "%f", &fi);
                if (fi > 6000) fi = 6000;//最大允许值6000%
                data_buffer().sys_para()->set_unbalance_thr(1, (unsigned short)(fi * 10 + 0.001));
                break;
            case NegativeILimit:
                if (fi > 6000) fi = 6000;//最大允许值为6000A
                data_buffer().sys_para()->set_neg_sequence_Ithr( (unsigned short)(fi * 10 + 0.001) );
               break;
            case kHarmonic10Cyc:
                if (fi > 50.9) fi = 50.9;//最大允许值为6000A
                data_buffer().equip_para()->set_harmnm10cyc(fi * 10 + 0.001);
                daram_update = true;
                break;
            case FrequecyLimit:
                if (fi > 200) fi = 200;//最大允许值为200Hz
                data_buffer().sys_para()->set_freq_limit(0, (unsigned short)(fi * 100 + 0.001));
                sscanf(key_buf[1], "%f", &fi);
                if (fi > 200) fi = 200;//最大允许值为200Hz
                data_buffer().sys_para()->set_freq_limit(1, (unsigned short)(fi * 100 + 0.001));
                break;
            case PstLimit:
                if (fi > 100) fi = 100;//最大允许值为100
                data_buffer().sys_para()->set_pst_limit((unsigned short)(fi * 100 + 0.0001));
                sscanf(key_buf[1], "%f", &fi);
                if (fi > 100) fi = 100;//最大允许值为100
                data_buffer().sys_para()->set_plt_limit((unsigned short)(fi * 100 + 0.0001));
                break;
            case kCapLifeThr:
                data_buffer().eew_para()->set_life_thr((unsigned short)(fi * 1000 + 0.001));
                sscanf(key_buf[1], "%d", &ui);
                data_buffer().eew_para()->set_life_thr_cnt(ui);
                eew_update_ = 3;
                break;
            case kCapVolt1Thr:
                data_buffer().eew_para()->set_volt1_thr((unsigned short)(fi * 1000 + 0.001));
                sscanf(key_buf[1], "%d", &ui);
                data_buffer().eew_para()->set_voltdur1_thr(ui);
                eew_update_ = 3;
                break;
            case kCapVolt2Thr:
                data_buffer().eew_para()->set_volt2_thr((unsigned short)(fi * 1000 + 0.001));
                sscanf(key_buf[1], "%d", &ui);
                data_buffer().eew_para()->set_voltdur2_thr(ui);
                eew_update_ = 3;
                break;
            case kCapCurrThr:
                data_buffer().eew_para()->set_curr_thr((unsigned short)(fi * 1000 + 0.001));
                eew_update_ = 3;
                break;
            case kCapCapThr:
                data_buffer().eew_para()->set_cap_thr((unsigned short)(fi * 1000 + 0.001));
                eew_update_ = 3;
                break;
            case kCapPeakThr:
                data_buffer().eew_para()->set_peakv_thr(fi * 1000 + 0.001);
                eew_update_ = 3;
                break;
            case kCapGamaThr:
                data_buffer().eew_para()->set_Th(fi * 1000 + 0.001);
                eew_update_ = 1;
                break;
            case kTrnsfmrPec:
                data_buffer().eew_para()->set_Pec_rpu(fi * 1000 + 0.001);
                eew_update_ = 1;
                break;
            case kTransformerMs:
                data_buffer().eew_para()->set_Ms(fi * 100 + 0.001);
                eew_update_ = 1;
                break;
            case kTrnsfmrIn:
                data_buffer().eew_para()->set_In(fi * 100 + 0.001);
                eew_update_ = 1;
                break;
            case kCapNmnlMc:
                data_buffer().eew_para()->set_Mc(fi * 100 + 0.001);
                eew_update_ = 3;
                break;
            case kCapNmnlUc:
                data_buffer().eew_para()->set_Uc(i);
                eew_update_ = 3;
                break;
            case kCapReactancex:
                data_buffer().eew_para()->set_x(fi * 100 + 0.001);
                eew_update_ = 1;
                break;
            case kImpedanceBeta:
                data_buffer().eew_para()->set_beta(fi * 100 + 0.001);
                eew_update_ = 1;
                break;
            case ZeroInputThr:
                data_buffer().equip_para()->set_zero_thr(0, i);
                sscanf(key_buf[1], "%d", &i);
                data_buffer().equip_para()->set_zero_thr(1, i);
                daram_update = true;
                break;
            case PTScale:
                data_buffer().SetRatio(0, i);   //set PT1
                sscanf(key_buf[1], "%d", &i);
                if (i>0) data_buffer().SetRatio(1, i);   //set PT2;
                daram_update = true;
                eew_update_ = 2;
                break;
            case CTScale:
                data_buffer().SetRatio(2, i);   //set CT1
                data_buffer().SetCclampPara(3, i); //保存一次侧数值，此句千万不能放在后面
                if (data_buffer().equip_para()->current_clamp_en()) {
                    data_buffer().SetCclampPara(4, buf_pt[1]); //设置电流钳变比和CT2
                    //data_buffer().line_para.CT2 = CClampRatioSet[data_buffer().current_clamp_type()][buf_pt[1]];
                } else {
                    sscanf(key_buf[1], "%d", &i);
                    if (i>0) data_buffer().SetRatio(3, i);   //set CT2;
                }
                daram_update = true;
                eew_update_ = 2;
                break;
            case UnitNum:
                data_buffer().equip_para()->set_device_num(i);
                break;
            case kMdfyEvnStm:
                data_buffer().equip_para()->set_modify_evn_stm(i);
                break;
            case ResetHysTime:
                data_buffer().equip_para()->set_reset_hyst_time(i);
                break;
            case kFilterCP:
                data_buffer().equip_para()->set_fltr_cpx(i);
                break;
            case kTimeDiff:
                data_buffer().equip_para()->set_time_diff(i);
                break;
            case LCMDelayTime:
                data_buffer().equip_para()->set_lcm_dely_time(i == 0 ? 1 : i);
                break;
            case kCapW24hThr:
                if (i < 1) i = 1;
                data_buffer().eew_para()->set_w24h_thr(i);
                eew_update_ = 1;
                break;
            case HarmRcdSpace:
                k = adj_save_space(i, 0);
                if (k != data_buffer().sys_para()->harm_rcd_space()) {
                    data_buffer().sys_para()->set_harm_rcd_space(k);
                }
                break;
            case VoltLvl:
                data_buffer().sys_para()->set_CUlevel(buf_pt[0]);
                break;
            case kUserName:
                para_update_ = false;
                break;
            case FreqRcdSpace:
                data_buffer().sys_para()->set_freq_rcd_space(Freq2Indx(buf_pt[0], 0));
                break;
            case kFreqEvalTm:
                data_buffer().equip_para()->set_freq_evaltm(buf_pt[0]);
                break;
            case kTimeZone:
                data_buffer().equip_para()->set_timezone(12-buf_pt[0]);
                data_buffer().equip_para()->set_timezone(buf_pt[1], 1);
                break;
            case kClockSource:
                data_buffer().equip_para()->set_clock_src(buf_pt[0]);
                daram_update = true;
                break;
            case AdSampleRate:
                data_buffer().equip_para()->set_ad_sample_rate(buf_pt[0]);
                daram_update = true;
                break;
            case kComtradeSvPath:
                data_buffer().equip_para()->set_cmtrd_sv_path(buf_pt[0]);
                daram_update = true;
                break;
            case kRecWaveSmplRt:
                data_buffer().equip_para()->set_wav_sample_rate(buf_pt[0]);
                daram_update = true;
                break;
            case ConnectType:
                data_buffer().sys_para()->set_connect_t(buf_pt[0]);
                daram_update = true;
                eew_update_ = 3;
                break;
            case HrmRcdSaveType:
                data_buffer().sys_para()->set_hrm_save_type(buf_pt[0]);
                break;
            case FreqSaveType:
                k = buf_pt[0];
                if (k != data_buffer().sys_para()->freq_save_type()) {
                    data_buffer().sys_para()->set_freq_save_type(k);
                }
                break;
            case VoltWarpRcdSpace:
                k = adj_save_space(i, 0);
                if (k != data_buffer().sys_para()->voltdv_rcd_space()) {
                    data_buffer().sys_para()->set_voltdv_rcd_space(k);
                }
                break;
            case VoltWarpSaveType:
                k = buf_pt[0];
                if (k != data_buffer().sys_para()->voltdv_save_type()) {
                    data_buffer().sys_para()->set_voltdv_save_type(k);
                }
                break;
            case ImbalanceRcdSpace:
                k = adj_save_space(i, 0);
                if (k != data_buffer().sys_para()->unbalance_rcd_space()) {
                    data_buffer().sys_para()->set_unbalance_rcd_space(k);
                }
                break;
            case ImbalanceSaveType:
                k = buf_pt[0];
                if (k != data_buffer().sys_para()->unbalance_save_type()) {
                    data_buffer().sys_para()->set_unbalance_save_type(k);
                }
                break;
            case SetLimitType:
                data_buffer().sys_para()->set_limit_type(buf_pt[0]);
                break;
            case kInterHmGroup:
                data_buffer().sys_para()->set_inharm_type(buf_pt[0]);
                break;
            case CurrClampType:
                data_buffer().equip_para()->set_current_clamp_type(buf_pt[0]);
                data_buffer().SetCclampPara(2);
                daram_update = true;
                break;
            case BaudRateSet:
                data_buffer().equip_para()->set_BaudRate(0, i);
                comm_update = true;
                break;
            case PhsAdjEnable: //相别校准使能
                ui = buf_pt[0];
                ui |= buf_pt[1] << 1;
                ui |= buf_pt[2] << 2;
                data_buffer().equip_para()->set_phs_adj_enable(ui);
                break;
            case VorC_AdjEnable: //电压电流校准使能
                ui = buf_pt[0];
                ui |= buf_pt[1] << 1;
                data_buffer().equip_para()->set_vc_adj_enable(ui);
                break;
            case kHarmTrgrEnable: //谐波触发使能
                data_buffer().sys_para()->set_trigger_enable(2, buf_pt[0]);
                data_buffer().sys_para()->set_trigger_enable(3, buf_pt[1]);
                break;
            case kUnblcTrgrEnable: //不平衡触发使能
                data_buffer().sys_para()->set_trigger_enable(4, buf_pt[0]);
                data_buffer().sys_para()->set_trigger_enable(5, buf_pt[1]);
                break;
            case kStdyTrgrEnable:   //稳态录波触发使能
                data_buffer().sys_para()->set_trigger_enable(0, buf_pt[0]);
                break;
            case kFreqTrgrEnable:   //频率触发使能
                data_buffer().sys_para()->set_trigger_enable(1, buf_pt[0]);
                break;
            case kFixFreqSmpl:   //固定频率采样
                if (buf_pt[0]>0) buf_pt[0] = 0x79;
                data_buffer().equip_para()->set_fix_smpl(buf_pt[0]);
                daram_update = true;
                break;
            case kHeartbeat61850:   //61850心跳监测
                if (buf_pt[0]>0) buf_pt[0] = 0x79;
                data_buffer().equip_para()->set_hrtbt_61850(buf_pt[0]);
                daram_update = true;
                break;
            case kVoltDvTrgrEnable:   //电压偏差触发使能
                data_buffer().sys_para()->set_trigger_enable(6, buf_pt[0]);
                break;
            case DebugEnable:
                ui = buf_pt[0] & 0x1;
                data_buffer().equip_para()->set_debug_enable(ui);
                break;
            case PstMdfyEnable:
                ui = buf_pt[0] & 0x1;
                data_buffer().equip_para()->set_Pst_mdfy_enable(ui);
                break;
            case PstEnable:
                ui = buf_pt[0] & 0x1;
                data_buffer().equip_para()->set_pst_enable(ui);
                daram_update = true;
                break;
            case kSecurityEn:
                ui = buf_pt[0] & 0x1;
                data_buffer().equip_para()->set_security_en(ui);
                break;
            case kSignalSimuEn:
                ui = buf_pt[0] & 0x1;
                data_buffer().equip_para()->set_signl_simu_en(ui);
                break;
            case SaveWaveEnable:
                ui = buf_pt[0] & 0x1;
                data_buffer().equip_para()->set_save_wave_en(ui);
                break;
            case kInterHarmEna:
                ui = buf_pt[0] & 0x1;
                data_buffer().sys_para()->set_inter_enable(ui);
                break;
            case kHarmAlmEna:
                ui = buf_pt[0]==0?0:1;
                data_buffer().equip_para()->set_alarm_enable(kAlmHarmEn, ui);
                break;
            case kFreqAlmEna:
                ui = buf_pt[0]==0?0:1;
                data_buffer().equip_para()->set_alarm_enable(kAlmFreqEn, ui);
                break;
            case kPstAlmEna:
                ui = buf_pt[0]==0?0:1;
                data_buffer().equip_para()->set_alarm_enable(kAlmPstEn, ui);
                break;
            case kUnblcAlmEna:
                ui = buf_pt[0]==0?0:1;
                data_buffer().equip_para()->set_alarm_enable(kAlmUnblcEn, ui);
                break;
            case kVoltDvAlmEna:
                ui = buf_pt[0]==0?0:1;
                data_buffer().equip_para()->set_alarm_enable(kAlmVoltdvEn, ui);
                break;
            case kHarmRecSvEn:
                ui = buf_pt[0] & 0x1;
                data_buffer().equip_para()->set_harm_rec_sven(ui);
                break;
            case Aggregation10min:
                ui = buf_pt[0] & 0x1;
                data_buffer().equip_para()->set_aggrgt_time_en(ui);
                break;
            case CurrClampEnable:
                ui = buf_pt[0] & 0x1;
                data_buffer().equip_para()->set_current_clamp_en(ui);
                data_buffer().SetCclampPara(1, ui); //设置电流钳使能
                break;
            case AdjustZeroEnable:
                ui = buf_pt[0] & 0x1;
                data_buffer().equip_para()->set_real_wave_zero(ui);
                break;
            case DeviceSn:
                sscanf(key_buf[0], "%li", &li);
                data_buffer().equip_para()->set_device_sn(li);
                break;
            case SmpVltScale:
                data_buffer().equip_para()->set_sample_pt(fi);
                daram_update = true;
                break;
            case SmpCurScale:
                data_buffer().equip_para()->set_sample_ct(fi);
                daram_update = true;
                break;
            case kSysTimeError:
                data_buffer().equip_para()->set_time_err(i);
                break;
            case VALineCoef:
            case VBLineCoef:
            case VCLineCoef:
                data_buffer().equip_para()->set_v_line_coef(type-VALineCoef, i);
                break;
            case VAResRatio:
            case VBResRatio:
            case VCResRatio:
                data_buffer().equip_para()->set_v_res_ratio(type-VAResRatio, (uint16_t)(fi*10000 + 0.001));
                daram_update = true;
                break;
            case CAResRatio:
            case CBResRatio:
            case CCResRatio:
                data_buffer().equip_para()->set_c_res_ratio(type-CAResRatio, (uint16_t)(fi*10000 + 0.001));
                daram_update = true;
                break;
            case VorCDatum: //电压电流基准值
                data_buffer().equip_para()->set_v_datum((uint16_t)(fi * 100 + 0.001));
                sscanf(key_buf[1], "%f", &fi);
                data_buffer().equip_para()->set_c_datum((uint16_t)(fi * 100 + 0.001));
                break;
            case SysIni:
            case ResetDefaultPara:
                RunIniSys(type);
                break;
            case kHarmRecSvMax:
                set_confirm_ok_cnt_++;
                if (set_confirm_ok_cnt_ >= 2) {
                    if (i < 10) {
                        i = 10;
                    } else {
                        if (i > 50000) {
                            i = 50000;
                        }
                    }
                    data_buffer().equip_para()->set_harm_rec_svmax(i);
                    notice_pthread(kTTSave, SAVEINFO, kAlterTrigger, NULL);
                    sleep(1);
                }
                break;
            case TranstRcdTime:
                set_confirm_ok_cnt_++;
                if (set_confirm_ok_cnt_ >= 2) {
                    if (i < 2) {
                        i = 2;
                    } else {
                        if (i > 600) {
                            i = 600;
                        }
                    }
                    data_buffer().equip_para()->set_transt_rcd_time(i);
                    j = data_buffer().equip_para()->transt_tol_time() * 60;
                    j /= i;
                    data_buffer().equip_para()->set_transt_max(j > 1200 ? 1200 : j + 3);
                    messageq_gui().PushCtrlSig(data_idx_, 6);    //Reset transient record
                }
                break;
            case TranstTolTime:
                set_confirm_ok_cnt_++;
                if (set_confirm_ok_cnt_ >= 2) {
                    if (i < 5) {
                        i = 5;
                    }
                    data_buffer().equip_para()->set_transt_tol_time(i);
                    j = data_buffer().equip_para()->transt_rcd_time();
                    j = (i * 60) / j;
                    data_buffer().equip_para()->set_transt_max(j > 256 ? 256 : j + 3);
                    messageq_gui().PushCtrlSig(data_idx_, 6);    //Reset transient record
                }
                break;
            case ModifyTime:
                sscanf(key_buf[0], "%hui", &time_set_.tm_year);
                sscanf(key_buf[1], "%hui", &time_set_.tm_mon);
                sscanf(key_buf[2], "%hui", &time_set_.tm_mday);
                sscanf(key_buf[3], "%hui", &time_set_.tm_hour);
                sscanf(key_buf[4], "%hui", &time_set_.tm_min);
                sscanf(key_buf[5], "%hui", &time_set_.tm_sec);
                verify_time(&time_set_);
                ui = MakeTime(&time_set_, 1);
                messageq_gui().PushCtrlSig(data_idx_, 7, ui>>16, ui&0xffff );
                break;
            case SocketServerPort:
                data_buffer().equip_para()->set_socket_server_port(i > 65535 ? 65535 : i);
                socket_update = true;
                break;
            case IPAddrSet:
                data_buffer().equip_para()->set_ip(key_buf[0]);
                netpara_update = true;
                break;
            case NTPServerIP:
                data_buffer().equip_para()->set_ntp_ip(key_buf[0]);
                ntppara_update = true;
                break;
            case NetMaskSet:
                data_buffer().equip_para()->set_nmask(key_buf[0]);
                netpara_update = true;
                break;
            case GateWaySet:
                data_buffer().equip_para()->set_gateway(key_buf[0]);
                netpara_update = true;
                break;
            case SetMacAddr:
                hwaddr[4] = i > 255 ? 255 : i;
                sscanf(key_buf[1], "%d", &i);
                hwaddr[5] = i > 255 ? 255 : i;
                sprintf(stri, "%02X:%02X:%02X:%02X:%02X:%02X",
                        hwaddr[0], hwaddr[1], hwaddr[2],
                        hwaddr[3], hwaddr[4], hwaddr[5]);
                data_buffer().equip_para()->get_mac_addr(stri);
                break;
            case ModifyPasswd: {
                clear();
                show_frame();
                i = left_+width/2-(font.cn_size+font.space)*5;
                j = top_+height/2 - font.cn_size;
                //sscanf(key_buf[0], "%li", &li);
                if (data_buffer().CheckPasswd(key_buf[0])==0) {
                    if (strcmp(key_buf[1], key_buf[2])==0) {
                        if (strlen(key_buf[1])<6) {
                            pqm_dis->puts("密码不能少于6个字符！", i, j, font.color, font.space);
                            i = 2;
                        } else {
                            data_buffer().SetPasswd(key_buf[1]);
                            pqm_dis->puts("密码修改成功！", i, j, font.color, font.space);
                            i = 1;
                        }
                    } else {
                        pqm_dis->puts("密码不一致！", i, j, font.color, font.space);
                        para_update_ = false;
                        i = 2;
                    }
                } else {
                    pqm_dis->puts("密码错误！", i, j, font.color, font.space);
                    para_update_ = false;
                    i = 2;
                }
                refresh();
                msSleep(1000);
                break;
            }
            case AutoAdj:
            case GetCharacterDCValue:
                return;
            default:
                return;
        }
        showcursor = false;
        if ( (set_confirm_ok_cnt_ < 2) &&
            (TranstRcdTime==type ||TranstTolTime==type ||kHarmRecSvMax==type)) {
            return;
        }
        curr_cmd_ = popcmd();
         
    } else if (ktype == KEY_ESC) {
        curr_cmd_ = popcmd();
        showcursor = false;
        switch (type) {
            case AutoAdj:
                messageq_gui().PushCtrlSig(data_idx_, 3, 0);
                break;
            case GetCharacterDCValue:
                messageq_gui().PushCtrlSig(data_idx_, 4, 0);
                break;
            default:
                break;
        }
        return;
    } else if (ktype == KEY_HMS) {  //switch param
        if (menu_tag_[curr_sn_]&0x4) { //设置项包含多个设置值
                cursor_left[buf_sel_] = cursorx; //把现在的指针放入对应的缓存中。
                cursor_top[buf_sel_] = cursory;
                buf_sel_++;
                buf_sel_ %= buf_sel_sz_;
                cursorx = cursor_left[buf_sel_];
                cursory = cursor_top[buf_sel_];
        } else {
            nd_rfrsh_ = false;
        }
        return;
    } else if (ktype == KEY_PHS) {  //page up
        switch (type) {
            case ShowOcTime:
                if (buf_pt[0] > 0) buf_pt[0]--;
                return;
            default:
                break;
        }
    } else if (ktype == KEY_OTH) {  //Page down
        switch (type) {
            case ShowOcTime:
                if (buf_pt[0]*8 + 8 < num_onoff) buf_pt[0]++;
                return;
            default:
                break;
        }
    }
//------------ 数据输入处理 ----------------------------------------------------
    switch (type) {
        case kTimeZone:
            if (buf_sel_==0) buf_pt[buf_sel_] = select_lst( ktype, buf_pt[buf_sel_], 25);
            else buf_pt[buf_sel_] = select_lst( ktype, buf_pt[buf_sel_], 2);
            break;
        case GpsSingleType:
            buf_pt[0] = select_lst( ktype, buf_pt[0], 4);
            break;
        case GpsPulseType:
        case kRecWaveFormat:
        case ConnectType:
        case HrmRcdSaveType:
        case FreqSaveType:
        case VoltWarpSaveType:
        case ImbalanceSaveType:
        case SetLimitType:
        case kInterHmGroup:
        case kComtradeSvPath:
        case kRecWaveSmplRt:
        case kFreqEvalTm:
        case kClockSource:
            buf_pt[0] = select_lst( ktype, buf_pt[0], 2);
            break;
        case AdSampleRate:
            buf_pt[0] = select_lst( ktype, buf_pt[0], 3);
            break;
        case FreqRcdSpace:
            buf_pt[0] = select_lst( ktype, buf_pt[0], 6);
            break;
        case CurrClampType:
            buf_pt[0] = select_lst( ktype, buf_pt[0], CClampTypeNum);
            break;
        case VoltLvl:
            buf_pt[0] = select_lst( ktype, buf_pt[0], ULevNum);
            break;
        case kUserName:
            buf_pt[0] = select_lst( ktype, buf_pt[0], data_buffer().usr_num());
            break;
        case PstMdfyEnable: //闪变修正使能
        case PstEnable:
        case kSecurityEn:
        case kSignalSimuEn:
        case SaveWaveEnable:
        case kInterHarmEna:
        case kHarmAlmEna:
        case kFreqAlmEna:
        case kPstAlmEna:
        case kUnblcAlmEna:
        case kVoltDvAlmEna:
        case kHarmRecSvEn:
        case Aggregation10min:
        case kStdyTrgrEnable:
        case kFreqTrgrEnable:
        case kFixFreqSmpl:
        case kHeartbeat61850:
        case kVoltDvTrgrEnable:
        case DebugEnable:
        case TransEnable:
        case TransCurEnable:
        case FluctuationEna:
        case PhsAdjEnable: //相别校准使能
        case VorC_AdjEnable: //电压电流校准使能
        case kHarmTrgrEnable:
        case kUnblcTrgrEnable:
        case CurrClampEnable:
        case AdjustZeroEnable:
        case StartCurEn:
        case TransTbEnable:
        case CvtModifyEn:
        case TransManualRec:
            buf_pt[buf_sel_] = select_lst( ktype, buf_pt[buf_sel_], 2);
            break;
        case CTScale:
            if ((data_buffer().equip_para()->current_clamp_en()&0x1) && buf_sel_) { //电流钳并且是二次侧
                buf_pt[buf_sel_] = select_lst( ktype, buf_pt[buf_sel_],
                        CClampRatioNum[data_buffer().equip_para()->current_clamp_type()]);
                sprintf( key_buf[buf_sel_], "%d",
                        CClampRatioSet[data_buffer().equip_para()->current_clamp_type()][buf_pt[buf_sel_]]);
                int tmp = data_buffer().GetMdfyParaCT1(buf_pt[buf_sel_]);
                sprintf(key_buf[0], "%i", tmp);
            } else {
                buf_pt[buf_sel_] = edit_num( ktype, buf_pt[buf_sel_],
                        key_buf[buf_sel_], 6 - 3 * buf_sel_);
            }
            break;
        case PTScale:
            buf_pt[buf_sel_] = edit_num( ktype, buf_pt[buf_sel_],
                    key_buf[buf_sel_], 7 - 3 * buf_sel_);
            break;
        case ZeroInputThr:
            buf_pt[buf_sel_] = edit_num( ktype, buf_pt[buf_sel_],
                    key_buf[buf_sel_], 2);
            break;
        case SetBTimeIntr:
        case SetProofTimeIntr:
        case SetMacAddr:
            buf_pt[buf_sel_] = edit_num( ktype, buf_pt[buf_sel_],
                    key_buf[buf_sel_], 3);
            break;
        case VoltWarp:
        case kInterHarmDb:
            buf_pt[buf_sel_] = edit_num( ktype, buf_pt[buf_sel_],
                    key_buf[buf_sel_], 4);
            break;
        case kCapLifeThr:
        case kCapVolt1Thr:
        case kCapVolt2Thr:
        case PstLimit:
            buf_pt[buf_sel_] = edit_num( ktype, buf_pt[buf_sel_],
                    key_buf[buf_sel_], 5);
            break;
        case TransLimit:
            if (buf_sel_>0) {
                buf_pt[buf_sel_] = edit_num( ktype, buf_pt[buf_sel_],
                        key_buf[buf_sel_], 4);
            } else {
                buf_pt[buf_sel_] = edit_num( ktype, buf_pt[buf_sel_],
                        key_buf[buf_sel_], 5);
            }
            break;
        case VorCDatum: //电压电流基准值
        case TransCurLimit:
        case ImbalanceLimit:
        case FrequecyLimit:
        case SetHarmLimitSub:
        case StartCurLimit:
        case FluctuationDb:
        case TransTbLimit:
        case BaudRateSet:
            buf_pt[buf_sel_] = edit_num( ktype, buf_pt[buf_sel_],
                    key_buf[buf_sel_], 6);
            break;
        case ModifyPasswd:
            buf_pt[buf_sel_] = edit_num( ktype, buf_pt[buf_sel_],
                    key_buf[buf_sel_], 8);
            break;
        case ModifyTime:
            if (buf_sel_ == 0)
                i = 4;
            else
                i = 2;
            buf_pt[buf_sel_] = edit_num( ktype, buf_pt[buf_sel_], key_buf[buf_sel_], i);
            break;
        case UnitNum:
        case kFilterCP:
        case kTimeDiff:
            buf_pt[0] = edit_num( ktype, buf_pt[0], key_buf[0], 2);
            break;
        case ResetHysTime:
        case TranstRcdTime:
        case TransEndNum:
        case kAuditLogSize:
        case LCMDelayTime:
        case TranstTolTime:
        case HarmRcdSpace:
        case VoltWarpRcdSpace:
        case ImbalanceRcdSpace:
            buf_pt[0] = edit_num( ktype, buf_pt[0], key_buf[0], 3);
            break;
        case kCapW24hThr:
            buf_pt[0] = edit_num( ktype, buf_pt[0], key_buf[0], 4);
            break;
        case kMdfyEvnStm:
        case kSysTimeError:
        case VALineCoef:
        case VBLineCoef:
        case VCLineCoef:
            buf_pt[0] = edit_num( ktype, buf_pt[0], key_buf[0], 4, true);
            break;
        case SocketServerPort:
        case kCapCurrThr:
        case kCapCapThr:
        case kCapPeakThr:
        case kCapGamaThr:
        case kTrnsfmrPec:
        case kCapReactancex:
        case kImpedanceBeta:
        case kHarmRecSvMax:
            buf_pt[0] = edit_num( ktype, buf_pt[0], key_buf[0], 5);
            break;
        case kTransformerMs:
        case kTrnsfmrIn:
        case kCapNmnlMc:
        case kCapNmnlUc:
        case VAResRatio:
        case VBResRatio:
        case VCResRatio:
        case CAResRatio:
        case CBResRatio:
        case CCResRatio:
        case CvtModifyKSub:
            buf_pt[0] = edit_num( ktype, buf_pt[0], key_buf[0], 6);
            break;
        case SmpVltScale:
            buf_pt[0] = edit_num( ktype, buf_pt[0], key_buf[0], 7);
            break;
        case ShortCap:
        case UserCap:
        case SuppCap:
        case SmpCurScale:
        case NegativeILimit:
        case kHarmonic10Cyc:
            buf_pt[0] = edit_num( ktype, buf_pt[0], key_buf[0], 8);
            break;
        case DeviceSn:
            buf_pt[0] = edit_num( ktype, buf_pt[0], key_buf[0], 9);
            break;
        case kPassword:
            buf_pt[0] = edit_num( ktype, buf_pt[0], key_buf[0], 10);
            break;
        case IPAddrSet:
        case NTPServerIP:
        case NetMaskSet:
        case GateWaySet:
            buf_pt[0] = edit_num( ktype, buf_pt[0], key_buf[0], 15);
            break;
        default:
            nd_rfrsh_ = false;
            break;
    }
}

//-------------------------------------------------------------------------
//Whether display be refresh when harm data refresh
int CSetView::harm_refresh()
{
    if (curr_cmd_ == AutoAdj || curr_cmd_ == GetCharacterDCValue) {
        return true;
    }
    return false;
}
