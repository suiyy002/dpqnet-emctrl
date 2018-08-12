#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace std;

#include "view.h"
//#include "../device/device.h"
#include "debug_info.h"
#include "setform.h"
#include "form/mainframe.h"
#include "form/mainform.h"
#include "form/harmform.h"
#include "form/otherform.h"
#include "../data/data_buffer.h"
#include "../thread/pthread_mng.h"
#include "display/vga_color.h"

static const int SHOW_Y_SCALE = 5; //Ƶ��ͼ������ʾ��������λ��
static const int SHOW_Y_PSCALE = 1; //����Ƶ��ͼ������ʾ��������λ��

int ViewPqm::SHOW_MAX_HARM_NUM = 50;
    
ViewPqm::ViewPqm(SocketClient * sock)
{
    sock_clnt_ = sock;
    ini_variable();

    char str[16];
    sprintf(str, "%4dA", data_buffer().GetRatio(2));
    main_frame->label_clamp->set_txt(str);
    main_frame->label_clamp->set_visible(data_buffer().equip_para()->current_clamp_en() & 0x1);
    battery_qnty = 20;
    main_frame->battery_indicator->set_power(battery_qnty);
    sys_prompt_valid_ = 0;
}

ViewPqm::~ViewPqm()
{
    delete dis_buf_;
}

/*!
bind socket

    Input:  name -- port number or filename
            hostname -- hostname or ip. only for internet
            phy_t -- physical layer protocol
            app_t -- application layer protocol
            intvl -- auto reconnect interval time. unit:s, 0 = not auto reconnect
    Return: index of pl_fd_(range:1-max_connect_), or <=0 in case of error
*/
void ViewPqm::BindSocket(const char *name, const char *hostname, int phy_t, int app_t, int intv)
{
    strncpy(clnt_inf_.name, name, sizeof(clnt_inf_.name));
    strncpy(clnt_inf_.hostname, hostname, sizeof(clnt_inf_.hostname));
    clnt_inf_.phy_prtcl = phy_t;
    clnt_inf_.app_prtcl = app_t;
    clnt_inf_.recnct_intvl = intvl;
    
    if (sock_clnt_) {
        data_idx_ = sock_clnt_->Start(name, hostname, phy_t, app_t, intv) - 1;
    }
}

//ϵͳ��Ϣ��ʾ����
//ÿ0.5s ������һ��
void ViewPqm::sys_prompt_treat()
{
    static int show_sys_dly = 0; //ϵͳ��Ϣ��ʾ��ʱ
    static int stat = 0;
    char stri[64];

    if (show_sys_dly > 0) {
        show_sys_dly --;
        if (show_sys_dly <= 0) {
            sys_prompt_valid_ = 0;
        }
    }

    if (stat == sys_prompt_valid_) return;
    if (!stat) main_frame->label_sys->set_visible(true);
    stat = sys_prompt_valid_;
    if (!stat) {
        main_frame->label_sys->set_visible(false);
        main_frame->label_help->set_visible(true);
        return;
    }
    main_frame->label_sys->font.color = vgacolor(kVGA_HiYellow);
    strcpy(stri, " ");
    switch (stat) {
        case kBtryPowerLow:
            strcat(stri, "��ص����ͣ��� ");
            show_sys_dly = 4;
            break;
        /*case kManualRecWvStr:
            strcat(stri, "��ʼ¼��...");
            show_sys_dly = 300;
            break;*/
        case kManualRecWvF:
            strcat(stri, "�����¼��������޷������ֶ�¼������");
            main_frame->label_sys->font.color = vgacolor(kVGA_HiRed);
            show_sys_dly = 7;
            break;
        case kSaveRec10Str:
            strcat(stri, "��ʼ�洢10�ܲ�����..., �����������");
            main_frame->label_sys->font.color = vgacolor(kVGA_HiYellow);
            show_sys_dly = 500;
            break;
        case kSaveRec10End:
            strcat(stri, "�����洢10�ܲ����ݡ�");
            main_frame->label_sys->font.color = vgacolor(kVGA_HiYellow);
            show_sys_dly = 7;
            break;
        case kSettingLocked:
            strcat(stri, "���ý��汻������");
            main_frame->label_sys->font.color = vgacolor(kVGA_HiYellow);
            show_sys_dly = 3;
            break;
            
    }
    main_frame->label_sys->set_txt(stri);
}

//��ص�����⴦��
void ViewPqm::battery_treat()
{
    static int cnti = 0;
    int qnty;

    if ((cnti++) % 6 == 0) {
        qnty = data_buffer().battery_power();
        if (qnty != battery_qnty) {
            battery_qnty = qnty;
            main_frame->battery_indicator->set_power(battery_qnty);
            if (battery_qnty < BATTRY_POWER_LOW && battery_qnty > BATTRY_POWER_LOW - 3) { //������
                /*
                if (!notice_pthread(kTTSave, SAVEINFO, kFlushFileAll, NULL)) {
                    sys_prompt_valid_ = FLUSH_FILE_B;
                }*/
            }
        }
    }

    if (battery_qnty > 100) { //���ڳ��
        main_frame->battery_indicator->set_power(battery_qnty);
    }
    if (battery_qnty <= BATTRY_POWER_LOW) { //������
        main_frame->battery_indicator->note_power_low();
        if ((cnti % 6) == 0) {
            sys_prompt_valid_ = kBtryPowerLow;
        }
    }
}

//-------------------------------------------------------------------------
void ViewPqm::view_frame()
{
    float fi;
    int i;
    char str[32];

    switch (msg_type) {
        case MSG_SWITCH_VIEW:
            //���ý���ѡ���ǩ
            main_frame->tabctrl_view->set_tab_index(view_num);
            //�趨��Ԫ���
            sprintf(str, "%2i", data_buffer().equip_para()->device_num());
            main_frame->label_unitnum->set_txt(str);
            main_frame->set_update();
            //���µ���ǯ���
            main_frame->label_clamp->set_visible(data_buffer().equip_para()->current_clamp_en() & 0x1);
            break;
        case MSG_SWITCH_CCLAMP_RATIO: //�л�����ǯ���
            data_buffer().SetCclampPara(0);
            break;
        case MSG_SWITCH_PHS:
            main_frame->tabctrl_phs->set_tab_index(power_phs_);
            break;
        case MSG_WAVE_DATA:
            fi = data_buffer().frequency(); // frequency
            fi /= 1000;
            i = float_fmt(fi, 4, 2);
            sprintf(str, "%4.*fHz", i, fi);
            main_frame->label_frq->set_txt(str);
            break;
        case MSG_CLOCK_UPDATE:
            main_frame->label_clock->set_txt(data_buffer().now_tm());
            //battery_treat();
            sys_prompt_treat();
            break;
        default:
            break;
    }
    if (msg_type == MSG_SWITCH_VIEW || msg_type == MSG_SWITCH_CCLAMP_RATIO) {
        sprintf(str, "%4dA", data_buffer().GetRatio(3));
        main_frame->label_clamp->set_txt(str);
    }
}

