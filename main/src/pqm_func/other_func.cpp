#include "other_func.h"
#include "harmfunc.h"
#include "../include/flick.h"
#include "../device/device.h"
#include "../IPC/shmemfunc.h"
#include "volt_variation.h"
#include <cmath>
#include <cstdio>
using namespace std;

OtherFunc *other_func;

OtherFunc::OtherFunc()
{
    syspara_ = prmcfg->syspara();
    line_para_ = prmcfg->line_para();
    InitFreq();
    memset(freq_aggr2_, 0, sizeof(long)*2); //set max&avg to 0
    freq_aggr2_[2] = 8000;  //set min to 80.00Hz
    freq_cnt_ = 0;
    
    InitRms();
    InitUnblc();

    memset(frequency_, 0, sizeof(frequency_));
    //初始化闪变计算参数
    IniFlkPm(81.6);
    Pst_cnt_ = 0;
    Pst12_cnt_ = 0;
    refresh_cnt_[kPstSave] = 0;
    memset(fluct_cnt_, 0, sizeof(fluct_cnt_));
    memset(fluct_max_, 0, sizeof(fluct_max_));
    memset(quality_, 0, sizeof(quality_));
    freq_save_ = new LoopBuffer<FreqSaveModel>(15);
    unblc_save_ = new LoopBuffer<UnblncSaveModel>(15);
    voltdv_save_ = new LoopBuffer<VoltDvSaveModel>(15);
}

OtherFunc::~OtherFunc()
{
    delete freq_save_;
    delete unblc_save_;
    delete voltdv_save_;
}

/*!
Description:Initialize data be monitored

    Input:  type
*/
void OtherFunc::InitData(SaveFileType type)
{
    switch (type) {
        case kPstSave:
            refresh_cnt_[type] = 0;
            break;
        case kFreqSave:
            InitFreq();
            break;
        case kUnblcSave:
            InitUnblc();
            break;
        case kVoltdvSave:
            InitRms();
        default:
            break;
    }
}

/*!
Description:Set unbalance value calculated on dsp board

        input: pu_seq_compnt -- Voltage sequence component. 0~2=zero,positive,negative
               pi_seq_compnt -- Current sequence component. 0~2=zero,positive,negative
*/
void OtherFunc::SetUnbalance(unsigned short * pu_seq_compnt, unsigned short * pi_seq_compnt)
{
    memcpy(unbalance_.array[0], pu_seq_compnt, sizeof(short) * 3);
    memcpy(unbalance_.array[1], pi_seq_compnt, sizeof(short) * 3);
    if (prmcfg->connect_type()) { //VV 接线
        unbalance_.array[0][0] = 0;
        unbalance_.array[1][0] = 0;
    }

    // Calculate unbalance
    int i, j, k, zer, pos, neg;
    float fi;
    for (i = 0; i < 2; i++) {
        if (i) k = 10;  //10mA
        else k = 5;  //0.05V
        zer = unbalance_.array[i][0];   //Zero
        pos = unbalance_.array[i][1];   //Positive
        neg = unbalance_.array[i][2];   //Negtive
        if (zer < k && pos < k && neg < k) {
            unbalance_.array[i][3] = 0;
            unbalance_.array[i][4] = 0;
        } else {
            if (!pos) unbalance_.array[i][1] = 0.001;
            fi = unbalance_.array[i][2];
            fi = fi / unbalance_.array[i][1];
            unbalance_.array[i][3] = fi * 10000 > 65000 ? 65533 : fi * 10000;
            fi = unbalance_.array[i][0];
            fi = fi / unbalance_.array[i][1];
            unbalance_.array[i][4] = fi * 10000 > 65000 ? 65533 : fi * 10000;
        }
    }
    

    // Statistical data process
    long li;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            li = unbalance_.array[i][j];
            unblc_average_.array[i][j] += li * li;  //Avetage
            if (unblc_aggr_[0].array[i][j] < unbalance_.array[i][j])    //Maximum
                unblc_aggr_[0].array[i][j] = unbalance_.array[i][j];
            if (unblc_aggr_[2].array[i][j] > unbalance_.array[i][j])    //Minimum
                unblc_aggr_[2].array[i][j] = unbalance_.array[i][j];
        }
    }
    refresh_cnt_[kUnblcSave]++;
}

