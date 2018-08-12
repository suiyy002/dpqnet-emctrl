#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include "comtrade_func.h"
#include "../IPC/shmem_srv.h"
#include "../base_func/time_cst.h"
#include "../base_func/utility.h"

const char * IEC61850RootPath = "/home/boyuu/save/data_sv/";  //61850文件传输服务的根目录
//extern const char * TranstFile;//暂态数据存储文件
ComtradeFunc *comtrade_func;

ComtradeFunc::ComtradeFunc(idx)
{
    ld_idx_ = idx;
    
    ratio_[0] = 0.01;
    ratio_[1] = 0.001;
    ptct_[0][0] = 10000;
    ptct_[0][1] = 100;
    ptct_[1][0] = 600;
    ptct_[1][1] = 5;
    cncttp_ = 0;
    sample_freq_ = 12800;
    memset(&trigtime_, 0, sizeof(timeval));
    //IniCfgPara(6, 0);
    f_strm_ = NULL;
    cfgpara_.achannel_param = NULL;
    cfgpara_.dchannel_param = NULL;
    
    strcpy(iedname_, "PQMonitor");
    sprintf(ldname_, "LD%d", idx+1);
    going_ = 0;
    flt_num_ = -1;
    rdre_soe_ = new LoopBuffer<PqmRdre>(8);
    pthread_mutex_init (&mutex_,NULL);
}

ComtradeFunc::~ComtradeFunc()
{
    pthread_mutex_destroy(&mutex_);
    delete rdre_soe_;
    int i;
    if (cfgpara_.achannel_param) {
        for (i=0; i<cfgpara_.nA; i++) {
            delete cfgpara_.achannel_param[i];
        }
        delete [] cfgpara_.achannel_param;
    }
    
    if (cfgpara_.dchannel_param) {
        for (i=0; i<cfgpara_.nD; i++) {
            delete cfgpara_.dchannel_param[i];
        }
        delete [] cfgpara_.dchannel_param;
    }
}

/*!
Initirlize parameter of cfg file

    Input:  nA -- number of analog channel. [0-1]:voltage/current
            nD -- number of digital channel
*/
void ComtradeFunc::IniCfgPara(int *nA, int nD)
{
    strcpy(cfgpara_.rev_year,"2001");

    cfgpara_.nA = nA[0]+nA[1]; cfgpara_.nD = nD; cfgpara_.TT = cfgpara_.nA + nD; 
    cfgpara_.achannel_param = new SAChannelPara*[cfgpara_.nA];
    ratio_[0] = 0.01;
    ratio_[1] = 0.001;
    int cnct = cncttp_*3;
    int m = 0;
    for (int vc=0; vc<2; vc++) {
        if (vc) cnct = 6;
        for (int i=0; i<nA[vc]; i++, m++) {
            int j = i/3;
            cfgpara_.achannel_param[m] = new SAChannelPara;
            cfgpara_.achannel_param[m]->An = m+1;
            strcpy(cfgpara_.achannel_param[m]->ch_id, UI[vc]);
            strcpy(cfgpara_.achannel_param[m]->ph, ABC[i+cnct]);
            strcpy(cfgpara_.achannel_param[m]->ccbm, "");
            strcpy(cfgpara_.achannel_param[m]->uu, UNIT[vc]);
            cfgpara_.achannel_param[m]->a = ratio_[vc];
            cfgpara_.achannel_param[m]->b = 0;
            cfgpara_.achannel_param[m]->skew = 0;
            cfgpara_.achannel_param[m]->min = -32000;
            cfgpara_.achannel_param[m]->max = 32000;
            cfgpara_.achannel_param[m]->primary = ptct_[j][0];
            cfgpara_.achannel_param[m]->secondary = ptct_[j][1];
            cfgpara_.achannel_param[m]->PS = 's';
        }    
    }

    cfgpara_.dchannel_param = new SDChannelPara*[nD];
    for (int i=0; i<nD; i++) {
        cfgpara_.dchannel_param[i] = new SDChannelPara;
        cfgpara_.dchannel_param[i]->Dn = i+1;
        strcpy(cfgpara_.dchannel_param[i]->ch_id, "");
        strcpy(cfgpara_.dchannel_param[i]->ph, "");
        strcpy(cfgpara_.dchannel_param[i]->ccbm, "");
        cfgpara_.dchannel_param[i]->y=1;
    }

    cfgpara_.lf=50;
    cfgpara_.nrates=1;
    cfgpara_.samp=sample_freq_;
    cfgpara_.endsamp=1;
    strcpy(cfgpara_.ft,"binary");
    cfgpara_.timemult = 1;
    cfgpara_.tz_data[0] = 0; cfgpara_.tz_data[1] = 0;
    cfgpara_.tz_recorder[0] = TimeZone(); cfgpara_.tz_recorder[1] = 0;
    cfgpara_.tmq = 'A'; cfgpara_.leap = 0;
}