//-------------------------------------------------------------------------
//�趨����ͼ��Y�ᵥλ�̶ȱ�ע.
inline void ViewPqm::set_iwav_scale()
{
    float fi;
    int i;
    static int scale = 1;
    static int ct = 0;

    i = data_buffer().GetRatio(2);  // CT1
    if ((i % 10) != 0 && i % 6 == 0) full_scale = 12;
    else full_scale = 10;
    fi = data_buffer().rms(1, power_phs_);
    if (fi == 0) {
        iwav_scale = full_scale;
    } else {
        iwav_scale = fi * full_scale / i + 1;
    }
    if (iwav_scale > full_scale || fi <= 0.001) iwav_scale = full_scale;
    if (iwav_scale != scale || ct != i) {
        scale = iwav_scale;
        ct = i;
        fi = i;
        main_form->chart_i->set_scale_val(1, fi * scale / full_scale);
    }
}

//-------------------------------------------------------------------------
void ViewPqm::view_main()
{
    float fi;
    int i, j;
    char str[32];
    short *buf;
    int vnum = main_form->view_num();

    switch (msg_type) {
        case MSG_SWITCH_VIEW: //�л�����
        case MSG_SWITCH_VIEW2: //�л�2������
            main_form->set_update();
            //���ý���ѡ���ǩ
            main_form->SwitchView();
            if (data_buffer().equip_para()->current_clamp_en()&0x1 
                    && CClampRatioNum[data_buffer().equip_para()->current_clamp_type()] > 1) {
                main_frame->label_help->set_txt("ȷ��:ϵͳ��������  2,8:�л�����  9:�л�����ǯ����");
            } else {
                main_frame->label_help->set_txt("ȷ��:ϵͳ��������  2,8:�л�����");
            }
            if (vnum == 0) {    //���ν���
                main_frame->tabctrl_phs->enable_tab(true);
                //�趨��ѹͼ��Y�ᵥλ�̶ȱ�ע.
                fi = data_buffer().GetRatio(0); //PT1
                if (!data_buffer().sys_para()->connect_t()) { //Y�ͽ���
                    fi *= 0.57737;
                }
                fi /= 1000;
                main_form->chart_u->set_scale_val(1, fi);
            } else {
                main_frame->tabctrl_phs->enable_tab(false);
            }
            break;
        case MSG_SWITCH_PHS: //�л����
            if (!vnum) main_form->set_update();
        case MSG_WAVE_DATA: //ʵʱ����
            if (vnum) { //���ǲ��ν���
                msg_type = 0;
                break;
            }
            if (man_rec_!=data_buffer().manual_state()) {  //�ֶ�¼��״̬�����仯
                if (data_buffer().manual_state()<2) {
                    man_rec_ = data_buffer().manual_state();
                    main_form->label_rcd_wave->set_visible(man_rec_);
                    main_form->label_rcd_wave->set_update(man_rec_);
                    main_form->set_update();
                }
            }
            buf = data_buffer().waveform(0, &i);    //pqmfunc->get_real_pt(0, i);
            if (data_buffer().equip_para()->real_wave_zero() & 0x1) j = 0; //ȥ����������
            else j = chk_zero(buf, i / 2 + 16);
            show_wave(0, j); // Draw the wave of voltage.
            show_wave(1, j); // Draw the wave of current.
            if (MSG_WAVE_DATA == msg_type) break;
        case MSG_HARM_DATA:     //г�����ݸ���
            show_mlabel();      //show voltage&current data
            set_iwav_scale();   //�趨����ͼ��Y�ᵥλ�̶ȱ�ע.
            break;
        case MSG_CHG_HARMNUMS:
        case MSG_SWITCH_DBG:
        case MSG_CLOCK_UPDATE:
        default:
            msg_type = 0;
            break;
    }
}

/* -----------------------------------------------------------------------------
Description:show label type data at main view
----------------------------------------------------------------------------- */
void ViewPqm::show_mlabel()
{
    float fi;
    char str[32];
    int i;

    //��ѹ������
    fi = data_buffer().rms(0, power_phs_);
    //fi = harmfunc->u_rms(1, power_phs_);
    if (fi >= 1000) {
        fi /= 1000;
        i = float_fmt(fi, 5, 3);
        sprintf(str, "%6.*fkV", i, fi);
    } else {
        i = float_fmt(fi, 5, 2);
        sprintf(str, "%6.*fV", i, fi);
    }
    main_form->label_urms->set_txt(str);
    //��ѹг���ܻ�����
    fi = data_buffer().GetHarmonic(kHarmTHD, power_phs_, 0);
    i = float_fmt(fi, 5, 3);
    sprintf(str, "%6.*f%%", i, fi);
    main_form->label_THDu->set_txt(str);
    //����������
    //fi = harmfunc->i_rms(1, power_phs_);
    fi = data_buffer().rms(1, power_phs_);
    i = float_fmt(fi, 5, 3);
    sprintf(str, "%6.*fA", i, fi);
    main_form->label_irms->set_txt(str);
    //����г���ܻ�����
    fi = data_buffer().GetHarmonic(kHarmTHD, power_phs_, 1);
    
    i = float_fmt(fi, 5, 3);
    sprintf(str, "%6.*f%%", i, fi);
    main_form->label_THDi->set_txt(str);
}

