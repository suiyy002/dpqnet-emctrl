#include<cmath>
#include<cstdio>
#include<cstdlib>
#include<cstring>

#include "harmfunc.h"
#include "../thread/pthread_mng.h"
#include "../device/device.h"
#include "prmconfig.h"
#include "../Version.h"
#include "../EEW/ee_warning.h"
#include "../IPC/shmemfunc.h"
#include "volt_variation.h"

using namespace std;

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//  CHarmFunc
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
CHarmFunc *harmfunc;

extern int HRM_MAX_NUM_CUR;

//-------------------------------------------------------------------------
CHarmFunc::CHarmFunc(LinePar* par): CHarmBase(par)
{
    InitAggregation();
    update_IntBase();

    timeval curtm;
    getlocaltime(&curtm);
    SetRecTime(&curtm);
    harm_quality_ = 0;
}

/*!
Input amplitude of hamonic voltage&current from DARAM.

    Input:  hu -- Harmonic voltage
            hi -- Harmonic current
*/
void CHarmFunc::write_hamp(unsigned short*hu, unsigned short*hi)
{
    int i, j;
    unsigned long ul;
    unsigned short * psi;
    float fi;

    for (int vc = 0; vc < 2; vc++) {
        if (!vc) psi = hu;
        else psi = hi;

        for (i = 0; i < 3; i++) {
            //-------------- 直流 -----------------------------
            ul = unzip_int(*psi);
            harmrms_[vc].array[i][0] = ul;
            harm_avgbuf_[vc].array[i][0] += ul; //注意直流分量的平均值与交流分量的不同

            //-------------- 基波 -----------------------------
            psi++;
            ul = unzip_int(*psi);
            if (vc == 0) {
                if (ul < 45774) { // 45.774V
                    int n = 45774 - ul;
                    if (n > 40000) n = 40000;
                    line_coef_[i] = n * prmcfg->vlinecoef(i) / 40000;
                    //printf("prmcfg->vlinecoef(%d)=%d line_coef=%d\n", i, prmcfg->vlinecoef(i), line_coef_[i]);
                } else line_coef_[i] = 0;
                ul += line_coef_[i];
                *psi = zip_int(ul);
            }
            harmrms_[vc].array[i][1] = ul;
            fi = ul;
            harm_avgbuf_[vc].array[i][1] += fi * fi;//由long改为float，否则，当>65535就会溢出。2008-4-1 seapex

            if (ul > harm_aggr_[0][vc].array[i][1]) {    //Maximum
                harm_aggr_[0][vc].array[i][1] = ul;
            } else if (ul < harm_aggr_[2][vc].array[i][1]) { //Minimum
                harm_aggr_[2][vc].array[i][1] = ul;
            }

            //-------------- 谐波 -----------------------------
            psi++;
            for (j = 2; j <= 25; j++) {
                ul = unzip_int(*psi);
                if (!vc && prmcfg->cvt_modify_en()) {
                    ul = ul * prmcfg->cvt_modify(j - 2) / 1000;
                }
                harmrms_[vc].array[i][j] = ul;
                harm_avgbuf_[vc].array[i][j] += ul * ul;

                if (ul > harm_aggr_[0][vc].array[i][j]) {    //Maximum
                    harm_aggr_[0][vc].array[i][j] = ul;
                } else if (ul < harm_aggr_[2][vc].array[i][j]) { //Minimum
                    harm_aggr_[2][vc].array[i][j] = ul;
                }
                psi++;
            }
        }
    }
}

//Description: Input amplitude of 26~50hamonic voltage&current from DARAM.
//Input: hu,电压谐波; hi,电流谐波; valid,扩展谐波是否有效
void CHarmFunc::write_exthamp(unsigned short*hu, unsigned short*hi,
                              unsigned short valid)
{
    int i, j;
    unsigned long ul;
    static unsigned int icnt = 0;

    if (valid & 0x1) { //26~50次谐波有效
        for (i = 0; i < 3; i++) {
            for (j = 26; j <= MAX_HARM_NUM; j++) {
                ul = unzip_int(*hu);
                if (prmcfg->cvt_modify_en()) {
                    ul = ul * prmcfg->cvt_modify(j - 2) / 1000;
                }
                harmrms_[0].array[i][j] = ul;
                harm_avgbuf_[0].array[i][j] += ul * ul;

                ul = unzip_int(*hi);
                harmrms_[1].array[i][j] = ul;
                if (ul > harm_aggr_[0][1].array[i][j]) {
                    harm_aggr_[0][1].array[i][j] = ul;
                } else if (ul < harm_aggr_[2][1].array[i][j]) {
                    harm_aggr_[2][1].array[i][j] = ul;
                }
                harm_avgbuf_[1].array[i][j] += ul * ul;
                hu ++;
                hi ++;
            }
        }
    } else {
        for (i = 0; i < 3; i++) {
            memset(&harmrms_[0].array[i][26], 0, (MAX_HARM_NUM - 25)*sizeof(long));
            memset(&harmrms_[1].array[i][26], 0, (MAX_HARM_NUM - 25)*sizeof(long));
        }
    }

    icnt++;
}

//-------------------------------------------------------------------------
//Input phase of hamonic voltage&current.
void CHarmFunc::write_hphs(unsigned short*hu, unsigned short*hi)
{
    unsigned short *pusi;
    int i, j;
    char stri[30];

    j = 26;
    if ((prmcfg->sys_para_sg()->current_clamp_enable & 1) && (prmcfg->current_clamp_type() == 2)) { //CPV051, 调整0.4度
        int k = 0; //j=26;
        for (i = 0; i < 3; i++) {
            if (lack_phs(1, i)) continue;
            pusi = hi + i * j;
            for (k = 0; k < j; k++) {
                *pusi  += 4;
                pusi ++;
            }
        }
    }
    for (i = 0; i < 3; i++) {
        pusi = &harmphs_[0].array[i][0];
        memcpy(pusi, hu, j * sizeof(unsigned short));
        pusi = &harmphs_[1].array[i][0];
        memcpy(pusi, hi, j * sizeof(unsigned short));
        hu += j;
        hi += j;
    }
}

//Input phase of 26~50hamonic voltage&current.
void CHarmFunc::write_exthphs(unsigned short*hu, unsigned short*hi,
                              unsigned short valid)
{
    unsigned short *pusi;
    int i, j;

    if (valid & 0x1) { //26~50次谐波有效
        j = 25;
        if ((prmcfg->sys_para_sg()->current_clamp_enable & 1) && (prmcfg->current_clamp_type() == 2)) { //CPV051, 调整0.4度
            int k; //j=25
            for (i = 0; i < 3; i++) {
                if (lack_phs(1, i)) continue;
                pusi = hi + i * j;
                for (k = 0; k < j; k++) {
                    *pusi  += 4;
                    pusi ++;
                }
            }
        }
        for (i = 0; i < 3; i++) {
            pusi = &harmphs_[0].array[i][26];
            memcpy(pusi, hu, j * sizeof(unsigned short));
            pusi = &harmphs_[1].array[i][26];
            memcpy(pusi, hi, j * sizeof(unsigned short));
            hu += j;
            hi += j;
        }

    }
}

#define JudgeInhmType \
            if ((*hui > *pmx) && (j < HRM_MAX_NUM_CUR + 1)) {\
                *pmx = *hui;  \
                *(pmx + 1) = *(hui + 1); \
            }   \
            if (prmcfg->inharm_type()) {\
                li = *hui;\
                lj += li*li;\
            }\
            hui += 2;\
            pmx += 2;

//-------------------------------------------------------------------------
//写入间谐波数据
void CHarmFunc::write_interhamp(unsigned short *hui)
{
    int i, j, k;
    unsigned short *pmx;
    pmx = &interhuimx[0][0][0];
    long li, lj;

    for (i = 0; i < 6; i++) {
        for (j = 0; j < 51; j++) {
            lj = 0;
            JudgeInhmType
            for (k = 1; k < (INTERH_MAX_RESO - 2); k++) {
                if ((*hui > *pmx) && (j < HRM_MAX_NUM_CUR + 1)) {
                    *pmx = *hui;  //幅值
                    *(pmx + 1) = *(hui + 1); //相位
                }
                li = *hui;
                lj += li * li;
                hui += 2;
                pmx += 2;
            }
            JudgeInhmType
            harm_aggr_in_[1][i / 3].array[i % 3][j] += lj;
            li = sqrt((float)lj);
            harm_in_[i / 3].array[i % 3][j] = li;
            if (i > 2) { //Current
                if (li > harm_aggr_in_[0][1].array[i - 3][j]) { //Maximum
                    harm_aggr_in_[0][1].array[i - 3][j] = li;
                } else if (li < harm_aggr_in_[2][1].array[i - 3][j]) { //Minimum
                    harm_aggr_in_[2][1].array[i - 3][j] = li;
                }
            }
        }
    }
}