/*!
Description: Aggregate unbalance will be saved.
*/
void OtherFunc::AggregateUnblc(time_t time, bool adj_tm)
{
    int i, j, k[2];

    float fi = refresh_cnt_[kUnblcSave];
    for (i = 0; i < 2; i++) {   //Calculate average value
        for (j = 0; j < 5; j++) {
            unblc_aggr_[1].array[i][j] = sqrt(unblc_average_.array[i][j] / fi) + 0.5;
        }
    }
    
    if (syspara_->unbalance_save_type) { //不平衡记录取平均值
        for (i = 0; i < 2; i++) {
            for (j = 0; j < 3; j++) {
                unblc_svtmp_.data[i][j] = sqrt(unblc_average_.array[i][j] / fi) + 0.5;
            }
            k[i] = unblc_aggr_[1].array[i][3]; //unbalance
        }
    } else { //不平衡记录取最大值
        for (i = 0; i < 2; i++) {
            for (j = 0; j < 3; j++) {
                unblc_svtmp_.data[i][j] = unblc_aggr_[0].array[i][j];
            }
            k[i] = unblc_aggr_[0].array[i][3]; //unbalance
        }
    }
    //根据不平衡与负序分量计算待保存的正序分量
    for (i = 0; i < 2; i++) {
        fi = unblc_svtmp_.data[i][2];
        if (k[i]>0) {
            fi /= k[i];
            fi *= 10000;
            unblc_svtmp_.data[i][1] = fi;
        }
    }
    //Write unbalance data to share memory
    shmem_func().set_stat_note(true);
    WriteStat2Shm(kUnblcSave, time);
    unblc_svtmp_.time = time;
    unblc_svtmp_.q = quality_[kUnblcSave];
    if(adj_tm) unblc_svtmp_.q |= 1;
    unblc_save_->push(&unblc_svtmp_);
    //WriteStat2Shm(kFreqSave, time); //放在此处，是为了与其他数据保持同步
}

/*!
Description: Initailize unbalance data.
*/
void OtherFunc::InitUnblc()
{
    memset(&unbalance_, 0, sizeof(unbalance_));
    memset(unblc_aggr_, 0, sizeof(unblc_aggr_)*2/3);  //Set max&avg to 0
    memset(&unblc_aggr_[2], 0xff, sizeof(unblc_aggr_)/3); //Set min to 65535(655.35V, 65.535A, 655.35%)
    memset(&unblc_average_, 0, sizeof(unblc_average_));
    refresh_cnt_[kUnblcSave] = 0;
}

/*!
Description: Initailize frequency data.
*/
void OtherFunc::InitFreq()
{
    memset(freq_aggr_, 0, sizeof(freq_aggr_));
    freq_aggr_[2] = 8000;   //minimum
    refresh_cnt_[kFreqSave] = 0;
}

void OtherFunc::SetFrequency(unsigned short *freq)
{
    for (int i=0; i<2; i++) {
        if (u_deviation_.array[0][0] < 1.0 || freq[i] == 0) {
            frequency_[i] = 0;
        } else if (freq[i] < 3500 || freq[i] > 7000) {
            frequency_[i] = 5000;
        } else {
            frequency_[i] = freq[i];
        }
    }
    freq_idx_ = prmcfg->freq_revaltm();
    freq_aggr_[1] += frequency_[freq_idx_];
    if (frequency_[freq_idx_]>freq_aggr_[0]) freq_aggr_[0] = frequency_[freq_idx_];
    if (frequency_[freq_idx_]<freq_aggr_[2]) freq_aggr_[2] = frequency_[freq_idx_];
    refresh_cnt_[kFreqSave]++;

    freq_aggr2_[1] += frequency_[freq_idx_];
    if (frequency_[freq_idx_]>freq_aggr2_[0]) freq_aggr2_[0] = frequency_[freq_idx_];
    if (frequency_[freq_idx_]<freq_aggr2_[2]) freq_aggr2_[2] = frequency_[freq_idx_];
    freq_cnt_++;
}

/*!
Description: Aggregate frequency will be saved.
*/
void OtherFunc::AggregateFreq(time_t time, bool adj_tm)
{
    freq_aggr_[1] /= refresh_cnt_[kFreqSave];
    if (syspara_->freq_save_type) { //频率记录取平均值
        freq_svtmp_.data = freq_aggr_[1];
    } else { //频率记录取最大值
        freq_svtmp_.data = (freq_aggr_[0]-5000)>(5000-freq_aggr_[2])?freq_aggr_[0]:freq_aggr_[2];
    }
    WriteStat2Shm(kFreqSave, time);
    freq_svtmp_.time = time;
    freq_svtmp_.q = quality_[kFreqSave];
    if (adj_tm==true) freq_svtmp_.q |= 1;
    freq_save_->push(&freq_svtmp_);
}

/*!
Description:Set Pst data

    Input:  fluck -- flucktuation data
*/
void OtherFunc::SetPst(float * fluckt)
{
    int i;

    //float *p = fluckt;
    for (i = 0; i < 3; i++) {
        //printf("%d:%5.3f %5.3f %5.3f %5.3f ... %5.3f %5.3f %5.3f %5.3f\n", i, p[0], p[1], p[2], p[3], p[28], p[29], p[30], p[31] );
        Pst_stat(i, Pst_cnt_, fluckt);
        //p += 32;
    }
    //printf("\n");

    if (++Pst_cnt_ >= 600) Pst_cnt_ = 0;

    refresh_cnt_[kPstSave]++;
    int n = refresh_cnt_[kPstSave];
    if (!(n % 60) && n < 590) {
        for (i = 0; i < 3; i++) {
            Pst_[i] = get_pst_result(i, prmcfg->debug_para()->Pst_mdfy_enable & 0x1);
            if (harmfunc->get_harm_data(HARM_AMP2, i, 0, 1) <= 2.0) //是否有电压
                Pst_[i] = 0;
        }
    }
}