//-------------------------------------------------------------------------
// Read harmonic ��ѹ������г��������,�й��޹�
//type=0,voltage; type=1,current; type=2,�й�power; type=3,�޹�power
// phs,����;hms,г������
// all_hr=0:����Ϊ�ܻ�����,all_hr=1:����Ϊ������
void ViewPqm::read_rms_hr(int phs, int type, int hms, char *str, int all_hr)
{
    char stri[64];
    char chj = 'I';
    char *strp;
    float fi, fj, fk;
    int i, j, k;

    if (type < 2) { //г��������
        int vc = type;
        strp = "A   ";
        fi = data_buffer().GetHarmonic(kHarmAmp1, phs, vc, hms);
        if (harm_nums == 1 && all_hr == 0) { //����
            fj = data_buffer().GetHarmonic(kHarmTHD, phs, vc);
        } else {
            fj = data_buffer().GetHarmonic(kHarmHR, phs, vc, hms);
        }
        fk = data_buffer().GetHarmonic(kHarmPhsAng, phs, vc, hms);

        if (vc == 0) {
            chj = 'U';
            if (fi < 1000) {
                strp = "V   ";
            } else {
                fi /= 1000;
                strp = "kV  ";
            }
        }
        i = float_fmt(fi, 5, 3);
        j = float_fmt(fj, 5, 2);
        k = float_fmt(fk, 4, 1);
        sprintf(stri, "%c(%2i) %7.*f%s%6.*f%% %6.*f��", chj,
                hms, i, fi, strp, j, fj, k, fk);
    } else { //г������
        fi = data_buffer().GetPower(kHrmPowerP+type-2, phs, hms);

        if ((type - 2) == 0) {
            chj = 'P';
            if (fi < 1000) {
                strp = " kW";
            } else {
                fi /= 1000;
                strp = " MW";
            }
        } else {
            chj = 'Q';
            if (fi < 1000) {
                strp = " kVar";
            } else {
                fi /= 1000;
                strp = " MVar";
            }
        }
        i = float_fmt(fi, 6, 4);
        sprintf(stri, "%c(%2i)  %7.*f%s", chj,
                hms, i, fi, strp);
    }

    strcpy(str, stri);
}

//-------------------------------------------------------------------------
void ViewPqm::view_other()
{
    char str[36];
    int i, j;
    int iwarp, ipst, iplt;
    float fwarp, fpst, fplt;
    char strwarp[36], strpst[36], strplt[36];
    float fi;
    char chj = 'A';
    int ovnum, ovnum2;
    other_form->get_view_num(ovnum, ovnum2);

    switch (msg_type) {
        case MSG_SWITCH_VIEW: //�л�����
        case MSG_SWITCH_VIEW2: //�л�2������
            other_form->set_update();
            main_frame->tabctrl_phs->enable_tab(false); //��ֹ��λѡ���ǩ
            main_frame->label_help->set_txt("����:����������");
            if (ovnum == 2) debug_info_handle(1);

        case MSG_HARM_DATA: //г�����ݸ���
            if (ovnum == 1) { //�ڵ��Խ���1
                if (ovnum2) { //��ʾdaram
                    other_form->set_update();
                } else {
                    show_debug();
                }
                break;
            } else if (ovnum == 2) { //�ڵ��Խ���2
                break;
            }
            other_form->labelset_lu->set_visible(false);
            if (data_buffer().LackPhs(0)) { //��ѹ��·���߲�ȫ
                other_form->label_lack_phsu->set_visible(true);
                other_form->labelset_imbalanceu->set_visible(false);
            } else {
                other_form->label_lack_phsu->set_visible(false);
                other_form->labelset_imbalanceu->set_visible(true);
                if (!data_buffer().sys_para()->connect_t()) { //Y�ͽ���
                    other_form->labelset_lu->set_visible(true);
                    char chi = 'a';
                    for (i = 0; i < 3; i++) { // Display �ߵ�ѹ.
                        fi = data_buffer().u_rms_ppv(i);
                        j = float_fmt(fi, 5, 3);
                        sprintf(str, "U%c%c:%6.*f", chi + i, chi + (i + 1) % 3, j, fi);
                        other_form->labelset_lu->set_txt(i + 1, str);
                    }
                }
                show_imbalance(0); //��ʾ��ѹ��ƽ��
            }
            if (data_buffer().LackPhs(1)) { //������·���߲�ȫ
                other_form->label_lack_phsi->set_visible(true);
                other_form->labelset_imbalancei->set_visible(false);
            } else {
                other_form->label_lack_phsi->set_visible(false);
                other_form->labelset_imbalancei->set_visible(true);
                show_imbalance(1); //��ʾ������ƽ��
            }
            show_vector_chart();    //Showʸ��ͼ

            for (i = 0; i < 3; i++) {
                // Display ��ѹƫ��.
                fwarp = data_buffer().u_deviation(i);
                iwarp = float_fmt(fi, 4, 2);
                // Display Pst.
                fpst = data_buffer().pst(i);
                ipst = float_fmt(fpst, 5, 3);
                // Display Plt.
                fplt = data_buffer().plt(i);
                iplt = float_fmt(fplt, 5, 3);
                if (data_buffer().sys_para()->connect_t()) {//�����νӷ�

                    sprintf(strwarp, "%c%c:%4.*f%%", chj + i, chj + (i + 1) % 3, iwarp, fwarp);
                    sprintf(strpst, "%c%c:%5.*f", chj + i, chj + (i + 1) % 3, ipst, fpst);
                    sprintf(strplt, "%c%c:%5.*f", chj + i, chj + (i + 1) % 3, iplt, fplt);
                } else {
                    sprintf(strwarp, "%c:%4.*f%%", chj + i, iwarp, fwarp);
                    sprintf(strpst, "%c:%5.*f", chj + i, ipst, fpst);
                    sprintf(strplt, "%c:%5.*f", chj + i, iplt, fplt);
                }
                other_form->labelset_warpu->set_txt(i + 1, strwarp);
                other_form->labelset_Pst->set_txt(i + 1, strpst);

                other_form->labelset_Plt->set_txt(i + 1, strplt);
            }

            break;
        case MSG_SWITCH_DBG:
            break;
        case MSG_CLOCK_UPDATE:
            if (ovnum == 2) debug_info_handle();
            break;
        default:
            msg_type = 0;
            break;
    }
}