/*!
Description:Calculate phase to phase voltage

    Input:  phv -- phase to neutral voltage of the 3 phases. [0-2]:A B C
    Output: ppv -- phase to phase voltage of the 3 phases. [0-2]:ab bc ca
*/
void CHarmFunc::CalcRmsppv(float* ppv, float*phv)
{
    if (prmcfg->connect_type()) {    //delta circuit
        memcpy(ppv, phv, sizeof(float) * 3);
        return;
    }

    Vector vec1, vec2;
    for (int i = 0; i < 3; i++) {
        vec1.amp = phv[i];
        vec1.phs = harmphs_[0].array[i][1];
        int j = i < 2 ? i + 1 : 0;
        vec2.amp = phv[j];
        vec2.phs = harmphs_[0].array[j][1];

        vec1.phs /= 10;
        vec2.phs /= 10;
        ppv[i] = vect_sub_amp(vec1, vec2);
    }
}

/*!
Description:zip harmonic data record that will be saved

    Input: type -- 0=Maximum, 1=Average
*/
void CHarmFunc::zip_harm_rcd(int type)
{
    int i, j;

    if (type) { //Average
        for (i = 0; i < 3; i++) {
            for (j = 0; j <= MAX_HARM_NUM; j++) {
                harmzip_[0].array[i][j] = zip_int( harm_aggr_[1][0].array[i][j] );
                harmzip_[1].array[i][j] = zip_int( harm_aggr_[1][1].array[i][j] );
            }
        }
    } else {    //Maximum
        for (i = 0; i < 3; i++) {
            //Note:dc & fundamental harmonic be saved is not max, but avg
            harmzip_[0].array[i][0] = zip_int( harm_aggr_[1][0].array[i][0] );
            harmzip_[1].array[i][0] = zip_int( harm_aggr_[1][1].array[i][0] );
            harmzip_[0].array[i][1] = zip_int( harm_aggr_[1][0].array[i][1] );
            harmzip_[1].array[i][1] = zip_int( harm_aggr_[1][1].array[i][1] );
            for (j = 2; j <= MAX_HARM_NUM; j++) {
                harmzip_[0].array[i][j] = zip_int( harm_aggr_[0][0].array[i][j] );
                harmzip_[1].array[i][j] = zip_int( harm_aggr_[0][1].array[i][j] );
            }
        }
    }


}

/*!
Description: Calculate derived data.

    Called by:  harm_hdl
*/
void CHarmFunc::CalcDerivedData()
{
    calc_hru_i(); //计算谐波含有率
    calc_thd(); //计算总谐波畸变率
    stat_hr_thd(); //统计最大值及累加平方和

    for (int i = 0; i < 3; i++) { //顺便计算一下电流有效值and voltage deviation
        i_rms_[i] = CalcUivalid(i, 1);  //Calculate current rms
        u_deviation_[i] = CalcVoltdv(i);  //Calculate voltage deviation
    }
    CalcRmsppv(u_rms_ppv_, u_rms_); //Calculate phase to phase voltage
    calc_hpower();
    calc_glpower();
    CalcUnbalance();
    CalcTrnsfmrLoss(); //calculate transformer loss

    hrm_refresh_cnt_++;

}

/*!
Description:Inspect if have alarm about harmonic happened.

    Output: alarm_word -- 超限指示字. refer to alarm_word_
            alm_ary -- refer to alm_array_
    Return: 谐波超限的个数
*/
int CHarmFunc::AlarmCheck(unsigned int&alarm_word, unsigned short *alm_ary)
{
    int i, j, k, n;
    float d, d1;
    unsigned long *phiam;
    unsigned short *array = alm_array_;

    if (alm_ary) array = alm_ary;
    else array = alm_array_;

    // 检查电压总谐波畸变率是否超限.
    float *pthd = THD_[0][0];
    n = 0;
    d = harm_u_limit_[0];
    for(j = 0; j < 3; j++) {
        if((pthd[j] * 100) > d) {
            array[n] = 2 * j << 8;
            array[n] |= 1;
            n++;
            alarm_word |= APH_THDu << j;
        } else {
            alarm_word &= ~(APH_THDu << j);
        }
    }
    // 检查电压谐波含有率是否超限.
    float *phru = &HRui_[0].array[0][2];
    for(i = 2; i <= 50; i++) {
        k = (i + 1) % 2;
        d = harm_u_limit_[k + 1];
        for(j = 0; j < 3; j++) {
            d1 = *(phru + (MAX_HARM_NUM + 1) * j);
            if(d1 >= d) {
                array[n] = 2 * j << 8;
                array[n] |= i;
                n++;
                alarm_word |= APH_HRU << j;
            } else {
                alarm_word &= ~(APH_HRU << j);
            }
        }
        phru ++;
    }

    // 检查谐波电流是否超限.
    for(i = 2; i <= 50; i++) {
        phiam = &harmrms_[1].array[0][i] ;
        d = hrmi_limit(i);
        for(j = 0; j < 3; j++) {
            d1 = *(phiam + (MAX_HARM_NUM + 1) * j);
            d1 = d1 * line_para_->CT1 / line_para_->CT2 / units_[1];
            if(d1 > d) {
                array[n] = (2 * j + 1) << 8;
                array[n] |= i;
                n++;
                alarm_word |= APH_HI << j;
            } else {
                alarm_word &= ~(APH_HI << j);
            }
        }
    }
    shmem_func().SetAlm(kAlmTHDu, array, n, refresh_time_);
    return n;
}

