#include<cmath>
#include<cstdio>
#include<cstdlib>
#include<cstring>

#include "harmfunc.h"
#include "../thread/pthread_mng.h"
#include "../device/device.h"
//#include "prmconfig.h"

using namespace std;

CHarmBase::CHarmBase(LinePar* par)
{
    line_para_ = prmcfg->line_para();
    calc_user_cur();
    syspara_ = prmcfg->syspara();
    set_harm_limit();
    memset(units_, 0, sizeof(units_));
}

//Description:  设置谐波限值
void CHarmBase::set_harm_limit()
{
    if (syspara_->limit_type) {
        harm_u_limit_[0] = syspara_->harm_ulimit[0];
        harm_u_limit_[0] /= 10;
        for (int i = 0; i < 24; i++) {
            harm_u_limit_[i + 1] = syspara_->harm_ulimit[i + 1];
            harm_u_limit_[i + 1] /= 10;
            harm_i_limit_[i] = syspara_->harm_ilimit[i];
            harm_i_limit_[i] /= 10;
        }
    } else {  //超限类型为国标
        harm_u_limit_[0] = HamULimit[line_para_->CUlevel][0];
        for (int i = 0; i < 12; i++) {
            harm_u_limit_[i * 2 + 1] = HamULimit[line_para_->CUlevel][1];
            harm_u_limit_[i * 2 + 2] = HamULimit[line_para_->CUlevel][2];
            harm_i_limit_[i * 2] = HarmILimit[line_para_->CUlevel][i * 2];
            harm_i_limit_[i * 2 + 1] = HarmILimit[line_para_->CUlevel][i * 2 + 1];
        }
    }
    for (int i = 24; i < 49; i++) {
        harm_u_limit_[i + 1] = syspara_->harm_ulimit[i + 1];
        harm_u_limit_[i + 1] /= 10;
        harm_i_limit_[i] = syspara_->harm_ilimit[i];
        harm_i_limit_[i] /= 10;
    }
    //等等 shmem_func().set_harm_limit();
}

/*//Input amplitude of hamonic voltage&current from DARAM.
void CHarmBase::write_hamp(unsigned short*hu, unsigned short*hi)
{
    int i, j;
    unsigned long ul;

    for(i = 0; i < 3; i++) {
        //0次谐波数据除以10
        ul = unzip_int(*hu);
        *hu = zip_int(ul / 10);
        ul = unzip_int(*hi);
        *hi = zip_int(ul / 10);

        for(j = 0; j <= 25; j++) {
            ul = unzip_int(*hu);
            harmrms_[0].array[i][j] = ul;

            ul = unzip_int(*hi);
            harmrms_[1].array[i][j] = ul;
            hu ++;
            hi ++;
        }
        ul = harmrms_[0].array[i][1];
        if (ul<55774) {  // 55.774V
            int n = 55774-ul;
            if (n>50000) n = 50000;
            line_coef_[i] = n*prmcfg->vlinecoef(i)/50000;
        } else line_coef_[i] = 0;
        harmrms_[0].array[i][1] + line_coef_[i];
    }
}*/

//Input amplitude of 26~50hamonic voltage&current from DARAM.
void CHarmBase::write_exthamp(unsigned short*hu, unsigned short*hi,
                              unsigned short valid)
{
    int i, j;
    unsigned long ul;

    if(valid & 0x1) { //26~50次谐波有效
        for(i = 0; i < 3; i++) {
            for(j = 26; j <= MAX_HARM_NUM; j++) {
                ul = unzip_int(*hu);
                harmrms_[0].array[i][j] = ul;

                ul = unzip_int(*hi);
                harmrms_[1].array[i][j] = ul;
                hu ++;
                hi ++;
            }
        }
    } else {
        for(i = 0; i < 3; i++) {
            memset(&harmrms_[0].array[i][26], 0, (MAX_HARM_NUM - 25)*sizeof(long));
            memset(&harmrms_[1].array[i][26], 0, (MAX_HARM_NUM - 25)*sizeof(long));
        }
    }
}

//-------------------------------------------------------------------------
//Description:  计算谐波含有率
void CHarmBase::calc_hru_i()
{
    int i, j;
    long l1, l2;
    float fi;

    // Calc HRU and HRI.
    for(j = 0; j < 3; j++) {
        l1 = harmrms_[0].array[j][1];
        if(!l1) l1 = 50000;
        l2 = harmrms_[1].array[j][1];
        if(!l2) l2 = 50000;
        for(i = 0; i <= MAX_HARM_NUM; i++) {
            fi = harmrms_[0].array[j][i];
            HRui_[0].array[j][i] = fi*100 / l1;
            fi = harmrms_[1].array[j][i];
            HRui_[1].array[j][i] = fi*100 / l2;
            fi = harm_in_[0].array[j][i];
            HRU_in_.array[j][i] = fi*100 / l1;
        }
    }
}

//Description:  计算总谐波畸变率
void CHarmBase::calc_thd()
{
    int i, j, k;

    memset(THD_, 0, sizeof(THD_));
    // Calc THDu and THDi.
    for(k = 0; k < 2; k++) {
        for(j = 0; j < 3; j++) {
            for(i = 2; i <= MAX_HARM_NUM; i++) {
                if (i&1) THD_[k][1][j] +=  HRui_[k].array[j][i] * HRui_[k].array[j][i];
                else  THD_[k][2][j] +=  HRui_[k].array[j][i] * HRui_[k].array[j][i];
                if (THD_[1][1][j]||THD_[1][2][j]) {
                }
            }
            THD_[k][0][j] = sqrt(THD_[k][1][j]+THD_[k][2][j]);
            THD_[k][1][j] = sqrt(THD_[k][1][j]);
            THD_[k][2][j] = sqrt(THD_[k][2][j]);
        }
    }
}