/*!
Description:Refresh relevant infomation
Input:      cnt -- count of sampling point
*/
void ComtradeFunc::RefreshInfo(int cnt)
{
    cfgpara_.endsamp = cnt;
    sprintf(cfgpara_.station_name, "%s %s", shmem_srv().station_name(), shmem_srv().monitor_point());
    int i, j;
    for (i=0; i<cfgpara_.nA; i++) {
        j = i/3;
        //strcpy(cfgpara_.achannel_param[i]->ccbm, ??);     //暂不实现，事件类型需与通道一一对应
        cfgpara_.achannel_param[i]->primary = ptct_[j][0];
        cfgpara_.achannel_param[i]->secondary = ptct_[j][1];
    }
    cfgpara_.samp = sample_freq_;

    tm tmi; char tm_str[24];
    GmTime(&tmi, &starttime_.tv_sec);
    strftime(tm_str, 20, "%d/%m/%Y,%H:%M:%S", &tmi);
    sprintf(cfgpara_.starttm, "%s.%06d", tm_str, starttime_.tv_usec);
    
    GmTime(&tmi, &trigtime_.tv_sec);
    strftime(tm_str, 20, "%d/%m/%Y,%H:%M:%S", &tmi);
    sprintf(cfgpara_.trigtm, "%s.%06d", tm_str, trigtime_.tv_usec);
}

/*!
Save comtrade config file

    Input:  name -- COMTRADE file name
            cause -- event cause
            type -- event type
    Return: 0=success, 1=failure
*/
int ComtradeFunc::SaveCfgFile(char *name, const char *cause, const char *type)
{
    char filename[128];
    sprintf(filename, "%s.cfg", name);
    FILE *fd = fopen(filename, "w");
    if(fd == NULL)  return 1;

    fprintf(fd, "%s,%s,%s\r\n", cfgpara_.station_name, cfgpara_.rec_dev_id, cfgpara_.rev_year);
    fprintf(fd, "%d,%dA,%dD\r\n", cfgpara_.TT, cfgpara_.nA, cfgpara_.nD);
    strncpy(cfgpara_.achannel_param[0]->ccbm, cause, 6);
    strncpy(cfgpara_.achannel_param[1]->ccbm, type, 6);
    for (int i=0;i<cfgpara_.nA;i++) {
        fprintf(fd, "%d,%s%d,%s,%s,%s,%4.3f,%4.3f,%4.2f,%d,%d,%f,%f,%c\r\n", cfgpara_.achannel_param[i]->An,
                cfgpara_.achannel_param[i]->ch_id, i%3+1, cfgpara_.achannel_param[i]->ph,
                cfgpara_.achannel_param[i]->ccbm, cfgpara_.achannel_param[i]->uu,
                cfgpara_.achannel_param[i]->a, cfgpara_.achannel_param[i]->b,
                cfgpara_.achannel_param[i]->skew, cfgpara_.achannel_param[i]->min,
                cfgpara_.achannel_param[i]->max, cfgpara_.achannel_param[i]->primary,
                cfgpara_.achannel_param[i]->secondary, cfgpara_.achannel_param[i]->PS );
    }
    for (int i=0;i<cfgpara_.nD;i++) {
        fprintf(fd, "%d,%s,%s,%s,%d\r\n", cfgpara_.dchannel_param[i]->Dn,
                cfgpara_.dchannel_param[i]->ch_id, cfgpara_.dchannel_param[i]->ph, 
                cfgpara_.dchannel_param[i]->ccbm, cfgpara_.dchannel_param[i]->y);
    }
    fprintf(fd, "%2.0f\r\n", cfgpara_.lf);  //Line frequency
    fprintf(fd, "%d\r\n", cfgpara_.nrates);
    fprintf(fd, "%4.3f,%d\r\n", cfgpara_.samp, cfgpara_.endsamp);
    fprintf(fd, "%s\r\n", cfgpara_.starttm); 
    fprintf(fd, "%s\r\n", cfgpara_.trigtm); 
    fprintf(fd, "%s\r\n",cfgpara_.ft);   //file type
    fprintf(fd, "%5.3f\r\n", cfgpara_.timemult);
    fprintf(fd, "%d,%d\r\n", cfgpara_.tz_data[0], cfgpara_.tz_recorder[0]);
    fprintf(fd, "%c,%d\r\n", cfgpara_.tmq, cfgpara_.leap);

    fclose(fd);
    return 0;
}