void CHarmFunc::GetShmReal(PqmMmxu *mmxu, PqmMhai *mhai, PqmMhaiIntr *mhai_in)
{
    int i, j;
    float fi;

    memcpy(mhai->thda, THD_[1][0], sizeof(float) * 3);
    memcpy(mhai->thdodda, THD_[1][1], sizeof(float) * 3);
    memcpy(mhai->thdevna, THD_[1][2], sizeof(float) * 3);
    if (prmcfg->connect_type()) {   //delta circuit
        memcpy(mhai->thdppv, THD_[0][0], sizeof(float) * 3);
        memcpy(mhai->thdoddppv, THD_[0][1], sizeof(float) * 3);
        memcpy(mhai->thdevnppv, THD_[0][2], sizeof(float) * 3);
        for (i = 0; i < 3; i++) {
            for (j = 0; j < 50; j++) {
                fi = harmrms_[0].array[i][j + 1];
                mhai->hppv[i][j] = fi * line_para_->PT1 / line_para_->PT2 / units_[0];
                fi = harm_in_[0].array[i][j + 1];
                mhai_in->hppv[i][j] = fi * line_para_->PT1 / line_para_->PT2 / units_[0];
            }
            memcpy(mhai->hrppv[i], &HRui_[0].array[i][2], sizeof(float) * 49);
            memcpy(mhai_in->hrppv[i], &HRU_in_.array[i][0], sizeof(float) * 50);
        }
    }
    if (1) { //!prmcfg->connect_type()){   //wye circuit
        memcpy(mhai->thdphv, THD_[0][0], sizeof(float) * 3);
        memcpy(mhai->thdoddphv, THD_[0][1], sizeof(float) * 3);
        memcpy(mhai->thdevnphv, THD_[0][2], sizeof(float) * 3);
        for (i = 0; i < 3; i++) {
            for (j = 0; j < 50; j++) {
                fi = harmrms_[0].array[i][j + 1];
                mhai->hphv[i][j] = fi * line_para_->PT1 / line_para_->PT2 / units_[0];
                fi = harm_in_[0].array[i][j + 1];
                mhai_in->hphv[i][j] = fi * line_para_->PT1 / line_para_->PT2 / units_[0];
            }
            memcpy(mhai->hrphv[i], &HRui_[0].array[i][2], sizeof(float) * 49);
            memcpy(mhai_in->hrphv[i], &HRU_in_.array[i][0], sizeof(float) * 50);

        }
    }
    for (i = 0; i < 3; i++) { //A-C
        for (j = 0; j < 50; j++) {
            fi = harmrms_[1].array[i][j + 1];
            mhai->ha[i][j] = fi * line_para_->CT1 / line_para_->CT2 / units_[1];
            fi = harm_in_[1].array[i][j];
            mhai_in->ha[i][j] = fi * line_para_->CT1 / line_para_->CT2 / units_[1];
            mhai->hvang[i][j] = get_phase(i, 0, j + 1, 1);
            mhai->haang[i][j] = get_phase(i, 1, j + 1, 1);
        }
    }
    if (!prmcfg->connect_type()) {   //wye circuit
        for (i = 0; i < 3; i++) { //A-C
            memcpy(mhai->hw[i], &hm_power_[0].array[i][1], sizeof(float) * 50);
            memcpy(mhai->hvar[i], &hm_power_[1].array[i][1], sizeof(float) * 50);
            memcpy(mhai->hva[i], &hm_power_[2].array[i][1], sizeof(float) * 50);
            memcpy(mhai->hpf[i], &hm_power_[3].array[i][1], sizeof(float) * 50);
            mhai->tothw[i] = hpower_[0][i];
            mhai->tothvar[i] = hpower_[1][i];
            mhai->tothva[i] = hpower_[2][i];
            mhai->tothpf[i] = hpower_[3][i];
            mmxu->w[i] = power_[0][i];
            mmxu->var[i] = power_[1][i];
            mmxu->va[i] = power_[2][i];
            mmxu->pf[i] = power_[3][i];
        }
    }
    for (i = 0; i < 50; i++) { //all
        mhai->hw[3][i] = hm_power_[0].array[0][i + 1] + hm_power_[0].array[1][i + 1] + hm_power_[0].array[2][i + 1];
        mhai->hvar[3][i] = hm_power_[1].array[0][i + 1] + hm_power_[1].array[1][i + 1] + hm_power_[1].array[2][i + 1];
        mhai->hva[3][i] = hm_power_[2].array[0][i + 1] + hm_power_[2].array[1][i + 1] + hm_power_[2].array[2][i + 1];
        if (mhai->hva[3][i] > 0) mhai->hpf[3][i] = mhai->hw[3][i] / mhai->hva[3][i];
        else mhai->hpf[3][i] = 0;
    }
    i = 3;  //all
    mhai->tothw[i] = hpower_[0][i];
    mhai->tothvar[i] = hpower_[1][i];
    mhai->tothva[i] = hpower_[2][i];
    mhai->tothpf[i] = hpower_[3][i];
    mmxu->w[i] = power_[0][i];
    mmxu->var[i] = power_[1][i];
    mmxu->va[i] = power_[2][i];
    mmxu->pf[i] = power_[3][i];
}

/*!
Description: 根据基波计算不平衡相关参数
*/
void CHarmFunc::CalcUnbalance()
{
    int i, j;
    float fi, fj, fk;

    for (i = 0; i < 2; i++) {
        for (j = 0; j < 3; j++) {
            unbalance_[i][j] = calc_uiph3(j, i);
        }
        if (!unbalance_[i][1]) {
            unbalance_[i][3] = 0;
        } else {
            unbalance_[i][3] = unbalance_[i][2] * 100 / unbalance_[i][1];
        }
    }
}

/*!
Description: Calculate zero,positive,negative sequence component.

    Input:  type -- 0=zero, 1=positive, 2=negative;
            vc -- 0=voltage, 1=current.
    Return: primary voltage or current. unit:V or A
*/
float CHarmFunc::calc_uiph3(int type, int vc)
{
    if (!type && prmcfg->connect_type()) return 0; //Type is zero-sequence & connecting is vv

    Vector vec1, vec2, vec3;
    float fi;

    vec1.amp = get_harm_data(HARM_AMP, 0, vc, 1);
    vec2.amp = get_harm_data(HARM_AMP, 1, vc, 1);
    vec3.amp = get_harm_data(HARM_AMP, 2, vc, 1);
    if (vc) {
        vec1.phs = harmphs_[1].array[0][1];
    } else {
        vec1.phs = harmphs_[0].array[0][1];
    }
    switch (type + vc * 3) {
        case 0:
            vec2.phs = harmphs_[0].array[1][1];
            vec3.phs = harmphs_[0].array[2][1];
            break;
        case 1:
            vec2.phs = harmphs_[0].array[1][1] + 1200;
            vec3.phs = harmphs_[0].array[2][1] + 2400;
            break;
        case 2:
            vec2.phs = harmphs_[0].array[1][1] + 2400;
            vec3.phs = harmphs_[0].array[2][1] + 1200;
            break;
        case 3:
            vec2.phs = harmphs_[1].array[1][1];
            vec3.phs = harmphs_[1].array[2][1];
            break;
        case 4:
            vec2.phs = harmphs_[1].array[1][1] + 1200;
            vec3.phs = harmphs_[1].array[2][1] + 2400;
            break;
        case 5:
            vec2.phs = harmphs_[1].array[1][1] + 2400;
            vec3.phs = harmphs_[1].array[2][1] + 1200;
        default:
            break;
    }
    vec1.phs /= 10;
    vec2.phs /= 10;
    vec3.phs /= 10;
    fi = vect_sum_three(vec1, vec2, vec3) / 3;
    return fi;
}

/*!
Description:谐波记录聚合前的预处理

    Input:  times -- Number of harm data be refreshed
*/
void CHarmFunc::hrm_prehandle()
{
    int i, j, k;
    float fi, fj;
    int times = hrm_refresh_cnt_;
    if (times <= 0) times = 1;
    for (i = 0; i < 3; i++) {   //Phase A,B,C
        //直流分量
        harm_aggr_[1][0].array[i][0] = harm_avgbuf_[0].array[i][0] / times;
        harm_aggr_[1][1].array[i][0] = harm_avgbuf_[1].array[i][0] / times;
        //基波
        harm_aggr_[1][0].array[i][1] = sqrt(harm_avgbuf_[0].array[i][1] / times);
        harm_aggr_[1][1].array[i][1] = sqrt(harm_avgbuf_[1].array[i][1] / times);

        //谐波及谐波功率平均值
        fj = harm_aggr_[1][0].array[i][1];
        for (j = 0; j <= MAX_HARM_NUM; j++) {
            harm_aggr_[1][0].array[i][j] = sqrt(harm_avgbuf_[0].array[i][j] / times);
            harm_aggr_[1][1].array[i][j] = sqrt(harm_avgbuf_[1].array[i][j] / times);
            fi = harm_aggr_[1][0].array[i][j];
            HRU_aggr_[1].array[i][j] = fi * 100 / fj;
            harm_aggr_in_[1][0].array[i][j] = sqrt((float)harm_aggr_in_[1][0].array[i][j] / times);
            harm_aggr_in_[1][1].array[i][j] = sqrt((float)harm_aggr_in_[1][1].array[i][j] / times);
            fi = harm_aggr_in_[1][0].array[i][j];
            HRU_aggr_in_[1].array[i][j] = fi * 100 / fj;
            hm_power_aggr_[1][0].array[i][j] = hm_power_aggr_[1][0].array[i][j] / times;    //P
            hm_power_aggr_[1][1].array[i][j] = hm_power_aggr_[1][1].array[i][j] / times;    //Q
            hm_power_aggr_[1][2].array[i][j] = hm_power_aggr_[1][2].array[i][j] / times;    //S
            if (hm_power_aggr_[1][2].array[i][j]) {
                hm_power_aggr_[1][3].array[i][j] = hm_power_aggr_[1][0].array[i][j] / hm_power_aggr_[1][2].array[i][j]; //pf
            }
        }

        for (j = 0; j < 2; j++) { //总谐波畸变率平均值. U,I
            for (k = 0; k < 3; k++) { //all,odd,even
                THD_aggr_[1][j][k][i] = sqrt(THD_aggr_[1][j][k][i] / times);
            }
        }
    }
    for (j = 0; j <= MAX_HARM_NUM; j++) {
        hm_pw3_aggr_[1][0][j] = hm_pw3_aggr_[1][0][j] / times;
        hm_pw3_aggr_[1][1][j] = hm_pw3_aggr_[1][1][j] / times;
        hm_pw3_aggr_[1][2][j] = hm_pw3_aggr_[1][2][j] / times;
        if (hm_pw3_aggr_[1][2][j] > 0) hm_pw3_aggr_[1][3][j] = hm_pw3_aggr_[1][0][j] / hm_pw3_aggr_[1][2][j];
    }
    for (i = 0; i < 3; i++) {   //P,Q,S
        for (j = 0; j < 4; j++) { //Phase A-C,all
            power_aggr_[1][i][j] = power_aggr_[1][i][j] / times;
            hpower_aggr_[1][i][j] = hpower_aggr_[1][i][j] / times;
        }
    }
    //PF
    for (i = 0; i < 4; i++) { //A-C,all
        if (power_aggr_[1][2][i] > 0) power_aggr_[1][3][i] = power_aggr_[1][0][i] / power_aggr_[1][2][i];
        if (hpower_aggr_[1][2][i] > 0) hpower_aggr_[1][3][i] = hpower_aggr_[1][0][i] / hpower_aggr_[1][2][i];
    }
}