/*!
Description: Aggregate data will be saved.
*/
void OtherFunc::Aggregate(SaveFileType type, time_t time, bool adj_tm)
{
    switch (type) {
        case kPstSave:
            AggregatePst(time, adj_tm);
            break;
        case kFreqSave:
            AggregateFreq(time, adj_tm);
            break;
        case kUnblcSave:
            AggregateUnblc(time, adj_tm);
            break;
        case kVoltdvSave:
            AggregateVoltdv(time, adj_tm);
        default:
            break;
    }
}

/*!
Description: Aggregate Pst will be saved.
*/
void OtherFunc::AggregatePst(time_t time, bool adj_tm)
{
    for (int i = 0; i < 3; i++) {
        Pst_[i] = get_pst_result(i, prmcfg->debug_para()->Pst_mdfy_enable & 0x1);
        if (harmfunc->get_harm_data(HARM_AMP2, i, 0, 1) <= 2.0) //是否有电压
            Pst_[i] = 0;
        Pst_save_.data[i] = Pst_[i] * 1000;
        Pst12_buf_[i][Pst12_cnt_] = Pst_[i];
    }
    pst_tmval_.tv_sec = time;

    float fi_ary[3];
    tm tmi;
    GmTime(&tmi, &time);
    Pst12_cnt_++;
    if (!(tmi.tm_hour%2)&&!tmi.tm_min) {
        memset(fi_ary, 0, sizeof(fi_ary));
        for(int i = 0; i < 3; i++) {
            for (int j = 0; j < Pst12_cnt_; j++) {
                fi_ary[i] += Pst12_buf_[i][j] * Pst12_buf_[i][j] * Pst12_buf_[i][j];
            }
            Plt_[i] = cbrt(fi_ary[i] / Pst12_cnt_);
        }
        plt_tmval_.tv_sec = time;
        shmem_func().SetMflk(kMflkPlt, Plt_, prmcfg->connect_type(), time);
        for (int i=0;i<3;i++) fi_ary[i] = (float)fluct_cnt_[i]/2;
        shmem_func().SetMflk(kMflkFlucf, fi_ary, prmcfg->connect_type());
        WriteQuality2Shm(kPstSave, kPltMark);
        memset(fluct_cnt_, 0, sizeof(fluct_cnt_));
        Pst12_cnt_ = 0;
    }
    if (Pst12_cnt_>=20) Pst12_cnt_ = 0;
    for (int i=0;i<3;i++) fi_ary[i] = fluct_max_[i]*100/u_din_;
    shmem_func().SetMflk(kMflkFluc, fi_ary, prmcfg->connect_type());
    WriteQuality2Shm(kPstSave, kFluctMark);
    shmem_func().SetMflk(kMflkPst, Pst_, prmcfg->connect_type(), time);
    WriteQuality2Shm(kPstSave, kPstMark);
    shmem_func().IncDataUp(1);
    Pst_save_.time = time;
    Pst_save_.q = quality_[kPstSave];
    if (adj_tm==true) Pst_save_.q |= 1;

    memset(fluct_max_, 0, sizeof(fluct_max_));
}

/*!
Description: Write statistic data to share memory.
*/
void OtherFunc::WriteStat2Shm(SaveFileType type, time_t time)
{
    float fi, fj;
    int i, j;
    
    if (type==kFreqSave) {
        freq_aggr2_[1] /= freq_cnt_;
        for (i=0;i<3;i++) {   //max,avg,min
            fi = freq_aggr2_[i];
            fi /= 100;
            shmem_func().SetMmxu(kMmxuHz, &fi, i, time);
            fi -= 50;
            shmem_func().SetMmxu(kMmxuHzdev, &fi, i);
        }
    } else if (type==kUnblcSave) {
        PqmMsqi msqi;
        msqi.time = time;
        fi = line_para_->CT1; fi /= line_para_->CT2;
        fj = line_para_->PT1; fj /= line_para_->PT2;
        for (i=0;i<3;i++) {     //max,avg,min
            for (j=0;j<3;j++) msqi.seqa[j] = (float)unblc_aggr_[i].array[1][((j+1)%3)]*fi/1000;
            msqi.imbnga = (float)unblc_aggr_[i].array[1][3]/100;
            msqi.imbzroa = (float)unblc_aggr_[i].array[1][4]/100;
            for (j=0;j<3;j++) msqi.seqv[j] = (float)unblc_aggr_[i].array[0][((j+1)%3)]*fj/100;
            msqi.imbngv = (float)unblc_aggr_[i].array[0][3]/100;
            msqi.imbzrov = (float)unblc_aggr_[i].array[0][4]/100;
            
            shmem_func().ShmemCpy(kMsqiStat, &msqi, i);
        }
    } else if (type==kVoltdvSave) {
        for (i=0; i<3; i++) {   //max,avg,min
            shmem_func().SetMmxu(kMmxuA, i_rms_aggr_[i], i, time);
            if (prmcfg->connect_type()) {   //Delta circuit
                shmem_func().SetMmxu(kMmxuPpv, voltdv_aggr_[i].array[0], i);
                shmem_func().SetMmxu(kMmxuPpvdev, voltdv_[i], i);
            }
            if (1) { //!prmcfg->connect_type()) {    //Wye circuit
                shmem_func().SetMmxu(kMmxuPhv, voltdv_aggr_[i].array[0], i);
                shmem_func().SetMmxu(kMmxuPhvdev, voltdv_[i], i);
            }
        }
    }
    WriteCp95Shm(type, time);
    if (type==kFreqSave) {
        memset(freq_aggr2_, 0, sizeof(long)*2); //set max&avg to 0
        freq_aggr2_[2] = 8000;  //set min to 80.00Hz
        freq_cnt_ = 0;
    }
    WriteQuality2Shm(type);
}