//-------------------------------------------------------------------------
void ViewPqm::view_harm()
{
    char stri[48];
    int x, y, i, hms;
    int hmvnum = harm_form->view_num;

    switch (msg_type) {
        case MSG_SWITCH_VIEW: //�л�����
        case MSG_SWITCH_VIEW2: //�л�2������
            harm_form->set_update();
            //���ý���ѡ���ǩ
            harm_form->switch_view();
            main_frame->tabctrl_phs->enable_tab(true);
            if (hmvnum) { //Ƶ��ͼ����
                main_frame->label_help->set_txt("����:����������  4,6:ѡ��г������  2,8:�л�����");
            } else {
                main_frame->label_help->set_txt("����:����������  4,6:��ҳ  2,8:�л�����");
            }
        case MSG_HARM_DATA: //г�����ݸ���
        case MSG_SWITCH_PHS: //�л����
            if (hmvnum) { //Ƶ��ͼ����
                // Show frequency chart of harmonics.
                show_frq_chart(power_phs_, hmvnum - 1);
            }
            if (hmvnum > 2) {
                show_power_param();//��ʾ���ʲ���
            } else {
                show_thd();// ��ʾ��г��������
            }
        case MSG_CHG_HARMNUMS:
            if (hmvnum) { //Ƶ��ͼ����
                //��ʾƵ��ͼ
                if (judge_frq_chart_page()) {
                    show_frq_chart(power_phs_, hmvnum - 1);
                }
                //��ʾ��ǰг������
                read_rms_hr(power_phs_, hmvnum - 1, harm_nums, stri);
                harm_form->label_harm->set_txt(stri);
                //��ʾг��ָʾ���
                y = harm_form->label_harm->top + harm_form->label_harm->height;
                x = harm_nums - frq_chart_page * HARM_NUM_PERPG - 1 ;
	            if (dis_buf_->disbufw()==480) {  //480*272
                    x *= 13;
                    x += harm_form->label_harm->left + 8;
                } else {    //320*240
                    x *= 8;
                    x += harm_form->label_harm->left + 3;
                }
                harm_form->pos_cursor->move(x, y);
            } else {
                // ��ʾ���ε�ѹ����г��������.
                i = 0;
                hms = harm_nums - 1;
                int nums = harm_form->hmnum_per_page;
                hms /= nums;
                hms = hms * nums + 1;
                harm_nums = hms;
                harm_form->labelset_harm->clear();
                while (i < nums && hms <= SHOW_MAX_HARM_NUM) {
                    read_rms_hr(power_phs_, 0, hms, stri);  // U
                    harm_form->labelset_harm->set_txt(2 * i, stri);
                    read_rms_hr(power_phs_, 1, hms, stri);  // I
                    harm_form->labelset_harm->set_txt(2 * i + 1, stri);
                    i ++;
                    hms ++;
                }
            }
            break;
        default:
            msg_type = 0;
            break;
    }
}

//-------------------------------------------------------------------------
// show vector chart of ����.
void ViewPqm::show_vector_chart()
{
    int i, j;
    float fi, fj;
    CChartVector *pvctrchart;
    static unsigned char connect_type = 100;

    for (j = 0; j < 2; j++) {
        if (j) {
            pvctrchart = other_form->vectorchart_i;
        } else {
            pvctrchart = other_form->vectorchart_u;
        }
        if (connect_type != data_buffer().sys_para()->connect_t()) {
            if (data_buffer().sys_para()->connect_t()) {
                pvctrchart->set_data_label(0, "AB");
                pvctrchart->set_data_label(1, "BC");
                pvctrchart->set_data_label(2, "CA");
            } else {
                pvctrchart->set_data_label(0, "A");
                pvctrchart->set_data_label(1, "B");
                pvctrchart->set_data_label(2, "C");
            }

        }
        for (i = 0; i < 3; i++) {
            fi = data_buffer().GetHarmonic(kHarmAmp1, i, j, 1);
            fj = data_buffer().GetHarmonic(kHarmPhsAng, i, j, 1);
            pvctrchart->add_data(i, fi, fj, phs_color[i]);
        }
    }
    if ( connect_type != data_buffer().sys_para()->connect_t() )
        connect_type = data_buffer().sys_para()->connect_t();
}

//-------------------------------------------------------------------------
void ViewPqm::view_set()
{
    int i = 0;

    switch (msg_type) {
        case MSG_HARM_DATA: //г�����ݸ���
            if (set_form->harm_refresh()) {
                i = 1;
            }
            break;
        case MSG_SWITCH_VIEW: //�л�����
            main_frame->tabctrl_view->enable_tab(false);
            main_frame->tabctrl_phs->enable_tab(false);
            main_frame->label_help->set_txt("����:ѡ������ F1:�л��༭�� F2:�˸�/��ҳ F3:��ҳ");
        case MSG_PRESS_KEY: //�м�����
            i = 1;
            break;
        default:
            msg_type = 0;
            break;
    }
    if (i) {
        set_form->set_update();
    }
}

//-------------------------------------------------------------------------
void ViewPqm::ini_variable()
{
    dis_buf_ = new DisplayBuf();
    dis_buf_->input_font_lib(ASC_8X10);
    dis_buf_->input_font_lib(ASC_8X12);
    dis_buf_->input_font_lib(ASC_8X16);
    dis_buf_->input_font_lib(CHINESE_12X12);
    dis_buf_->input_font_lib(CHINESE_14X14);
    dis_buf_->set_font(14, 8);

    main_frame = new CMainFrame;
    main_form = new CMainForm;
    harm_form = new CHarmForm;
    other_form = new COtherForm;
    set_form = new CSetForm;
    view_num = VW_MAIN;
    power_phs_ = 0;
    test1 = test2 = test3 = 0;
    msg_type = MSG_SWITCH_VIEW;
    phs_color[0] = vgacolor(kVGA_HiYellow);
    phs_color[1] = vgacolor(kVGA_HiGreen);
    phs_color[2] = vgacolor(kVGA_HiRed);
    harm_nums = 1;
    bar_scale = 1;
    debug_show = 0;
    tstcnt = 0;
    frq_chart_page = 0;

    memset(phs_errcnt, 0, sizeof(phs_errcnt));
}
//------------------------------------------------------------