/*!
Description: Aggregate data will be saved.
*/
void CHarmFunc::AggregateData(time_t time)
{
    hrm_prehandle();

    int i, j;

    if (syspara_->hrm_save_type) { //谐波记录取平均值
        zip_harm_rcd(1); //压缩要保存的记录数据
    } else { //谐波记录取最大值
        for (j = 0; j < 3; j++) { //根据含有率最大值计算出对应谐波幅值
            for (i = 2; i <= MAX_HARM_NUM; i++) {
                harm_aggr_[0][0].array[j][i] = harm_aggr_[1][0].array[j][1] * HRU_aggr_[0].array[j][i] / 100;
            }
        }
        zip_harm_rcd(0); //压缩要保存的记录数据
    }
    memcpy(inter_save_, interhuimx, sizeof(interhuimx));
    WriteStat2Shm(time);
    shmem_func().IncDataUp(1);
}

//统计谐波含有率及畸变率
void CHarmFunc::stat_hr_thd()
{
    int i, j, k;

    for (j = 0; j < 3; j++) {   //Phase A/B/C
        for (i = 2; i <= MAX_HARM_NUM; i++) {   //Harmonic ratio
            if (HRui_[0].array[j][i] > HRU_aggr_[0].array[j][i]) {
                HRU_aggr_[0].array[j][i] = HRui_[0].array[j][i];
            } else if (HRui_[0].array[j][i] < HRU_aggr_[2].array[j][i]) {
                HRU_aggr_[2].array[j][i] = HRui_[0].array[j][i];
            }
            if (HRU_in_.array[j][i] > HRU_aggr_in_[0].array[j][i]) {
                HRU_aggr_in_[0].array[j][i] = HRU_in_.array[j][i];
            } else if (HRU_in_.array[j][i] < HRU_aggr_in_[2].array[j][i]) {
                HRU_aggr_in_[2].array[j][i] = HRU_in_.array[j][i];
            }
        }
        for (i = 0; i < 2; i++) {   //Total harmonic distortion. V/A
            for (k = 0; k < 3; k++) {
                if (THD_[i][k][j] > THD_aggr_[0][i][k][j]) {        //maximum
                    THD_aggr_[0][i][k][j] = THD_[i][k][j];
                } else if (THD_[i][k][j] < THD_aggr_[2][i][k][j]) { //minimum
                    THD_aggr_[2][i][k][j] = THD_[i][k][j];
                }
                THD_aggr_[1][i][k][j] += THD_[i][k][j] * THD_[i][k][j];
            }
        }
    }
}

/*!
Description:Initialize data before aggregation.
*/
void CHarmFunc::InitAggregation()
{
    hrm_refresh_cnt_ = 0;
    memset(HRU_aggr_, 0, sizeof(HarmData <float>) * 2);     //set max&avg to 0
    memset(&HRU_aggr_[2], 0x45, sizeof(HarmData <float>));  //Set min to 3156.329
    memset(HRU_aggr_in_, 0, sizeof(HarmData <float>) * 2);    //set max&avg to 0
    memset(&HRU_aggr_in_[2], 0x45, sizeof(HarmData <float>)); //Set min to 3156.329
    memset(hm_power_aggr_, 0xd3, sizeof(hm_power_aggr_) / 3);   //set max to -1.82e12
    memset(&hm_power_aggr_[1], 0, sizeof(hm_power_aggr_) / 3);  //set avg to 0
    memset(&hm_power_aggr_[2], 0x55, sizeof(hm_power_aggr_) / 3); //Set min to 1.47e13
    memset(THD_aggr_, 0, sizeof(THD_aggr_));            //Set max&avg to 0
    memset(&THD_aggr_[2], 0x45, sizeof(THD_aggr_) / 3); //Set min to 3156.329
    memset(hpower_aggr_, 0xd3, sizeof(hpower_aggr_) / 3);   //Set max to -1.82e12
    memset(&hpower_aggr_[1], 0, sizeof(hpower_aggr_) / 3);  //Set avg to 0
    memset(&hpower_aggr_[2], 0x55, sizeof(hpower_aggr_) / 3); //Set min to 1.47e13
    memset(harm_aggr_in_, 0, sizeof(harm_aggr_in_) * 2 / 3); //Set max&avg to 0
    memset(harm_aggr_in_[2], 1, sizeof(harm_aggr_in_) / 3); //Set min to 16843009(16843V, 1684.3A)
    memset(harm_aggr_, 0, sizeof(harm_aggr_) * 2 / 3);  //Set max&avg to 0
    memset(&harm_aggr_[2][0], 1, sizeof(harm_aggr_) / 3); //Set min to 16843009(16843V, 1684.3A)
    memset(harm_avgbuf_, 0, sizeof(harm_avgbuf_));
    memset(&interhuimx[0][0][0], 0, 6 * (MAX_HARM_NUM + 1) * (INTERH_MAX_RESO - 1) * 2 * 2);
    memset(power_aggr_, 0xd3, sizeof(power_aggr_) / 3);   //set max to -1.82e12
    memset(&power_aggr_[1], 0, sizeof(power_aggr_) / 3);  //set avg to 0
    memset(&power_aggr_[2], 0x55, sizeof(power_aggr_) / 3); //Set min to 1.47e13
}

/*!
Description:读取谐波数据

    Input:  type -- 0=voltage; 1=current;
*/
int CHarmFunc::get_harm_rcd(int type, void *to)
{
    unsigned short *ptto = (unsigned short*)to;
    unsigned short *ptfrm;
    ptfrm = (unsigned short*)harmzip_[type].array;
    int i;
    for (i = 0; i < 3 * (MAX_HARM_NUM + 1); i++) {
        *ptto = *ptfrm;
        ptto++;
        ptfrm++;
    }
    return i;
}

//Description: 读取总谐波畸变率
//Output: to,输出的数值;
void CHarmFunc::get_thd_rcd(void *to)
{
    unsigned short *ptto = (unsigned short*)to;
    unsigned short buf[2][3];

    int i, j;
    if (syspara_->hrm_save_type) { //谐波记录取平均值
        for (i = 0; i < 2; i++) {
            for (j = 0; j < 3; j++) {
                buf[i][j] = THD_aggr_[1][i][0][j] * 100 + 0.5;
            }
        }
    } else { //谐波记录取最大值
        for (j = 0; j < 3; j++) {
            buf[0][j] = THD_aggr_[0][0][0][j] * 100 + 0.5;   //THDu
            buf[1][j] = THD_aggr_[0][1][0][j] * 100 + 0.5;   //THDi
        }
    }
    memcpy(ptto, buf, 6 * sizeof(short));
}
//-------------------------------------------------------------------------

// 设置谐波数据更新时间
void CHarmFunc::SetRecTime(struct timeval * tim)
{
    //将时间调整至3秒的整数倍
    int i = tim->tv_sec % 3;
    tim->tv_sec -= i;
    memcpy(&refresh_time_, &tim->tv_sec, sizeof(time_t));
}

//判断是否缺相
//vc=0,电压; vc=1,电流.
//phs:相位 0=A, 1=B, 2=C 0x0ff=ABC
//Return: true=缺
bool CHarmFunc::lack_phs(int vc, int phs)
{
    if (phs == 0x0ff) {
        if (lack_phs(vc, 0) || lack_phs(vc, 1) || lack_phs(vc, 2))
            return true;
        return false;
    }
    if (phs < 0 || phs > 2) return true; //相位不能大于2

    if (vc) {
        if (harmrms_[1].array[phs][1] < 1)
            return true;
    } else {
        if (harmrms_[0].array[phs][1] < 20)
            return true;
    }
    return false;

}