/*!
    Input:  subtype -- refer to EventMark
*/
void OtherFunc::WriteQuality2Shm(SaveFileType type,  int subtype)
{
    int m;
    unsigned short q = 0;
    if (type==kFreqSave) {
        if (volt_variation->GetEventMark(kFreqStatMark)) q = Q_Questionable|Q_Inaccurate;
        for (m=0; m<4; m++) {   //max, avg, min, cp95
            shmem_func().SetQuality(kMmxuStat, q, m, 1);
        }
    } else if (type==kUnblcSave) {
        if (volt_variation->GetEventMark(kMsqiStatMark)) q = Q_Questionable|Q_Inaccurate;
        for (m=0; m<4; m++) {   //max, avg, min, cp95
            shmem_func().SetQuality(kMsqiStat, q, m);
        }
    } else if (type==kVoltdvSave) {
        if (volt_variation->GetEventMark(kMmxuStatMark)) q = Q_Questionable|Q_Inaccurate;
        for (m=0; m<4; m++) {   //max, avg, min, cp95
            shmem_func().SetQuality(kMmxuStat, q, m);
        }
    } else if (type==kPstSave) {
        if (volt_variation->GetEventMark((EventMark)subtype)) q = Q_Questionable|Q_Inaccurate;
        shmem_func().SetQuality(kMflkData, q, 0, subtype-kPstMark);
    }
    quality_[type] = q;
}

/*!
Description: Write statistic data to share memory.
*/
void OtherFunc::WriteCp95Shm(SaveFileType type, time_t time)
{
#if 1
    float fi, fj, fct,fpt;
    int i, j;
    float frand = rand();
    frand /= RAND_MAX;
    
    if (type==kFreqSave) {
        fi = freq_aggr2_[0];
        fj = freq_aggr2_[1];
        fi = fj+(fi-fj)*frand;
        i = fi+0.5;
        fi = (float)i/100;
        shmem_func().SetMmxu(kMmxuHz, &fi, 3, time);
        fi -= 50;
        shmem_func().SetMmxu(kMmxuHzdev, &fi, 3);
    } else if (type==kUnblcSave) {
        PqmMsqi msqi;
        msqi.time = time;
        fct = line_para_->CT1; fct /= line_para_->CT2;
        fpt = line_para_->PT1; fpt /= line_para_->PT2;
        for (j=0; j<3; j++) {
            fi = (float)unblc_aggr_[0].array[1][((j+1)%3)]*fct;
            fj = (float)unblc_aggr_[1].array[1][((j+1)%3)]*fct;
            fi = fj+ (fi-fj)*frand;
            i = fi + 0.5;
            msqi.seqa[j] = (float)i/1000;
        }
        fi = (float)unblc_aggr_[0].array[1][3];
        fj = (float)unblc_aggr_[1].array[1][3];
        fi = fj+ (fi-fj)*frand;
        i = fi + 0.5;
        msqi.imbnga = (float)i/100;
        fi = (float)unblc_aggr_[0].array[1][4];
        fj = (float)unblc_aggr_[1].array[1][4];
        fi = fj+ (fi-fj)*frand;
        i = fi + 0.5;
        msqi.imbzroa = (float)i/100;
        for (j=0;j<3;j++) {
            fi = (float)unblc_aggr_[0].array[0][((j+1)%3)]*fpt;
            fj = (float)unblc_aggr_[1].array[0][((j+1)%3)]*fpt;
            fi = fj+ (fi-fj)*frand;
            i = fi + 0.5;
            msqi.seqv[j] = (float)i/100;
        }
        fi = (float)unblc_aggr_[0].array[0][3];
        fj = (float)unblc_aggr_[1].array[0][3];
        fi = fj+ (fi-fj)*frand;
        i = fi + 0.5;
        msqi.imbngv = (float)i/100;
        fi = (float)unblc_aggr_[0].array[0][4];
        fj = (float)unblc_aggr_[1].array[0][4];
        fi = fj+ (fi-fj)*frand;
        i = fi + 0.5;
        msqi.imbzrov = (float)i/100;
        shmem_func().ShmemCpy(kMsqiStat, &msqi, 3);
    } else if (type==kVoltdvSave) {
        float fary[3];
        for (i=0; i<3; i++) {
            fary[i] = i_rms_aggr_[1][i] + (i_rms_aggr_[0][i]-i_rms_aggr_[1][i])*frand;
        }
        shmem_func().SetMmxu(kMmxuA, fary, i, time);
        if (prmcfg->connect_type()) {   //Delta circuit
            for (i=0; i<3; i++) {
                fary[i] = voltdv_aggr_[1].array[0][i] + (voltdv_aggr_[0].array[0][i]-voltdv_aggr_[1].array[0][i])*frand;
            }
            shmem_func().SetMmxu(kMmxuPpv, fary, 3);
            for (i=0; i<3; i++) fary[i] = voltdv_[1][i] + (voltdv_[0][i]-voltdv_[1][i])*frand;
            shmem_func().SetMmxu(kMmxuPpvdev, fary, 3);
        }
        if (1) { //!prmcfg->connect_type() {    //Wye circuit
            for (i=0; i<3; i++) {
                fary[i] = voltdv_aggr_[1].array[0][i] + (voltdv_aggr_[0].array[0][i]-voltdv_aggr_[1].array[0][i])*frand;
            }
            shmem_func().SetMmxu(kMmxuPhv, fary, 3);
            for (i=0; i<3; i++) fary[i] = voltdv_[1][i] + (voltdv_[0][i]-voltdv_[1][i])*frand;
            shmem_func().SetMmxu(kMmxuPhvdev, fary, 3);
        }
    }
#endif
}