// ��λA���ѹʵʱ�������ι����
// Input: buf, A���ѹʵʱ�������ݻ������ʼ��ַ
//    numi, ��λ���������ʵʱ�������ݵĸ���
// Return: ������ڻ����е����
int ViewPqm::chk_zero(short *buf, int numi)
{
    int i, k, m, n, num;
    int regi[6];

    num = numi;
    if (buf[0] >= 0) {
        k = 1;
    } else {
        k = -1;
    }
    n = 0;
    regi[n] = 0;
    for (i = 0; i < num; i++) { //�жϹ����
        m = k * buf[i];
        if (m < 0) {
            if (abs(m) > abs(buf[i - 1])) {
                regi[n] = i - 1;
            } else {
                regi[n] = i;
            }
            k = -k;
            if (++n >= 5) break;  //����㳬��5���������ź�ʱ����ִ������
        }
    }
    k = regi[0];
    for (i = 1; i < n; i++) { //�����һ�����ϵĹ���㣬ѡ���Ӧ��ֵ����
        m = regi[i];
        if ( buf[m + 16] > buf[k + 16] ) {
            k = m;
        }
    }
    /*  if(buf[k+16]<0){//�ж������������������ɸ������Ĺ���㡣
            k += 32;
        }
        n = 0; //�����������
        k += 3;
        i = k;
        while(buf[k]>0&&n<6){
            n++;
            i = k;
            k--;
            k = k<0?numi-1:k;
        }
    */

    if (buf[k] > 0) {
        i = k - 1;
    } else {
        i = k + 1;
    }
    char str[40];

    if (debug_show) { //For debug
        sprintf(str, "%02d %5d %5d", k, buf[k], buf[i]);
        main_form->label_debug2->set_txt(str);
    }

    return k;
}

//------------------------------------------------------------
//��ʾ������Ϣ
//rfrs(I): =1 ǿ��ˢ��
void ViewPqm::debug_info_handle(int rfrs)
{
    static int cnti = 0;
    int sz, i;
    cnti++;

    pthread_mutex_lock(&debug_mutex);
    sz = debugInfo().Size();
    if (rfrs) sz = 6;
    if (cnti > 6 || sz > 5) {
        cnti = 0;
        if (sz > 0) {
            debugInfo().InitPopdown(10);
            for (i = 0; i < 10; i++) {
                other_form->labelset_debug->set_txt(
                    i, debugInfo().Popdown());
            }
        }
    }
    pthread_mutex_unlock(&debug_mutex);
}
//------------------------------------------------------------
//��ʾ���Խ���
void ViewPqm::show_debug()
{
    char stri[48];
    int i = 0;

    //show count of comm,daram,display pthread
    other_form->labelset_debug->set_txt(i, stri);
    i++;
    i++;
    //other_form->labelset_debug->set_txt(i, stri);
    i++;
    sprintf(stri, "SwitchIn:%d %d", data_buffer().switch_in(0), data_buffer().switch_in(1));
    other_form->labelset_debug->set_txt(i, stri);
    i++;
    i++;
    sprintf(stri, "MAIN BOARD: %s V%i.%i.%i.%i", "PQNet310D", VER_1st, VER_2nd,
            VER_3rd, VER_sn);
    other_form->labelset_debug->set_txt(i, stri);
    i++;
}

//-------------------------------------------------------------------------
// show fraquency chart of harmonics.
//type=0,voltage; type=1,current; type=2,�й�power; type=3,�޹�power
void ViewPqm::show_frq_chart(int phs, int type)
{

    int i, j, k, dscale;
    float fi, fj, buf[HARM_NUM_PERPG], max, min;
    CChart * pchart = harm_form->chart_harm;

    for (i = 0; i < HARM_NUM_PERPG; i++) {
        buf[i] = 0;
    }
    pchart->x_dotnum = HARM_NUM_PERPG; //������ʾ����
    k = HARM_NUM_PERPG * frq_chart_page;
    if (type < 2) { //��ȡг��������
        for (i = 0; i < 25; i++) {
            fi = data_buffer().GetHarmonic(kHarmHR, phs, type, k + i + 1);
            buf[i] = fi;
        }
        dscale = SHOW_Y_SCALE;
    } else {  //��ȡг�����ʲ�����ɹ��ʺ�����
        fj = data_buffer().GetPower(kHrmPowerP+type-2, phs, 1);
        
        if (fj < 0) fj = -fj;
        if (fj) {
            for (i = 0; i < 25; i++) {
                fi = data_buffer().GetPower(kHrmPowerP+type-2, phs, k + i + 1);
                buf[i] = fi * 100 / fj;
            }
        } else {
            for (i = 0; i < 25; i++) {
                buf[i] = 0;
            }
        }
        dscale = SHOW_Y_PSCALE;
    }

    max = 0;
    min = 0;
    for (i = 0; i < 25; i++) { //�ó�������Сֵ
        fi = buf[i];
        if (fi > max && (k + i) > 1) {
            max = fi;
        }
        if (fi < min && (k + i) > 1) {
            min = fi;
        }
    }
    //���ݺ����ʼ�����ʾ����
    max += 0.1;
    max = max > 100 ? 100 : max;
    fj = max;
    if (min < 0) {
        min = min < -100 ? -100 : min;
        max -= min;
        min = -(min - 0.1);
    }
    bar_scale = max / dscale + 1;

    fi = dscale * bar_scale / 5.0; //����Y��ÿ�̶ȶ�Ӧ����ֵ
    if (fi < 1) pchart->axis_y.val_decimal = 1;
    else pchart->axis_y.val_decimal = 0;
    //�趨x���λ��
    if (buf[0] < 0) {
        pchart->axis_x.position = 1000 - pchart->axis_y.ticks_spc * fj / fi;
    } else {
        pchart->axis_x.position = pchart->axis_y.ticks_spc * min / bar_scale;
    }
    pchart->set_start_val(0, k);//�趨Ƶ��ͼX��̶ȱ�ע����ʼֵ.
    pchart->set_scale_val(1, fi);//�趨Ƶ��ͼY�ᵥλ�̶ȱ�ע.
    pchart->set_y_scale(fi); //�趨Y�ᵥλ�̶ȶ�Ӧ����ֵ

    pchart->ClearData();
    pchart->add_dataset(0, buf, HARM_NUM_PERPG, phs_color[power_phs_]);
}