/*!
Description:Get phase

    Input:  phs -- phase
            vc -- 0=voltage; 1=current;
            hms -- 谐波次数
            range -- 0=0~360; 1=-180~180
    Return: phase
*/
float CHarmFunc::get_phase(int phs, int vc, int hms, int range)
{
    float fi;

    fi = harmphs_[vc].array[phs][hms];
    if (!(prmcfg->debug_para()->debug_enable & 0x1)) { //不处于调试状态
        fi += 900;
        if (fi >= 3600) fi -= 3600;
    }
    if (range) {
        if(fi > 1800) fi -= 3600;
    }
    return fi / 10;
}


/*!
Description: Calc harmonic P&Q. unit is kW
*/
void CHarmFunc::calc_hpower()
{
    int i, j, m;
    double fi, fj;

    if (prmcfg->connect_type()) {    //三相三线(△型接线),二瓦计法
        for (i = 1; i <= MAX_HARM_NUM; i++) {
            fi = harmrms_[0].array[0][i];    //Uab的幅值
            fi = fi * line_para_->PT1 / line_para_->PT2 / units_[0];
            fj = harmrms_[1].array[0][i];    //Ian的幅值
            fj = fj * line_para_->CT1 / line_para_->CT2 / units_[1];

            fi = fi * fj;
            fj = harmphs_[0].array[0][i];
            fj -= harmphs_[1].array[0][i];  //Uab与Ian的相角差
            while (fj < 0) fj += 3600;
            fj = PI2 * fj / 3600; //Convert degrees to radian

            hm_power_[1].array[0][i] = fi * sin(fj) / 1000;
            hm_power_[0].array[0][i] = fi * cos(fj) / 1000;

            fi = harmrms_[0].array[1][i]; //Ubc的幅值，也是Ucb的幅值
            fi = fi * line_para_->PT1 / line_para_->PT2 / units_[0];
            fj = harmrms_[1].array[2][i]; //Icn的幅值
            fj = fj * line_para_->CT1 / line_para_->CT2 / units_[1];

            fi = fi * fj; //
            fj = harmphs_[0].array[1][i] - 1800; //-1800:变成Ucb的相位
            fj -= harmphs_[1].array[2][i];
            while (fj < 0) fj += 3600;
            fj = PI2 * fj / 3600; //Convert degrees to radian

            hm_power_[0].array[0][i] += fi * cos(fj);    //active power
            hm_power_[1].array[0][i] += fi * sin(fj);    //reactive power
            hm_power_[2].array[0][i] = sqrt(hm_power_[0].array[0][i] * hm_power_[0].array[0][i]
                                            + hm_power_[1].array[0][i] * hm_power_[1].array[0][i]); //apparent power
            if(hm_power_[2].array[0][i] > 0) {
                hm_power_[3].array[0][i] = hm_power_[0].array[0][i] / hm_power_[2].array[0][i]; //PF
            } else {
                hm_power_[3].array[0][i] = 0;
            }
        }
        //phase BC&CA clear to 0
        memset(hm_power_[0].array[1], 0, sizeof(HarmData <float>) * 2 / 3);
        memset(hm_power_[1].array[1], 0, sizeof(HarmData <float>) * 2 / 3);
        memset(hm_power_[2].array[1], 0, sizeof(HarmData <float>) * 2 / 3);
        memset(hm_power_[3].array[1], 0, sizeof(HarmData <float>) * 2 / 3);
    } else {        //三相四线(Y型接线)
        for (j = 0; j < 3; j++) {
            for (i = 1; i <= MAX_HARM_NUM; i++) {
                fi = harmrms_[0].array[j][i];
                fi = fi * line_para_->PT1 / line_para_->PT2 / units_[0];
                fj = harmrms_[1].array[j][i];
                fj = fj * line_para_->CT1 / line_para_->CT2 / units_[1];
                fi = fi * fj;
                fj = harmphs_[0].array[j][i];
                fj -= harmphs_[1].array[j][i];
                if (fj < 0) fj += 3600; //2006-10-24
                fj = PI2 * fj / 3600;   //Convert degrees to radian

                hm_power_[0].array[j][i] = fi * cos(fj);    //active power
                hm_power_[1].array[j][i] = fi * sin(fj);    //reactive power
                hm_power_[2].array[j][i] = fi;              //apparent power
                if(hm_power_[2].array[0][i] > 0) {  //PF
                    hm_power_[3].array[j][i] = hm_power_[0].array[j][i] / hm_power_[2].array[j][i];
                } else {
                    hm_power_[3].array[j][i] = 0;
                }
            }
        }
    }
    for (m = 0; m < 4; m++) {   //P,Q,S,PF
        for (j = 0; j < 3; j++) {   //phase
            for (i = 1; i <= MAX_HARM_NUM; i++) {
                fi = hm_power_[m].array[j][i];
                if (fi > hm_power_aggr_[0][m].array[j][i]) {
                    hm_power_aggr_[0][m].array[j][i] = fi;
                } else if (fi < hm_power_aggr_[2][m].array[j][i]) {
                    hm_power_aggr_[2][m].array[j][i] = fi;
                }
                hm_power_aggr_[1][m].array[j][i] += fi;
            }
        }
        for (i = 1; i <= MAX_HARM_NUM; i++) {
            fi = hm_power_[m].array[0][i] + hm_power_[m].array[1][i] + hm_power_[m].array[2][i];
            if (m == 3) fi /= 3;
            if (fi > hm_pw3_aggr_[0][m][i]) {
                hm_pw3_aggr_[0][m][i] = fi;
            } else if (fi < hm_pw3_aggr_[2][m][i]) {
                hm_pw3_aggr_[2][m][i] = fi;
            }
            hm_pw3_aggr_[1][m][i] += fi;
        }
    }
}

//-------------------------------------------------------------------------
// calc global P,Q,S; PF and DPF
void CHarmFunc::calc_glpower()
{
    int i, j;
    float fi, fj;

    for (j = 0; j < 3; j++) {   //phase A,B,C
        fi = fj = 0;
        for (i = 1; i <= MAX_HARM_NUM; i++) {
            fi += hm_power_[0].array[j][i];
            fj += hm_power_[1].array[j][i];
        }
        power_[0][j] = fi;
        power_[1][j] = fj;
        power_[2][j] = sqrt(fi * fi + fj * fj);
        if (power_[2][j]) {
            power_[3][j] = power_[0][j] / power_[2][j];
        } else {
            power_[3][j] = 0;
        }
        fi -= hm_power_[0].array[j][1];
        fj -= hm_power_[1].array[j][1];
        hpower_[0][j] = fi;
        hpower_[1][j] = fj;
        hpower_[2][j] = sqrt(fi * fi + fj * fj);
        if (hpower_[2][j] > 0) {
            hpower_[3][j] = fi / hpower_[2][j];
        }
    }
    for (i = 0; i < 4; i++) { //P,Q,S,PF
        if (i == 3) { //PF
            if ( hpower_[2][3] > 0 ) fi =  hpower_[0][3] /  hpower_[2][3];
            else fi = 0;
        } else {
            fi = hpower_[i][0] + hpower_[i][1] + hpower_[i][2];
        }
        hpower_[i][3] = fi;
        for (j = 0; j < 4; j++) { //A-C,all
            fi = hpower_[i][j];
            if (fi > hpower_aggr_[0][i][j]) {
                hpower_aggr_[0][i][j] = fi;
            } else if (fi < hpower_aggr_[2][i][j]) {
                hpower_aggr_[2][i][j] = fi;
            }
            hpower_aggr_[1][i][j] += fi;
        }

        if (i == 3) { //PF
            if ( power_[2][3] > 0 ) fi =  power_[0][3] /  power_[2][3];
            else fi = 0;
        } else {
            fi = power_[i][0] + power_[i][1] + power_[i][2];
        }
        power_[i][3] = fi;
        for (j = 0; j < 4; j++) { //Phase A-C,all
            fi = power_[i][j];
            if (fi > power_aggr_[0][i][j]) {
                power_aggr_[0][i][j] = fi;
            } else if (fi < power_aggr_[2][i][j]) {
                power_aggr_[2][i][j] = fi;
            }
            power_aggr_[1][i][j] += fi;
        }
    }
}