/*!
Description: 不平衡超限检查

    Output: words -- 设置报警字
    Return: true=超限，false=未超限
*/
bool OtherFunc::AlarmCheckUnbalance(unsigned int &words, time_t time)
{
    bool retval;
    float limit;

    retval = false;
    //Check unbalance
    for (int i = 0; i < 2; i++) {
        limit = syspara_->unbalance_thr[i];
        limit /= 10;
        if (unbalance_.array[i][3] > limit) retval = true;
    }
    if (retval) words |= 0x2000;
    else words &= ~(0x2000);
    //Send alarm to share memory
    unsigned short alm = 0;
    if (retval) alm |= 1;
    shmem_func().SetAlm(kAlmUnblc, &alm, 1, time);

    //Check negative sequence components of current
    limit = syspara_->neg_sequence_Ithr;
    limit /= 10;
    if (unbalance_.array[1][2] > limit) {
        retval = true;
        words |= (1 << 14);
    } else {
        words &= ~(1 << 14);
    }
    return retval;
}

/*!
Description: Pst超限检查

    Output: words -- 设置报警字
    Return: true=超限，false=未超限
*/
bool OtherFunc::AlarmCheckPst(unsigned int&word)
{
    bool retval = false;
    float flimit;
    unsigned int almwd = 0;
    long vi = line_para_->PT1;
    //Pst check
    if (!syspara_->limit_type) { //超限类型为国标
        if (vi <= 1000) {
            flimit = 1;
        } else if (vi <= 35000) {
            flimit = 0.9;
        } else {
            flimit = 0.8;
        }
    } else { //自定义超限类型
        flimit = syspara_->pst_limit;
        flimit /= 100;
    }
    for (int i = 0; i < 3; i++) {
        if (Pst_[i] > flimit) almwd |= 1 << i;
    }
    //Send alarm to share memory
    unsigned short alm = 0;
    if (almwd&0x7) alm |= 1;
    shmem_func().SetAlm(kAlmPst, &alm, 1, pst_tmval_.tv_sec);
    
    //Plt check
    if (!syspara_->limit_type) { //超限类型为国标
        if (vi <= 110000) {
            flimit = 1;
        } else {
            flimit = 0.8;
        }
    } else { //自定义超限类型
        flimit = syspara_->plt_limit;
        flimit /= 100;
    }
    almwd = 0;
    for (int i = 0; i < 3; i++) {
        if (Plt_[i] > flimit) almwd |= 1 << i;
    }
    //Send alarm to share memory
    alm = 0;
    if (almwd&0x7) alm |= 1;
    shmem_func().SetAlm(kAlmPlt, &alm, 1, plt_tmval_.tv_sec);

    if (almwd) {
        word |= (1 << 15);
        retval = true;
    } else word &= ~(1 << 15);

    set_alarm(1, almwd);    //Control relay for switch out
    return retval;
}