//-------------------------------------------------------------------------
//�ж�Ƶ��ͼ��ǰΪ�ڼ�ҳ
//return = true:ҳ�ı䣬=false:ҳδ��
bool ViewPqm::judge_frq_chart_page()
{
    static int page = 0;
    bool retval;
    retval = false;
    frq_chart_page = (harm_nums - 1) / HARM_NUM_PERPG; //��ǰ��ʾ���ǵڼ�ҳ
    if (frq_chart_page != page) {
        retval = true;
        page = frq_chart_page;
    }
    return retval;
}

/*!
Handle message

    Input:  majortype
            minortype
    Return: 0=success, 
*/
void ViewPqm::HandleMsg(int majortype, int minortype)
{
    static int lcm_on_delay = 2 * 60 * data_buffer().equip_para()->lcm_dely_time();
    static char lcm_power_on = true;

    if (data_idx_>=0) {
        if (majortype == kKeyInfo) { //������Ϣ
            if (!lcm_power_on) {
                lcm_power_on = true;
                msg_type = MSG_SWITCH_VIEW;
                //lcm_poweron();
            } else {
                msgkey_treat(minortype);
            }
            lcm_on_delay = 2 * 60 * data_buffer().equip_para()->lcm_dely_time();
        } else if (majortype == kPTimerInfo) { //��ʱ����Ϣ
            HandleMsgTimer(minortype);
        } else if (majortype == kGUICommuCmd) { //���ݸ�����Ϣ
            if (!lcm_power_on) {
                return;
            }
            msgdata_treat(minortype);
        }
    }
    dis_treat();
    refresh();
    return retval;
}

//-------------------------------------------------------------------------
//���ݸ�����Ϣ����
void ViewPqm::msgdata_treat(int dftype)
{

    switch (dftype) {
        case kWaveformT:
            msg_type = MSG_WAVE_DATA;
            break;
        case kHarmonicT:
            msg_type = MSG_HARM_DATA;
            break;
        default:
            msg_type = 0;
            break;
    }
}

//-------------------------------------------------------------------------
//������Ϣ����
void ViewPqm::msgkey_treat(int ktype)
{

    msg_type = 0;
    switch (view_num) {
        case VW_MAIN:
            view_main_key(ktype);
            break;
        case VW_HARM:
            view_harm_key(ktype);
            break;
        case VW_OTHER:
            view_other_key(ktype);
            break;

        case VW_SET:
            view_set_key(ktype);
            break;
        default:
            view_num = VW_MAIN;
            break;
    }
}

//-------------------------------------------------------------------------
//ʱ�Ӹ�����Ϣ����
void ViewPqm::HandleMsgTimer(int tmtype)
{
    msg_type = MSG_CLOCK_UPDATE;
    set_form->show_cursor();
    if (lcm_power_on) {
        if (data_buffer().equip_para()->lcm_dely_time()<999) lcm_on_delay--;
        if (lcm_on_delay <= 0) {
            //apm_ctrl(0);
            //lcm_poweroff();
            lcm_power_on = false;
        }
    }
    if (lcm_on_delay <= 1) { //�ر�LCM��Դǰ
        if (view_num == VW_SET) { //��������ý��棬���˻�������
            view_num = VW_MAIN;
            set_form->return_from_set();
        }
        msg_type = MSG_SWITCH_VIEW;
        return retval;
    }
    if (tmtype&kOneMinute) {
        if (data_idx_<0 && sock_clnt_) {
            data_idx_ = sock_clnt_->Start(clnt_inf_.name, clnt_inf_.hostname, 
                clnt_inf_.phy_prtcl, clnt_inf_.app_prtcl, clnt_inf_.recnct_intvl) - 1;
        }
    }
}

//-------------------------------------------------------------------------
void ViewPqm::dis_treat()
{
    if (!msg_type) return;
    view_frame();
    switch (view_num) {
        case VW_MAIN:
            view_main();
            break;
        case VW_HARM:
            view_harm();
            break;
        case VW_OTHER:
            view_other();
            break;
        case VW_SET:
            view_set();
            break;
    }
}

//-------------------------------------------------------------------------
void ViewPqm::refresh()
{
    int rfrsh;

    rfrsh = main_frame->refresh();

    if (!msg_type) return;
    switch (view_num) {
        case VW_MAIN:
            main_form->refresh(rfrsh);
            break;
        case VW_HARM:
            harm_form->refresh(rfrsh);
            break;
        case VW_OTHER:
            other_form->refresh(rfrsh);
            break;
        case VW_SET:
            set_form->refresh(rfrsh);
            break;
        default:
            break;
    }
    if (rfrsh == 1) {
        dis_buf_->refresh();
    }
}