/*!
Description:Get power

    Input:  type -- 0=harmonic active; 1=harmonic reactive; 2-total active; 3=total reactive;
                    4=total apparent; unit:kW/kvar/kVA. 5=PF; 6=DPF.
*/
float CHarmFunc::get_power(int type, int phase, int hmnum)
{
    float retval;

    switch (type) {
        case 0: // harm P
            retval = hm_power_[0].array[phase][hmnum] / 1000;
            break;
        case 1: // harm Q
            retval = hm_power_[1].array[phase][hmnum] / 1000;
            break;
        case 2: // P
            retval = power_[0][phase] / 1000;
            break;
        case 3: // Q
            retval = power_[1][phase] / 1000;
            break;
        case 4: // S
            retval = power_[2][phase] / 1000;
            break;
        case 5: // PF
            retval = power_[3][phase];
            break;
        case 6: // DPF
            retval = hm_power_[3].array[phase][1];
            break;
        default:
            break;
    }
    return retval;
}

//-------------------------------------------------------------------------
//更新间谐波越限比较基准值
void CHarmFunc::update_IntBase()
{
    int i, k;
    float *tobuf = &IntBase[0][0];

    //电压基准值
    float fi;
    if (prmcfg->connect_type()) fi = 100.0;    //delta circuit
    else fi = 173.2;      //wye circuit
    *tobuf = line_para_->PT2 * HamULimit[line_para_->CUlevel][0] / fi;
    tobuf ++;
    for (i = 1; i < 49; i++) {
        k = i % 2;
        *tobuf = line_para_->PT2 * HamULimit[line_para_->CUlevel][k + 1] / fi;
        tobuf++;
    }
    *tobuf = *(tobuf-1);
    //电流基准值
    *tobuf = hrmi_limit(2) * line_para_->CT2 / line_para_->CT1;
    tobuf ++;
    for (i = 2; i <= 25; i++) {
        *tobuf = hrmi_limit(i) * line_para_->CT2 / line_para_->CT1;
        tobuf ++;
    }
    fi = hrmi_limit(25) * line_para_->CT2 / line_para_->CT1;
    for (i = 0; i < 25; i++) {
        *tobuf = fi;
        tobuf++;
    }
}

/*!
Description:Calculate transformer loss

    Called by:  CalcDerivedData
*/
void CHarmFunc::CalcTrnsfmrLoss()
{
    int i, j;
    float fi;

    float t_p_c = 0;    //P_c of transformer
    float t_p_ec = 0;   //P_{ec} of transformer
    float I1 = 0;       //Fundamental current

    for(j = 0; j < 3; j++) {
        fi = THD_[1][0][j] * THD_[1][0][j] / 10000 + 1;
        if (t_p_c < fi) t_p_c = fi;

        fi = 0;
        for(i = 2; i <= MAX_HARM_NUM; i++) {
            fi += (HRui_[1].array[j][i] * HRui_[1].array[j][i] * i);
        }
        fi /= 10000;
        fi += 1;
        if (t_p_ec < fi) t_p_ec = fi;

        if (I1 < harmrms_[1].array[j][1]) I1 = harmrms_[1].array[j][1];
    }
    I1 = I1 * line_para_->CT1 / line_para_->CT2 / units_[1];

    pee_warning->TWHandle(t_p_c, t_p_ec, I1);
}

/*!
Description:make inter-harmonic data record struction

    Input:  frmbufp --
    Output: cnt -- interharmonic count
    Return: point to buffer
*/
unsigned char *CHarmFunc::make_inter_struct(int &cnt, unsigned short *frmbufp)
{
    unsigned short buf[6 * (MAX_HARM_NUM + 1) * (INTERH_MAX_RESO - 1) * 2];
    //组头3字节 记录头标签+组数目9字节
    unsigned short i, j, k, m, vi_interh, inter, inum, totl, grpnum;
    unsigned char  ph;//相别+谐波次数
    unsigned short reso = kIntrReso[intrh_reso_id_];
    unsigned short *bufp;
    if (!frmbufp) frmbufp = (unsigned short*)inter_save_;
    //间谐波超限判断
    memset(buf, 0, 6 * (MAX_HARM_NUM + 1) * (INTERH_MAX_RESO - 1) * 2 * 2);
    inter_limit_hdl(buf, frmbufp);
    bufp = buf;

    //间谐波数据打包
    k = INTERH_RCD_HEAD;
    memcpy(&interbuf_[0], &k, 2);
    totl = 9;   //前面9个字节
    grpnum = 0;
    for (i = 0; i < 2; i++) { //电压、电流
        for (j = 0; j < 3; j++) { //相位
            for (k = 0; k < 51; k++) { //谐波次数
                ph = j << 6;    //相位标示
                vi_interh = i << 15;    //电压电流标示
                inter = 0;
                inum = 0;
                for (m = 0; m < reso - 1; m++) { //间谐波
                    if (*bufp) {
                        if (!inter) {
                            totl += 3;//3个字节组头
                        }
                        inum++;
                        inter |= 1 << m;
                        memcpy(&interbuf_[totl], bufp, 2);      //幅值
                        totl += 2;
                        memcpy(&interbuf_[totl], (bufp + 1), 2);    //相位
                        totl += 2;
                    }
                    bufp += 2;
                }
                if (inum) { //有间谐波数据
                    ph += k;
                    vi_interh += inter;
                    interbuf_[totl - inum * 4 - 3] = ph;
                    memcpy(&interbuf_[totl - inum * 4 - 2], &vi_interh, 2);
                    grpnum ++;
                    //if (i == 1) printf("grpnum:%d ph:%02x interh:%04x \n", grpnum, ph, vi_interh);
                }
            }
        }
    }
    k = totl - 7;
    memcpy(&interbuf_[4], &k, 2);//总字节数
    interbuf_[6] = intrh_reso_id_;  //间谐波分辨率标示
    memcpy(&interbuf_[7], &grpnum, 2);//组数
    cnt = 0;
    if (grpnum) cnt = totl;
    return interbuf_;
}

//-------------------------------------------------------------------------
//inter-harmonic 越限处理
void CHarmFunc::inter_limit_hdl(unsigned short *tobufp, unsigned short *frmbufp)
{
    unsigned short i, j, k, m, si, slimit, l;
    float fi, fj;
    unsigned short ignore;
    unsigned short reso = kIntrReso[intrh_reso_id_];
    //缩间谐波:frmbuffp来的数据分辨率是1/16，缩成合适的1/8,1/10
    unsigned short cut_buf[6 * (MAX_HARM_NUM + 1) * (INTERH_MAX_RESO - 1) * 2];
    unsigned short *bufp = cut_buf;
    int hrm_num = HRM_MAX_NUM_CUR;

    for (i = 0; i < 6; i++)
        for (j = 0; j < 51; j++)
            for (k = 0; k < (reso - 1) * 2; k++)
                *bufp++ = frmbufp[i * 51 * 30 + j * 30 + k];
    float *fpbase = &IntBase[0][0]; //间谐波超限比较基准值
    slimit = syspara_->inter_limit;
    bufp = cut_buf;
    ignore = (reso / 2 - 1) * 2;    //对于x/10间谐波，跳过1/10-4/10
    bufp += ignore;
    tobufp += ignore;
    for (i = 0; i < 2; i++) {   //电压、电流
        for (j = 0; j < 3; j++) { //相位
            for (k = 1; k < 51; k++) { //谐波次数
                fi = 0;
                for (m = 0; m < reso; m++) {
                    fj = *(bufp + m * 2);
                    fi += fj * fj;
                }
                fi = sqrt(fi) / *(fpbase + i * 50 + k - 1); //与整数次谐波相比

                si = fi;
                if (si >= slimit) { //超限
                    if (k == 1) { //若是1次附近的间谐波，就包括0.1~0.4
                        memcpy(tobufp - ignore, bufp - ignore, (reso + ignore / 2) * 4);
                    } else if (k == hrm_num) {  //若是50次附近的间谐波，就包括50.5 - 50.9
                        memcpy(tobufp, bufp, (reso + ignore / 2) * 4);
                    } else {
                        memcpy(tobufp, bufp, (reso) * 4);
                    }
                }
                bufp += (reso - 1) * 2;
                tobufp += (reso - 1) * 2;
            }
            bufp += (reso - 1) * 2;
            tobufp += (reso - 1) * 2;
        }
    }
}