//-------------------------------------------------------------------------
// calc 3 phase 有效值. voltage(vc=0), current(vc=1)
float CHarmBase::CalcUivalid(int phase, int vc)
{
    int i;
    double fi, fj;
    unsigned long *pli;

    fj = 0;
    if(vc) {
        pli = &harmrms_[1].array[0][0] + (MAX_HARM_NUM + 1) * phase + 1;
    } else {
        pli = &harmrms_[0].array[0][0] + (MAX_HARM_NUM + 1) * phase + 1;
    }

    for(i = 0; i < MAX_HARM_NUM; i++) {
        fi = *pli;
        fi /= units_[vc];
        fj += fi * fi; //此处一定要先除后乘，否则可能溢出
        pli ++;
    }
    return sqrt(fj);
}


//-------------------------------------------------------------------------
//计算用户第h次谐波电流允许值
void CHarmBase::calc_user_cur()
{
    double d1, d2;

    d1 = line_para_->Short_cap;
    d1 /= Short_cap_datum[line_para_->CUlevel];

    d2 = line_para_->User_cap;
    d2 /= line_para_->Supp_cap;

    usr_c_cp.user_cur3 = d1 * pow(d2, 0.9091); // 1/1.1
    usr_c_cp.user_cur5 = d1 * pow(d2, 0.8333); // 1/1.2
    usr_c_cp.user_cur7 = d1 * pow(d2, 0.7143); // 1/1.4
    usr_c_cp.user_cur11 = d1 * pow(d2, 0.5556); // 1/1.8
    usr_c_cp.user_cur13 = d1 * pow(d2, 0.5263); // 1/1.9
    usr_c_cp.user_cur_oth = d1 * sqrt(d2); // 1/2
}

/*!
Description:获取谐波数据

    Input:      type -- 数据类型. HARM_AMP=幅值; HARM_AMP2=2次侧幅值; HARM_HR=谐波含有率;
                    HARM_THD=总畸变率; 
                phs -- 相序.
                vc -- 0=voltage; 1=current
                nums-谐波次数.
    Return:     获取的谐波数值
*/
float CHarmBase::get_harm_data(int type, int phs, int vc, int nums)
{
    float fi = 0;

    if(phs > 2 || nums > MAX_HARM_NUM) return fi;

    switch(type) {
        case HARM_AMP:
            if(vc) {
                fi = harmrms_[1].array[phs][nums];
                fi *= line_para_->CT1;
                fi /= line_para_->CT2;
            } else {
                fi = harmrms_[0].array[phs][nums];
                fi *= line_para_->PT1;
                fi /= line_para_->PT2;
            }
            fi /= units_[vc];
            break;
        case HARM_AMP2:
            if(vc) {
                fi = harmrms_[1].array[phs][nums];
            } else {
                fi = harmrms_[0].array[phs][nums];
            }
            break;
        case HARM_HR:
            if(vc) {
                fi = HRui_[1].array[phs][nums];
            } else {
                fi = HRui_[0].array[phs][nums];
            }
            break;
        case HARM_THD:
            if(vc) {
                fi = THD_[1][0][phs];
            } else {
                fi = THD_[0][0][phs];
            }
            break;
        default:
            break;
    }
    return fi;
}

//-------------------------------------------------------------------------
//计算谐波电流允许值
//nums,谐波次数.
float CHarmBase::hrmi_limit(int nums)
{
    float d;

    d = harm_i_limit_[nums - 2];
    switch(nums) {
        case 3:
            d *= usr_c_cp.user_cur3;
            break;
        case 5:
            d *= usr_c_cp.user_cur5;
            break;
        case 7:
            d *= usr_c_cp.user_cur7;
            break;
        case 11:
            d *= usr_c_cp.user_cur11;
            break;
        case 13:
            d *= usr_c_cp.user_cur13;
            break;
        default:
            d *= usr_c_cp.user_cur_oth;
            break;
    }
    return d;
}


/*!
Description:Set unit_[3]

    Input:  addr -- daram start address of stdy_z_buf_->hreserve
*/
void CHarmBase::SetUnits(short *addr)
{
    short ui = addr[0xf8];
    if ((ui & 0xff00) == 0x9500) {
        old_unit_ = false;
        switch ((ui >> 4) & 0xf) {
            case 1:
                units_[0] = 1000;    //mV
                units_type_[0] = 1;
                units_[1] = 10000;   //0.1mA
                units_type_[1] = 2;
                units_[2] = 100;     //0.01Hz
                units_type_[2] = 0;
                break;
            default:
                units_[0] = 1000;    //mV
                units_type_[0] = 1;
                units_[1] = 1000;    //mA
                units_type_[1] = 1;
                units_[2] = 100;     //0.01Hz
                units_type_[2] = 0;
                break;
        }
    } else {
        old_unit_ = true;
        units_[0] = 1000;    //mV
        units_type_[0] = 1;
        units_[1] = 1000;    //mA
        units_type_[1] = 1;
        units_[2] = 100;     //0.01Hz
        units_type_[2] = 0;
    }
}

/*!
Description:Calc voltage deviation.

    input: phase -- 0-2=A-C
*/
float CHarmBase::CalcVoltdv(int phase)
{
    long l1;
    float d1;
    if(harmrms_[0].array[phase][1] <= 3) { //Fundamental voltage<3V
        u_rms_[phase] = 0;
        return 0;
    } else {
        d1 = CalcUivalid(phase, 0);
        u_rms_[phase] = d1;
        l1 = line_para_->PT2; // Read PT secondary
        if(!l1)return 0;

        if (!prmcfg->connect_type()) { //wye circuit
            d1 *= 1.732;
        }
        d1 = (d1 - l1) * 100;
        return d1 / l1;
    }
}