/*！
Description:Save comtrade information file

    Input:  name -- COMTRADE file name
    Return: 0=success, 1=failure
*/
int ComtradeFunc::SaveInfFile(char *name)
{
    char filename[128];
    sprintf(filename, "%s.inf", name);
    FILE *fd = fopen(filename, "w");
    if(fd == NULL)  return 1;

    fprintf(fd, "[Public Record_Information]\r\n");
    fprintf(fd, "type=%s\r\n", cfgpara_.achannel_param[1]->ccbm);
    float pt2 = ptct_[0][1];
    if (cncttp_==0) pt2 /= sqrt(3.0);
    //printf("pt2=%6.3f,extre_=%6.3f\n", pt2, extre_);
    if (extre_>pt2) {
        fprintf(fd, "max_voltage=%6.3f\r\n", extre_);
        fprintf(fd, "min_voltage=null\r\n");
    } else {
        fprintf(fd, "max_voltage=null\r\n");
        fprintf(fd, "min_voltage=%6.3f\r\n", extre_);
    }
    fprintf(fd, "phase=%s\r\n", ABC[6+trig_phs_]);
    SaveInfPseudo(fd, pt2);
    fclose(fd);
    return 0;
}

/*!
add user's pseudo requirement into info file
*/
void ComtradeFunc::SaveInfPseudo(FILE *fp, float pt2)
{
    float trigger_deep = fabs(extre_-pt2);
    float trigger_range = trigger_deep*100 / pt2;
    fprintf(fp, "trigger_range=%6.3f\r\n", trigger_range);
    fprintf(fp, "trigger_duration=%6.4f\r\n", dur_);
    fprintf(fp, "[StateGrid event_inf]\r\n");
    fprintf(fp, "trigger_deep=%6.3f\r\n", trigger_deep);
    fprintf(fp, "trigger_range=%6.3f\r\n", trigger_range);
    fprintf(fp, "trigger_duration=%6.4f\r\n", dur_);
}

/*!
Initialize save comtrade data file

    Input:  rtime -- record start time. time of 1st sv be recorded.
            ttime -- trigger time. event start time.
            achl -- analog channel state. [0-1]:voltage/current. 0=no use.
    Return:     0=success, 1=failure
*/
int ComtradeFunc::IniSaveData(struct timeval *rtime, struct timeval *ttime, uint8_t *achl)
{
    int vc[2];
    memset(vc, 0, sizeof(vc));
    if (achl[0]) vc[0]=3;
    if (achl[1]) vc[1]=3;
    IniCfgPara(vc, 0);
    
    memcpy(&starttime_, rtime, sizeof(timeval));
    memcpy(&trigtime_, ttime, sizeof(timeval));
    PushRdre(0);
    
    if (strlen(shmem_srv().ld_name(ld_idx_))>0) strcpy(ldname_, shmem_srv().ld_name(ld_idx_));
    if (strlen(shmem_srv().ied_name())>0) strcpy(iedname_, shmem_srv().ied_name());
    cfgpara_.tz_recorder[0] = TimeZone();
    
    char stime[50];  tm tmi;
    GmTime(&tmi, &trigtime_.tv_sec);
    int n = strftime(stime, 20, "%Y%m%d_%H%M%S", &tmi);
    sprintf(&stime[n], "_%03i", (trigtime_.tv_usec +500) / 1000);

    sprintf(comtrade_path_, "%s%s%s/%s/", IEC61850RootPath, iedname_, ldname_, "COMTRADE");
    if (AssertPath(comtrade_path_)) return 1;
    sprintf(filename_, "%s%s_%s_%03d_%s", comtrade_path_, iedname_, ldname_, flt_num_, stime);
    printf("%s\n", filename_);

    sn_=0;
    timestamp_ = 0;
    intrvl_ = 1000000/sample_freq_;

    // ~~~~~~~~~ Remove old file ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    char rm_cmd[128];
    uint8_t rm_num = flt_num_;
    rm_num -= dre_tol_;
    sprintf(rm_cmd, "rm %s%s_%s_%03d*", comtrade_path_, iedname_, ldname_, rm_num);
    system(rm_cmd);
    printf("%s\n", rm_cmd);
    
    if (f_strm_) fclose(f_strm_);
    char fname[128];
    sprintf(fname, "%s/temp_dfile", comtrade_path_);
    f_strm_ = fopen(fname, "wb");
    if(f_strm_ == NULL)  return 1;
    flt_num_++;
    flt_num_ &= 0xff;
}