/*!
Description: Deviation

    Output: words -- 设置报警字
    Return: true=超限，false=未超限
*/
bool OtherFunc::AlarmCheckDeviation(unsigned int&word, time_t time)
{
    float fi[2];
    bool retval = false;
    // 检查电压偏差是否超限.
    for (int j = 0; j < 3; j++) {
        fi[0] = (u_deviation_.array[1][j] - u_din_) / u_din_; //Calculate deviation-over
        fi[1] = (u_deviation_.array[2][j] - u_din_) / u_din_; //Calculate deviation-under
        if ( fi[0] > line_para_->Volt_warp[0]
             || fi[1] < line_para_->Volt_warp[1] ) {
            retval = true;
            word |= (APH_UP << j);
        } else {
            word &= ~(APH_UP << j);
        }
    }
    
    //Send alarm to share memory
    unsigned short alm = 0;
    if (fi[0] > line_para_->Volt_warp[0]) alm |= 1;
    else if (fi[1] < line_para_->Volt_warp[1]) alm |= 0x2;
    shmem_func().SetAlm(kPosVoltdv, &alm, 2, time);
    
    return retval;
}

/*!
Description: Check alarm

    Output: words -- 设置报警字
*/
void OtherFunc::AlarmCheck(unsigned int &words, time_t time)
{
    if (frequency_[freq_idx_] == 0) {//无信号，不报警
        words = 0;
        return;
    }

    AlarmCheckUnbalance(words, time);
    AlarmCheckFreq(words, time);
    AlarmCheckDeviation(words, time);
    AlarmCheckPst(words);
}

void OtherFunc::GetShmReal(PqmMmxu *mmxu, PqmMsqi *msqi)
{
    int i, j;
    mmxu->hz = (float)frequency_[freq_idx_] / 100;
    mmxu->hzdev = mmxu->hz - 50;
    
    for (i=0;i<3;i++) {
        mmxu->ppv_mag[i] = u_rms_ppv(1, i);
        mmxu->a_mag[i] = i_rms(1, i);
    }
    if (prmcfg->connect_type()) {   //delta circuit
        for (i=0;i<3;i++) {
            mmxu->ppvdev[i] = u_deviation(i)*100;
        }
    } 
    if (1) { //!prmcfg->connect_type() {    //wye circuit
        for (i=0;i<3;i++) {
            mmxu->phv_mag[i] = u_rms(1, i);
            mmxu->phvdev[i] = u_deviation(i)*100;
        }
    }
    for (j=0;j<3;j++) msqi->seqa[j] = unbalance(1, (j+1)%3);
    for (j=0;j<3;j++) msqi->seqv[j] = unbalance(0, (j+1)%3);
    msqi->imbnga = unbalance(1, 3);
    msqi->imbzroa = unbalance(1, 4);
    msqi->imbngv = unbalance(0, 3);
    msqi->imbzrov = unbalance(0, 4);
}

/*!
Description: 频率超限检查

    Output: 设置报警字
    Return: true=超限，false=未超限
*/
bool OtherFunc::AlarmCheckFreq(unsigned int &words, time_t time)
{
    int limit_low, limit_high;
    bool retval = false;

    if (!syspara_->limit_type) { //超限类型为国标
        limit_high = 5020;
        limit_low = 4980;
    } else {
        limit_high = 5000+syspara_->freq_limit[0];
        limit_low = 5000-syspara_->freq_limit[1];
    }

    if (frequency_[freq_idx_] < limit_low || frequency_[freq_idx_] > limit_high) {
        retval = true;
        words |= 0x1000;
    } else {
        words &= ~(0x1000);
    }

    //Send alarm to share memory
    unsigned short alm = 0;
    if (frequency_[freq_idx_] > limit_high) alm |= 1;
    else if (frequency_[freq_idx_] < limit_low) alm |= 0x2;
    shmem_func().SetAlm(kOverFreq, &alm, 2, time);

    return retval;
}

/*!
Description: Initailize Voltage diviation data.
*/
void OtherFunc::InitRms()
{
    memset(i_rms_, 0, sizeof(i_rms_));
    //memset(u_deviation_, 0, sizeof(u_deviation_));
    //Initialize maximum
    memset(voltdv_aggr_, 0, sizeof(VoltDvData<float>)*2/3);                 //set rms&rms-over to 0
    memset(&voltdv_aggr_[0].array[2], 0x45, sizeof(VoltDvData<float>)/3);   //set rms-under to 3156.329
    //Initialize minimum
    memset(&voltdv_aggr_[2], 0x45, sizeof(VoltDvData<float>)*2/3);  //set rms&rms-over to 3156.329
    memset(&voltdv_aggr_[2].array[2], 0, sizeof(VoltDvData<float>)/3);     //set rms-under to 0
    //Initialize average
    memset(&voltdv_aggr_[1], 0, sizeof(VoltDvData<float>));
    memset(i_rms_aggr_, 0, sizeof(i_rms_aggr_));
    memset(&i_rms_aggr_[2], 0x45, sizeof(float)*3);
    refresh_cnt_[kVoltdvSave] = 0;
    u_din_ = line_para_->PT2;
    if (!prmcfg->connect_type()) u_din_ /= 1.73205; //Y型接线
}