/*!
Description: Write statistic data to share memory.
*/
void CHarmFunc::WriteStat2Shm(time_t time)
{
    float fi;
    int i, j, m;
    PqmMhai mhai;
    PqmMhaiIntr mhai_in;    //interharmonic statistic
    memset(&mhai, 0, sizeof(mhai));
    memset(&mhai_in, 0, sizeof(mhai_in));

    mhai.time = time;
    mhai_in.time = time;

    for (m = 0; m < 3; m++) { //max, avg, min
        //Harmonic statistic
        memcpy(mhai.thda, THD_aggr_[m][1][0], sizeof(float) * 3);
        memcpy(mhai.thdodda, THD_aggr_[m][1][1], sizeof(float) * 3);
        memcpy(mhai.thdevna, THD_aggr_[m][1][2], sizeof(float) * 3);
        if (prmcfg->connect_type()) {   //delta circuit
            memcpy(mhai.thdppv, THD_aggr_[m][0][0], sizeof(float) * 3);
            memcpy(mhai.thdoddppv, THD_aggr_[m][0][1], sizeof(float) * 3);
            memcpy(mhai.thdevnppv, THD_aggr_[m][0][2], sizeof(float) * 3);
            for (i = 0; i < 3; i++) {
                for (j = 0; j < 50; j++) {
                    fi = harm_aggr_[m][0].array[i][j + 1];
                    mhai.hppv[i][j] = fi * line_para_->PT1 / line_para_->PT2 / units_[0];
                    fi = harm_aggr_in_[m][0].array[i][j];
                    mhai_in.hppv[i][j] = fi * line_para_->PT1 / line_para_->PT2 / units_[0];
                }
                memcpy(mhai.hrppv[i], &HRU_aggr_[m].array[i][2],  sizeof(float) * 49);
                memcpy(mhai_in.hrppv[i], &HRU_aggr_in_[m].array[i][0],  sizeof(float) * 50);
            }
            memset(power_aggr_[m][0], 0, sizeof(float) * 3);
            memset(power_aggr_[m][1], 0, sizeof(float) * 3);
            memset(power_aggr_[m][2], 0, sizeof(float) * 3);
            memset(power_aggr_[m][3], 0, sizeof(float) * 3);
        }
        if (1) { //!prmcfg->connect_type()) {  //wye circuit
            memcpy(mhai.thdphv, THD_aggr_[m][0][0], sizeof(float) * 3);
            memcpy(mhai.thdoddphv, THD_aggr_[m][0][1], sizeof(float) * 3);
            memcpy(mhai.thdevnphv, THD_aggr_[m][0][2], sizeof(float) * 3);
            for (i = 0; i < 3; i++) { //Phase A,B,C
                for (j = 0; j < 50; j++) {
                    fi = harm_aggr_[m][0].array[i][j + 1];
                    mhai.hphv[i][j] = fi * line_para_->PT1 / line_para_->PT2 / units_[0];
                    fi = harm_aggr_in_[m][0].array[i][j + 1];
                    mhai_in.hphv[i][j] = fi * line_para_->PT1 / line_para_->PT2 / units_[0];
                }
                memcpy(mhai.hrphv[i], &HRU_aggr_[m].array[i][2],  sizeof(float) * 49);
                memcpy(mhai_in.hrphv[i], &HRU_aggr_in_[m].array[i][0],  sizeof(float) * 50);
            }
        }
        for (i = 0; i < 3; i++) { //A-C
            for (j = 0; j < 50; j++) {
                mhai.ha[i][j] = (float)harm_aggr_[m][1].array[i][j + 1] * line_para_->CT1 / line_para_->CT2 / units_[1];
                mhai_in.ha[i][j] = (float)harm_aggr_in_[m][1].array[i][j] * line_para_->CT1 / line_para_->CT2 / units_[1];
            }
        }
        if (!prmcfg->connect_type()) {  //wye circuit
            for (i = 0; i < 3; i++) { //A-C
                memcpy(mhai.hw[i], &hm_power_aggr_[m][0].array[i][1], sizeof(float) * 50);
                memcpy(mhai.hvar[i], &hm_power_aggr_[m][1].array[i][1], sizeof(float) * 50);
                memcpy(mhai.hva[i], &hm_power_aggr_[m][2].array[i][1], sizeof(float) * 50);
                memcpy(mhai.hpf[i], &hm_power_aggr_[m][3].array[i][1], sizeof(float) * 50);
                mhai.tothw[i] = hpower_aggr_[m][0][i];
                mhai.tothvar[i] = hpower_aggr_[m][1][i];
                mhai.tothva[i] = hpower_aggr_[m][2][i];
                mhai.tothpf[i] = hpower_aggr_[m][3][i];
            }
        }
        memcpy(mhai.hw[3], &hm_pw3_aggr_[m][0][1], sizeof(float) * 50);
        memcpy(mhai.hvar[3], &hm_pw3_aggr_[m][1][1], sizeof(float) * 50);
        memcpy(mhai.hva[3], &hm_pw3_aggr_[m][2][1], sizeof(float) * 50);
        memcpy(mhai.hpf[3], &hm_pw3_aggr_[m][3][1], sizeof(float) * 50);

        i = 3;  //all
        mhai.tothw[i] = hpower_aggr_[m][0][i];
        mhai.tothvar[i] = hpower_aggr_[m][1][i];
        mhai.tothva[i] = hpower_aggr_[m][2][i];
        mhai.tothpf[i] = hpower_aggr_[m][3][i];

        shmem_func().ShmemCpy(kMhaiStat, &mhai, m);
        shmem_func().ShmemCpy(kMhaiInStat, &mhai_in, m);

        shmem_func().SetMmxu(kMmxuW, power_aggr_[m][0], m);
        shmem_func().SetMmxu(kMmxuVar, power_aggr_[m][1], m);
        shmem_func().SetMmxu(kMmxuVa, power_aggr_[m][2], m);
        shmem_func().SetMmxu(kMmxuPf, power_aggr_[m][3], m);
    }
    //clock_t clkst, clkend;
    //clkst = clock();
    WriteCp95Shm(time);
    //clkend = clock();
    //fi = clkend - clkst;
    //fi /= CLOCKS_PER_SEC;
    //printf("WriteCp95Shm() spend %5.2fs\n", fi);

    WriteQuality2Shm();
    shmem_func().set_statistic_ok();
}

void CHarmFunc::WriteQuality2Shm()
{
    int m;
    unsigned short q = 0;
    if (volt_variation->GetEventMark(kMhaiStatMark)) q = Q_Questionable | Q_Inaccurate;
    for (m = 0; m < 4; m++) { //max, avg, min, cp95
        shmem_func().SetQuality(kMhaiStat, q, m);
        shmem_func().SetQuality(kMhaiInStat, q, m);
    }
    harm_quality_ = q;
    /*q = 0;
    if (volt_variation->GetEventMark(kMmxuStatMark)) q = Q_Questionable|Q_Inaccurate;
    for (m=0; m<4; m++) {   //max, avg, min, cp95
        shmem_func().SetQuality(kMmxuStat, q, m);
    }*/
}