/*!
Save comtrade data file

Input:      cnt -- count of data buffer
            pbuf_u -- voltage data buffer
            pbuf_i -- current data buffer
Return:     0=success, 1=failure
*/
int ComtradeFunc::SaveDataFile(int cnt, uint8_t *pbuf_u, uint8_t *pbuf_i)
{
    if(f_strm_ == NULL) {
        printf("3b. !!!!!!!error SaveDataFile... f_strm_ is null  \n");
        return 1;
    }
        
    uint32_t tmstmp;
    for (int i=0; i<cnt; i++,sn_++) {
        tmstmp = (uint32_t)(timestamp_+0.5);
        fwrite(&sn_, 1, sizeof(unsigned int), f_strm_);
        fwrite(&tmstmp, 1, sizeof(unsigned int), f_strm_);
        if (pbuf_u) {
            fwrite(pbuf_u, 1, 6, f_strm_);
            pbuf_u += 6;
        }
        if (pbuf_i) {
            fwrite(pbuf_i, 1, 6, f_strm_);
            pbuf_i += 6;
        }
        timestamp_ += intrvl_;
    }
    return 0;
}

/*!
Save & Close comtrade file
    
    Input:  device_sn -- device serial number
            cause -- event cause
            type -- event type
            valid -- event if valid. 0=valid
    Return:     0=success, 1=failure
    Called by:  VoltVariation::save_transt_data
*/
int ComtradeFunc::EndSave(const char * cause, const char *type, int valid)
{
    int retval = 0;
    RefreshInfo(sn_);
    if (f_strm_) {
        fclose(f_strm_);
        f_strm_ = NULL;
    }
    char syscmd[128];
    if (valid!=0) {
        sprintf(syscmd, "rm %s/temp_dfile", comtrade_path_);
    } else {
        // ~~~~~~~~~ Save comtrade cfg file ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        retval = SaveCfgFile(filename_, cause, type);
        // ~~~~~~~~~ Save comtrade inf file ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        retval = SaveInfFile(filename_);
        
        sprintf(syscmd, "mv %s/temp_dfile %s.dat", comtrade_path_, filename_);
        PushRdre(1);
    }
    printf("%s\n", syscmd);
    system(syscmd);
    return retval;
}

/*!
清空Comtrade 记录文件

    Called by:  thread_save
*/
void ComtradeFunc::ClearFile()
{
    if (prmcfg->cmtrd_sv_path()) {
        sprintf(comtrade_path_, "%s%s%s/%s/", IEC61850RootPath, iedname_, ldname_, "COMTRADE");
    } else {
        sprintf(comtrade_path_, "%s/%s/", IEC61850RootPath, "COMTRADE");
    }

    char cmd[128];
    sprintf(cmd, "rm %s*", comtrade_path_);
    system(cmd);
}

/*!
Set disturbance record attribute

    Input:  tol -- total number of disturbance record
            flt -- fault number
*/
void ComtradeFunc::SetDreAttr(uint8_t tol, uint8_t fltn)
{
    if (flt_num_<0) flt_num_ = fltn;
    if (dre_tol_ != tol) {
        if (tol < dre_tol_) {
            ClearFile();
            flt_num_ = 0;
        }
        dre_tol_ = tol;
    }
}

/*!
Push rdre data into rdre_soe_

    Input:  type -- 0=start, 1=end
*/
void ComtradeFunc::PushRdre(int type)
{
    PqmRdre rdre;
    memset(&rdre, 0, sizeof(rdre));
    
    rdre.happen = 1;
    rdre.fltnum = flt_num_;
    //rdre.rcdtrg = 1;
    //rdre.trig_time = trigtime_
    if (!type) {    //start
        rdre.rcdmade = 0;
        rdre.time = starttime_;
    } else {        //end
        rdre.rcdmade = 1;
        gettimeofday(&rdre.time, NULL);
    }
    
    pthread_mutex_lock(&mutex_);
    mutex__soe_->push(&rdre);
    pthread_mutex_unlock(&mutex_);
}

/*!
Refresh RDRE data to share memory
*/
void ComtradeFunc::Rdre2Shm()
{
    if (!rdre_soe_->data_num()) return;
    if (shmem_func().rdre_happen(ld_idx_)) return;
    
    PqmRdre rdre;
    pthread_mutex_lock(&mutex_);
    rdre_soe_->pop(&rdre);
    pthread_mutex_unlock(&mutex_);

    shmem_func().ShmemCpy(kRdreSOE, &rdre, ld_idx_);
    shmem_func().IncDataUp(1, ld_idx_);
    shmem_func().set_rdre_ok(ld_idx_);
}