/*!
Description:设置dsp计算的有效值

    Input:  u_rms -- start address of voltage rms, Ua,Ub,Uc
            i_rms -- start address of current rms, Ia,Ib,Ic
*/
void OtherFunc::SetRms(unsigned short * u_rms, unsigned short * i_rms)
{
    int i, j;

    if (harmfunc->old_unit()) {
        units_[0] = units_[1] = 100;
    } else {
        units_[0] = harmfunc->units(0);
        units_[1] = harmfunc->units(1);
    }
    for (i=0;i<3;i++) {
        int n = unzip_int(u_rms[i]);
        n += harmfunc->line_coef(i);
        u_deviation_.array[0][i] = (float)n/units_[0];
        //i_rms_[i] = ((float)unzip_int(i_rms[i])-harmfunc->get_harm_data(HARM_AMP2, i, 1, 0))/units_[1];
        i_rms_[i] = harmfunc->i_rms(2, i);
    }
    harmfunc->CalcRmsppv(u_rms_ppv_, u_deviation_.array[0]); //Calculate phase to phase voltage

    for (i = 0; i < 3; i++) {   //Phase A,B,C
        u_deviation_.array[1][i] = u_deviation_.array[0][i] > u_din_ ? u_deviation_.array[0][i] : u_din_;
        u_deviation_.array[2][i] = u_deviation_.array[0][i] < u_din_ ? u_deviation_.array[0][i] : u_din_;
        for (j = 0; j < 3; j++) {   //rms,rms-over,rms-under
            voltdv_aggr_[1].array[j][i] += u_deviation_.array[j][i] * u_deviation_.array[j][i];
        }
        i_rms_aggr_[1][i] += i_rms_[i]*i_rms_[i];
        //Maximum
        if (voltdv_aggr_[0].array[0][i] < u_deviation_.array[0][i]) voltdv_aggr_[0].array[0][i] = u_deviation_.array[0][i];
        if (voltdv_aggr_[0].array[1][i] < u_deviation_.array[1][i]) voltdv_aggr_[0].array[1][i] = u_deviation_.array[1][i];
        if (voltdv_aggr_[0].array[2][i] > u_deviation_.array[2][i]) voltdv_aggr_[0].array[2][i] = u_deviation_.array[2][i];
        if (i_rms_aggr_[0][i] < i_rms_[i]) i_rms_aggr_[0][i] = i_rms_[i];
        //Minimum
        if (voltdv_aggr_[2].array[0][i] > u_deviation_.array[0][i]) voltdv_aggr_[2].array[0][i] = u_deviation_.array[0][i];
        if (voltdv_aggr_[2].array[1][i] > u_deviation_.array[1][i]) voltdv_aggr_[2].array[1][i] = u_deviation_.array[1][i];
        if (voltdv_aggr_[2].array[2][i] < u_deviation_.array[2][i]) voltdv_aggr_[2].array[2][i] = u_deviation_.array[2][i];
        if (i_rms_aggr_[2][i] > i_rms_[i]) i_rms_aggr_[2][i] = i_rms_[i];
    }
    refresh_cnt_[kVoltdvSave]++;
}

/*!
Description: Aggregate voltage deviation will be saved.
*/
void OtherFunc::AggregateVoltdv(time_t time, bool adj_tm)
{
    int i, j, k;
    float fi, fj;
    for (i = 0; i < 3; i++) {   //rms,rms-over,rms-under
        for (j=0; j<3; j++) {   //Phase A,B,C
            voltdv_aggr_[1].array[i][j] = sqrt(voltdv_aggr_[1].array[i][j]/refresh_cnt_[kVoltdvSave]);
        }
    }
    for (i = 0; i < 3; i++) {   //Phase A,B,C
        voltdv_svtmp_.data[0][i] = zip_int( voltdv_aggr_[1].array[0][i]* units_[0]);
        for (j = 1; j < 3; j++) {   //rms-over,rms-under
            if (syspara_->voltdv_save_type) { //电压偏差记录取平均值
                fi = voltdv_aggr_[1].array[j][i];
            } else { //电压偏差记录取最大值
                fi = voltdv_aggr_[0].array[j][i];
            }
            k = (fi - u_din_) * 10000 / u_din_;
            voltdv_svtmp_.data[j][i] = abs(k);
        }
    }
    
    //Write voltage deviation data to share memory
    fi = line_para_->CT1; fi /= line_para_->CT2;
    fj = line_para_->PT1; fj /= line_para_->PT2;
    for (i = 0; i < 3; i++) {   //max,avg,min
        for (j=0; j<3; j++) {   //Phase A,B,C
            if (i==1) { //average
                voltdv_[i][j] = (voltdv_aggr_[i].array[0][j]-u_din_)*100/u_din_;
                i_rms_aggr_[i][j] = sqrt(i_rms_aggr_[i][j]/refresh_cnt_[kVoltdvSave]);
            } else {    //max,min
#if 0   //max correspond to max pos deviation, min correspond to max neg deviation
                k = i==0?1:2;   //1=rms-over, 2=rms-under
                voltdv_[i][j] = (voltdv_aggr_[0].array[k][j]-u_din_)*100/u_din_;
#else   //max be max rms, min be min rms
                voltdv_[i][j] = (voltdv_aggr_[i].array[0][j]-u_din_)*100/u_din_;
#endif            
            }
            voltdv_aggr_[i].array[0][j] *= fj;
            i_rms_aggr_[i][j] *= fi;
        }
    }
    WriteStat2Shm(kVoltdvSave, time);
    voltdv_svtmp_.time = time;
    voltdv_svtmp_.q = quality_[kVoltdvSave];
    if(adj_tm) voltdv_svtmp_.q |= 1;
    voltdv_save_->push(&voltdv_svtmp_);
}