/*!
Description: Write statistic data to share memory.
*/
void CHarmFunc::WriteCp95Shm(time_t time)
{
    float fi, fj, frand;
    int i, j;
    PqmMhai mhai;
    PqmMhaiIntr mhai_in;    //interharmonic statistic
    memset(&mhai, 0, sizeof(mhai));
    memset(&mhai_in, 0, sizeof(mhai_in));

    mhai.time = time;
    mhai_in.time = time;

    frand = rand();
    frand /= RAND_MAX;
    //Harmonic statistic
    for (i = 0; i < 3; i++) {
        mhai.thda[i] = THD_aggr_[1][1][0][i] + (THD_aggr_[0][1][0][i] - THD_aggr_[1][1][0][i]) * frand;
        mhai.thdodda[i] = THD_aggr_[1][1][1][i] + (THD_aggr_[0][1][1][i] - THD_aggr_[1][1][1][i]) * frand;
        mhai.thdevna[i] = THD_aggr_[1][1][2][i] + (THD_aggr_[0][1][2][i] - THD_aggr_[1][1][2][i]) * frand;
    }
    if (prmcfg->connect_type()) {   //delta circuit
        for (i = 0; i < 3; i++) {
            mhai.thdppv[i] = THD_aggr_[1][0][0][i] + (THD_aggr_[0][0][0][i] - THD_aggr_[1][0][0][i]) * frand;
            mhai.thdoddppv[i] = THD_aggr_[1][0][1][i] + (THD_aggr_[0][0][1][i] - THD_aggr_[1][0][1][i]) * frand;
            mhai.thdevnppv[i] = THD_aggr_[1][0][2][i] + (THD_aggr_[0][0][2][i] - THD_aggr_[1][0][2][i]) * frand;
            for (j = 0; j < 50; j++) {
                fi = (float)harm_aggr_[0][0].array[i][j + 1] * line_para_->PT1 / line_para_->PT2 / units_[0];
                fj = (float)harm_aggr_[1][0].array[i][j + 1] * line_para_->PT1 / line_para_->PT2 / units_[0];
                mhai.hppv[i][j] = fj + (fi - fj) * frand;
                fi = (float)harm_aggr_in_[0][0].array[i][j] * line_para_->PT1 / line_para_->PT2 / units_[0];
                fj = (float)harm_aggr_in_[1][0].array[i][j] * line_para_->PT1 / line_para_->PT2 / units_[0];
                mhai_in.hppv[i][j] = fj + (fi - fj) * frand;
                fi = HRU_aggr_in_[0].array[i][j];
                fj = HRU_aggr_in_[1].array[i][j];
                mhai_in.hrppv[i][j] = fj + (fi - fj) * frand;
            }
            for (j = 0; j < 49; j++) {
                fi = HRU_aggr_[0].array[i][j + 2];
                fj = HRU_aggr_[1].array[i][j + 2];
                mhai.hrppv[i][j] = fj + (fi - fj) * frand;
            }
        }
    }
    if (1) { //!prmcfg->connect_type()) {                    //wye circuit
        for (i = 0; i < 3; i++) {
            mhai.thdphv[i] = THD_aggr_[1][0][0][i] + (THD_aggr_[0][0][0][i] - THD_aggr_[1][0][0][i]) * frand;
            mhai.thdoddphv[i] = THD_aggr_[1][0][1][i] + (THD_aggr_[0][0][1][i] - THD_aggr_[1][0][1][i]) * frand;
            mhai.thdevnphv[i] = THD_aggr_[1][0][2][i] + (THD_aggr_[0][0][2][i] - THD_aggr_[1][0][2][i]) * frand;
            for (j = 0; j < 50; j++) {
                fi = (float)harm_aggr_[0][0].array[i][j + 1] * line_para_->PT1 / line_para_->PT2 / units_[0];
                fj = (float)harm_aggr_[1][0].array[i][j + 1] * line_para_->PT1 / line_para_->PT2 / units_[0];
                mhai.hphv[i][j] = fj + (fi - fj) * frand;
                fi = (float)harm_aggr_in_[0][0].array[i][j] * line_para_->PT1 / line_para_->PT2 / units_[0];
                fj = (float)harm_aggr_in_[1][0].array[i][j] * line_para_->PT1 / line_para_->PT2 / units_[0];
                mhai_in.hphv[i][j] = fj + (fi - fj) * frand;
                fi = HRU_aggr_in_[0].array[i][j];
                fj = HRU_aggr_in_[1].array[i][j];
                mhai_in.hrphv[i][j] = fj + (fi - fj) * frand;
            }
            for (j = 0; j < 49; j++) {
                fi = HRU_aggr_[0].array[i][j + 2];
                fj = HRU_aggr_[1].array[i][j + 2];
                mhai.hrphv[i][j] = fj + (fi - fj) * frand;
            }
        }
    }
    for (i = 0; i < 3; i++) { //Phase A,B,C
        for (j = 0; j < 50; j++) {
            fi = (float)harm_aggr_[0][1].array[i][j + 1] * line_para_->CT1 / line_para_->CT2 / units_[1];
            fj = (float)harm_aggr_[1][1].array[i][j + 1] * line_para_->CT1 / line_para_->CT2 / units_[1];
            mhai.ha[i][j] = fj + (fi - fj) * frand;
            fi = (float)harm_aggr_in_[0][1].array[i][j + 1] * line_para_->CT1 / line_para_->CT2 / units_[1];
            fj = (float)harm_aggr_in_[1][1].array[i][j + 1] * line_para_->CT1 / line_para_->CT2 / units_[1];
            mhai_in.ha[i][j] = fj + (fi - fj) * frand;
        }
    }
    if (!prmcfg->connect_type()) {   //wye circuit
        for (i = 0; i < 3; i++) { //Phase A,B,C
            for (j = 0; j < 50; j++) {
                mhai.hw[i][j] = hm_power_aggr_[1][0].array[i][j + 1] + (hm_power_aggr_[0][0].array[i][j + 1] - hm_power_aggr_[1][0].array[i][j + 1]) * frand;
                mhai.hvar[i][j] = hm_power_aggr_[1][1].array[i][j + 1] + (hm_power_aggr_[0][1].array[i][j + 1] - hm_power_aggr_[1][1].array[i][j + 1]) * frand;
                mhai.hva[i][j] = hm_power_aggr_[1][2].array[i][j + 1] + (hm_power_aggr_[0][2].array[i][j + 1] - hm_power_aggr_[1][2].array[i][j + 1]) * frand;
                mhai.hpf[i][j] = hm_power_aggr_[1][3].array[i][j + 1] + (hm_power_aggr_[0][3].array[i][j + 1] - hm_power_aggr_[1][3].array[i][j + 1]) * frand;
            }
            mhai.tothw[i] = hpower_aggr_[1][0][i] + (hpower_aggr_[0][0][i] - hpower_aggr_[1][0][i]) * frand;
            mhai.tothvar[i] = hpower_aggr_[1][1][i] + (hpower_aggr_[0][1][i] - hpower_aggr_[1][1][i]) * frand;
            mhai.tothva[i] = hpower_aggr_[1][2][i] + (hpower_aggr_[0][2][i] - hpower_aggr_[1][2][i]) * frand;
            mhai.tothpf[i] = hpower_aggr_[1][3][i] + (hpower_aggr_[0][3][i] - hpower_aggr_[1][3][i]) * frand;
        }
    }
    for (j = 0; j < 50; j++) { //all
        mhai.hw[3][j] = hm_pw3_aggr_[1][0][j + 1] + (hm_pw3_aggr_[0][0][j + 1] - hm_pw3_aggr_[1][0][j + 1]) * frand;
        mhai.hvar[3][j] = hm_pw3_aggr_[1][1][j + 1] + (hm_pw3_aggr_[0][1][j + 1] - hm_pw3_aggr_[1][1][j + 1]) * frand;
        mhai.hva[3][j] = hm_pw3_aggr_[1][2][j + 1] + (hm_pw3_aggr_[0][2][j + 1] - hm_pw3_aggr_[1][2][j + 1]) * frand;
        mhai.hpf[3][j] = hm_pw3_aggr_[1][3][j + 1] + (hm_pw3_aggr_[0][3][j + 1] - hm_pw3_aggr_[1][3][j + 1]) * frand;
    }
    i = 4; //all
    mhai.tothw[i] = hpower_aggr_[1][0][i] + (hpower_aggr_[0][0][i] - hpower_aggr_[1][0][i]) * frand;
    mhai.tothvar[i] = hpower_aggr_[1][1][i] + (hpower_aggr_[0][1][i] - hpower_aggr_[1][1][i]) * frand;
    mhai.tothva[i] = hpower_aggr_[1][2][i] + (hpower_aggr_[0][2][i] - hpower_aggr_[1][2][i]) * frand;
    mhai.tothpf[i] = hpower_aggr_[1][3][i] + (hpower_aggr_[0][3][i] - hpower_aggr_[1][3][i]) * frand;

    shmem_func().ShmemCpy(kMhaiStat, &mhai, 3);
    shmem_func().ShmemCpy(kMhaiInStat, &mhai_in, 3);

    //Measurement statistic
    float fary[4];
    for (i = 0; i < 4; i++) fary[i] = power_aggr_[1][0][i] + (power_aggr_[0][0][i] - power_aggr_[1][0][i]) * frand;
    shmem_func().SetMmxu(kMmxuW, fary, 3);
    for (i = 0; i < 4; i++) fary[i] = power_aggr_[1][1][i] + (power_aggr_[0][1][i] - power_aggr_[1][1][i]) * frand;
    shmem_func().SetMmxu(kMmxuVar, fary, 3);
    for (i = 0; i < 4; i++) fary[i] = power_aggr_[1][2][i] + (power_aggr_[0][2][i] - power_aggr_[1][2][i]) * frand;
    shmem_func().SetMmxu(kMmxuVa, fary, 3);
    for (i = 0; i < 4; i++) fary[i] = power_aggr_[1][3][i] + (power_aggr_[0][3][i] - power_aggr_[1][3][i]) * frand;
    shmem_func().SetMmxu(kMmxuPf, fary, 3);
}