//-------------------------------------------------------------------------
void ViewPqm::view_main_key(int ktype)
{
    static int pcnt = 0;
    static char key_buf[12];
    static int save_cnt = 0;
    unsigned long li;

    if (data_buffer().manual_state()==1) { //press any key to cancel manual record wave
        msg_type = MSG_SWITCH_VIEW;
        messageq_gui().PushCtrlSig(data_idx_, 0, 0);
        return;
    }
    if (data_buffer().save_cyc10()) {
        sys_prompt_valid_ = kSaveRec10End;
        messageq_gui().PushCtrlSig(data_idx_, 1, 0);
    }
    exit_flag = 0;
    if (ktype > 47 && ktype < 59) {
        key_buf[pcnt] = ktype;
        pcnt++;
        key_buf[pcnt] = 0;

        if (data_buffer().sys_para()->manual_rec_enable()&&!main_form->view_num()) {
            if (pcnt >= 3) {
                sscanf(key_buf, "%li", &li);
                if (li == 557) {
                    messageq_gui().PushCtrlSig(data_idx_, 0, 1);
                    /*
                    if (volt_variation->ManualTrigger(1)<0) {
                        sys_prompt_valid_ = kManualRecWvF;
                    }
                    msg_type = MSG_SWITCH_VIEW;
                    */
                    pcnt = 0;
                } else if (li == 114) {
                    messageq_gui().PushCtrlSig(data_idx_, 1, 1);
                    pcnt = 0;
                    sys_prompt_valid_ = kSaveRec10Str;
                    msg_type = MSG_SWITCH_VIEW;
                }
            }
        }

        if (pcnt >= 5) { //�����˳��봦��
            sscanf(key_buf, "%li", &li);
            if (li == 26755) {
                exit_flag = 0xff;
            }
            pcnt = 0;
        }
    } else {
        pcnt = 0;
    }

    switch (ktype) {
        case KEY_PHS:
            power_phs_++;
            power_phs_ %= 3;
            msg_type = MSG_SWITCH_PHS;
            break;
        case KEY_SET:
            if (set_form->Locked()) {
                sys_prompt_valid_ = kSettingLocked;
                break;
            }
            set_kb_leds(0, 1);//NumLock on
            view_num = VW_SET; // 'SET' key
            tstcnt = 0;
            set_form->ini_para();
            msg_type = MSG_SWITCH_VIEW;
            break;
        case KEY_OTH:
            view_num = VW_OTHER; // 'OTH' key
            tstcnt = 0;
            msg_type = MSG_SWITCH_VIEW;
            break;
        case KEY_HMS:
            view_num = VW_HARM; // 'HMS' key
            tstcnt = 0;
            msg_type = MSG_SWITCH_VIEW;
            break;
        case KEY_dot:   // Whether 'dot' key
            tstcnt ++;
            break;
        case KEY_START: //���������
            msg_type = MSG_SWITCH_VIEW;
            break;
        case KEY_9: //�л�����ǯ���
            if (data_buffer().equip_para()->current_clamp_en() & 0x1) {
                msg_type = MSG_SWITCH_CCLAMP_RATIO;
            }
            break;
        case KEY_8:
            main_form->switch_view_num(-1);
            msg_type = MSG_SWITCH_VIEW2;
            break;
        case KEY_2:
            main_form->switch_view_num(1);
            msg_type = MSG_SWITCH_VIEW2;
            break;
        default:
            tstcnt = 0;
            break;
    }

    if (!tstcnt) {
        if (debug_show) { //������ʾ������Ϣ
            debug_show = 0;
            main_form->label_debug1->set_visible(false);
            main_form->label_debug2->set_visible(false);
        }

    } else if (tstcnt > 2) {
        if (!debug_show) { //δ��ʾ������Ϣ
            debug_show = 100;
            main_form->label_debug1->set_visible(true);
            main_form->label_debug2->set_visible(true);
        }
    }
}

//-------------------------------------------------------------------------
void ViewPqm::view_harm_key(int ktype)
{
    int hmvnum = harm_form->view_num;
    int nums = harm_form->hmnum_per_page;
    int tl = harm_form->view_totl - 1;
    switch (ktype) {
        case KEY_4:
            if (hmvnum) { //Ƶ�׽���
                harm_nums--;
            } else {
                harm_nums -= nums;
            }
            if (harm_nums < 1) harm_nums = SHOW_MAX_HARM_NUM;
            msg_type = MSG_CHG_HARMNUMS;
            break;
        case KEY_6:
            if (hmvnum) { //Ƶ�׽���
                harm_nums++;
            } else {
                harm_nums += nums;
            }
            if (harm_nums > SHOW_MAX_HARM_NUM) harm_nums = 1;
            msg_type = MSG_CHG_HARMNUMS;
            break;
        case KEY_PHS:
            power_phs_++;
            power_phs_ %= 3;
            msg_type = MSG_SWITCH_PHS;
            break;
        case KEY_8:
            hmvnum--;
            if (hmvnum < 0) hmvnum = tl;
            msg_type = MSG_SWITCH_VIEW2;
            break;
        case KEY_2:
            hmvnum++;
            if (hmvnum > tl) hmvnum = 0;
            msg_type = MSG_SWITCH_VIEW2;
            break;
        case KEY_ESC:
            view_num = VW_MAIN; // 'esc' key
            msg_type = MSG_SWITCH_VIEW;
            break;
        case KEY_OTH:
            view_num = VW_OTHER; // 'OTH' key
            msg_type = MSG_SWITCH_VIEW;
            break;
        default:
            break;
    }
    harm_form->view_num = hmvnum;
}

//-------------------------------------------------------------------------
void ViewPqm::view_other_key(int ktype)
{
    int ovnum, ovnum2, tl, tl2;
    other_form->get_view_num(ovnum, ovnum2);
    other_form->get_view_totl(tl, tl2);
    switch (ktype) {
        case KEY_ESC:
            view_num = VW_MAIN; /* 'esc' key */
            debug_show = 0;
            tstcnt = 0;
            msg_type = MSG_SWITCH_VIEW;
            ovnum = 0;
            break;
        case KEY_dot:   // Whether 'dot' key
            tstcnt ++;
            if (tstcnt > 2) {
                debug_show = 100;
            }
            break;
        case KEY_8:
            ovnum--;
            if (ovnum < 0) ovnum = tl - 1;
            ovnum2 = 0;
            msg_type = MSG_SWITCH_VIEW2;
            break;
        case KEY_2:
            ovnum++;
            if (ovnum >= tl) ovnum = 0;
            msg_type = MSG_SWITCH_VIEW2;
            ovnum2 = 0;
            break;
        case KEY_HMS:
            view_num = VW_HARM;
            msg_type = MSG_SWITCH_VIEW;
            ovnum = 0;
            break;
        case KEY_4:
            ovnum2--;
            if (ovnum2 < 0) ovnum2 = tl2 - 1;
            msg_type = MSG_SWITCH_VIEW2;
            break;
        case KEY_6:
            ovnum2++;
            if (ovnum2 > tl2) ovnum2 = 0;
            msg_type = MSG_SWITCH_VIEW2;
            break;
        default:
            tstcnt = 0;
            debug_show = 0;
            break;
    }
    other_form->set_view_num(ovnum, ovnum2);
}

//-------------------------------------------------------------------------
//vc=0,voltage; =1,current
void ViewPqm::show_imbalance(int vc)
{
    const char *stru[3] = {"��ƽ��", "����", "����"};
    const char *strvc[2] = {"��ѹ", "����"};
    const char unit[2] = {'V', 'A'};
    char stri[48];
    float d[3], d2[3];
    int i, j, k;
    d[0] = data_buffer().GetUnbalance(vc, 3);    //unbalance
    d[1] = data_buffer().GetUnbalance(vc, 2);    //negtive
    d[2] = data_buffer().GetUnbalance(vc, 0);    //zero

    for (j = 0; j < 3; j++) {
        if (vc) k=3; else k=2;
        i = float_fmt(d[j], 5, k);
        if (j > 0) {
            //sprintf(stri, "%s:%6.*f%c",stru[j],i,d[j],unit[vc]);
            sprintf(stri, "%6.*f%c", i, d[j], unit[vc]);
        } else {
            if (d[j] > 9999) {
                //sprintf(stri, "%s%s: >9999%%",strvc[vc],stru[j]);
                sprintf(stri, ">9999%%");
            } else {
                //sprintf(stri, "%s%s:%6.*f%%",strvc[vc],stru[j],i,d[j]);
                sprintf(stri, "%6.*f%%", i, d[j]);
            }
        }
        if (vc) {
            other_form->labelset_imbalancei->set_txt(2 * j + 1, stri);
        } else {
            other_form->labelset_imbalanceu->set_txt(2 * j + 1, stri);
        }
    }
}