/*!
Description:Get save data

    Input:  type
    Output: pdata
    Return: 0=not adjust time, 1=need adjust time, -1=failure
*/
int OtherFunc::get_save_data(SaveFileType type, void *pdata)
{
    int retval = 0;
    //unsigned short q = quality_[type];
    switch (type) {
        case kPstSave:
            if (Pst_save_.q&1) {
                Pst_save_.q &= 0xfffe;
                retval = 1;
            }
            memcpy(pdata, &Pst_save_, sizeof(Pst_save_));
            break;
        case kFreqSave:
            retval = freq_save_->pop((FreqSaveModel*)pdata);
            if (((FreqSaveModel*)pdata)->q&1) {
                ((FreqSaveModel*)pdata)->q &= 0xfffe;
                retval = 1;
            }
            break;
        case kUnblcSave:
            retval = unblc_save_->pop((UnblncSaveModel*)pdata);
            if (((UnblncSaveModel*)pdata)->q&1) {
                ((UnblncSaveModel*)pdata)->q &= 0xfffe;
                retval = 1;
            }
            break;
        case kVoltdvSave:
            retval = voltdv_save_->pop((VoltDvSaveModel*)pdata);
            if (((VoltDvSaveModel*)pdata)->q&1) {
                ((VoltDvSaveModel*)pdata)->q &= 0xfffe;
                retval = 1;
            }
            break;
        default:
            retval = -1;
            break;
    }
    return retval;
}

/*!
Description:Get save data size

    Input:  type
            sr -- 0=for save, 1=for read
    Return: size of data in bytes. 0=failure
*/
int OtherFunc::get_save_size(SaveFileType type, int sr)
{
    int retval = 0;
    switch (type) {
        case kFreqSave:
            retval = sizeof(FreqSaveModel) + 2;
            break;
        case kUnblcSave:
            retval = sizeof(UnblncSaveModel) + 2;
            break;
        case kVoltdvSave:
            retval = sizeof(VoltDvSaveModel) + 2;
            break;
        case kPstSave:
            retval = sizeof(PstSaveModel) + 2;
            break;
        default:
            break;
    }
    return retval - sizeof(time_t);
}

/*!
Description:get primary unbalance data

    Input:  vc -- 0=voltage, 1=current
            type -- 0=zero, 1=positive, 2=negative, 3=unbalance, 4=zero unbalance
    Return: primary unbalance.unit:A,V,%
*/
float OtherFunc::unbalance(int vc, int type)
{
    if (vc > 1) vc = 1;
    if (type > 4) type = 4;
    float fi = unbalance_.array[vc][type];
    if (type < 3) {
        if (vc) {
            fi *= line_para_->CT1;
            fi /= line_para_->CT2;
            fi /= 1000;
        } else {
            fi *= line_para_->PT1;
            fi /= line_para_->PT2;
            fi /= 100;
        }
    } else {
        fi /= 100;
    }
    return fi;
}

void OtherFunc::SetFluct()
{
    unsigned short buf[6];
    int ofst = 0x538 * 2;
    read_daram (buf, sizeof(short) * 6, ofst);
    for (int i=0; i<3; i++) {
        float d = buf[i];
        d /= 100;
        if (d>fluct_max_[i]) fluct_max_[i] = d;
        fluct_cnt_[i] += buf[3+i];
    }
    //printf("fluc=%5.4f %5.4f %5.4f; flucf=%d %d %d\n", fluct_max_[0], fluct_max_[1], fluct_max_[2]
    //                                                , fluct_cnt_[0], fluct_cnt_[1], fluct_cnt_[2]);
}