//-------------------------------------------------------------------------
// ��ʾ��г��������
void ViewPqm::show_thd()
{
    int i, j;
    char stri[48], chi;
    float fi;

    chi = 'a';
    for (i = 0; i < 3; i++) {
        fi = data_buffer().GetHarmonic(kHarmTHD, i, 0);
        j = float_fmt(fi, 4, 2);
        sprintf(stri, "U%c:%4.*f%%", chi + i, j, fi);
        harm_form->labelset_thd->set_txt(2 * i, stri);

        fi = data_buffer().GetHarmonic(kHarmTHD, i, 1);
        j = float_fmt(fi, 4, 2);
        sprintf(stri, "I%c:%4.*f%%", chi + i, j, fi);
        harm_form->labelset_thd->set_txt(2 * i + 1, stri);
    }

}

//-------------------------------------------------------------------------
// ��ʾ���ʲ���
void ViewPqm::show_power_param()
{
    char stri[48];
    int x, y, i, hms;
    float fi;

    // Display �й�����.
    fi = data_buffer().GetPower(kPowerP, power_phs_);
    if (fi < 1000) {
        harm_form->labelset_power->set_txt(2, "P(kW):");
    } else {
        harm_form->labelset_power->set_txt(2, "P(MW):");
        fi /= 1000;
    }
    i = float_fmt(fi, 5, 3);
    sprintf(stri, "%6.*f", i, fi);
    harm_form->labelset_power->set_txt(3, stri);

    // Display �޹�����.
    fi = data_buffer().GetPower(kPowerQ, power_phs_);
    if (fi < 1000) {
        harm_form->labelset_power->set_txt(4, "Q(kVar):");
    } else {
        harm_form->labelset_power->set_txt(4, "Q(MVar):");
        fi /= 1000;
    }
    i = float_fmt(fi, 5, 3);
    sprintf(stri, "%6.*f", i, fi);
    harm_form->labelset_power->set_txt(5, stri);

    // Display ���ڹ���.
    fi = data_buffer().GetPower(kPowerS, power_phs_);
    if (fi < 1000) {
        harm_form->labelset_power->set_txt(6, "S(kVA):");
    } else {
        harm_form->labelset_power->set_txt(6, "S(MVA):");
        fi /= 1000;
    }
    i = float_fmt(fi, 5, 3);
    sprintf(stri, "%6.*f", i, fi);
    harm_form->labelset_power->set_txt(7, stri);

    // Display PF.
    fi = data_buffer().GetPower(kPowerPF, power_phs_);
    i = float_fmt(fi, 5, 4);
    sprintf(stri, "%5.*f", i, fi);
    harm_form->labelset_power->set_txt(9, stri);
    // Display DPF.
    fi = data_buffer().GetPower(kHrmPowerPF, power_phs_);
    i = float_fmt(fi, 5, 4);
    sprintf(stri, "%5.*f", i, fi);
    harm_form->labelset_power->set_txt(11, stri);
}

/*!
Description:show the wave of real voltage or current.

    Input:  type -- 0=voltage; 1=current
            st -- start position of real data (�����);
*/
void ViewPqm::show_wave(int type, int st)
{
    static const int buf_num = 320;
    int i, m, num, disnum;
    float fi;
    short *psi;
    float buf[buf_num + 2];
    CChart * pchart;
    static unsigned int order = 0;

    if (!type) { //��ѹ
        pchart = main_form->chart_u;
        fi = data_buffer().GetRatio(1); //PT2
        fi = fi / data_buffer().equip_para()->sample_pt();
        if (!data_buffer().sys_para()->connect_t()) { //Y�ͽ���
            fi *= 0.57737;
        }
        pchart->set_y_scale((int)fi); //�趨Y�ᵥλ�̶ȶ�Ӧ����ֵ
    } else {
        pchart = main_form->chart_i;
        fi = data_buffer().GetRatio(3); //CT2;
        fi = fi / data_buffer().equip_para()->sample_pt();
        pchart->set_y_scale((int)(fi * iwav_scale / full_scale)); //�趨Y�ᵥλ�̶ȶ�Ӧ����ֵ
    }
    disnum = pchart->get_dotnum();
    if (disnum > buf_num) disnum = buf_num;
    psi = data_buffer().waveform(power_phs_ + 3 * type, &num); //pqmfunc->get_real_pt(power_phs_ + 3 * type, num);
    if (num == 0) num = 1;
    buf[0] = psi[st];

	if (dis_buf_->disbufw()==480) {  //480*272
        for (i = 1; i <= disnum / 2; i++) {
            m = st + i;
            if (m < num) {
                buf[2 * i] = psi[m];
            } else {
                buf[2 * i] = psi[m - num + 26]; //���ܲ�25.6�㣬ȡ26
            }
            buf[2 * i - 1] = (buf[2 * i - 2] + buf[2 * i]) / 2;
        }     
    } else {    //320*240
        for (i = 1; i < disnum; i++) {
            m = st + i;
            if (m < num) {
                buf[i] = psi[m];
            } else {
                buf[i] = psi[m - num + 26]; //���ܲ�25.6�㣬ȡ26
            }
        }
    }
    pchart->ClearData();
    pchart->add_dataset(0, buf, disnum, phs_color[power_phs_]);
}
//-------------------------------------------------------------------------

void ViewPqm::view_set_key(int ktype)
{
    msg_type = MSG_PRESS_KEY;
    if (set_form->view_set_key(ktype)) { //�˳����������
        view_num = VW_MAIN;
        msg_type = MSG_SWITCH_VIEW;
        main_frame->tabctrl_view->enable_tab(true);
    }
}
