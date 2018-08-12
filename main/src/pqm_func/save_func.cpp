#include "save_func.h"
#include "harmfunc.h"
#include "other_func.h"
#include "../base_func/utility.h"
#include "../GUI/form/mainform.h"
#include "../thread/pthread_mng.h"
#include "../IPC/shmemfunc.h"
#include <cmath>
using namespace std;

static const char * Hrm10Path = "save/cyc10/"; //10周波测试数据存储路径

static const char * Power01File = "save/power01_time.sav";   //开关机时间数据存储文件
static const char * kSaveDir[] = {     //record data save directory
    "save/steady_sv/Pst/",
    "save/steady_sv/frequency/",
    "save/steady_sv/unbalance/",
    "save/steady_sv/volt_dv/"    //voltage diviation
};
static const char * kSaveFileExt[] = {".Pst", ".frq", ".blc", ".vdv"};  //record data save file extension
static const int MaxSaveDay = 62;    //record be saved maximum day

SaveFunc *save_func;

SaveFunc::SaveFunc()
{
    syspara = prmcfg->syspara();
    sqlite_db = new SQLiteDB("/boyuu/save/steady_sv", "harmonic", prmcfg->harmrec_svmax()); 

    memset(f_rcd_, 0, sizeof(f_rcd_));
    memset(psave_status_, 0, sizeof(psave_status_));
    for (int i = 0; i <= kVoltdvSave; i++) {
        LoadStatus((SaveFileType)i);
        last_time_[i] = 0;
        next_time_[i] = 0;
    }
    harm_save_delay_ = 20;
    pst_save_delay_ = 600;
}

SaveFunc::~SaveFunc()
{
    for (int i = 0; i <= kVoltdvSave; i++) {
        if (psave_status_[i]) delete psave_status_[i];
        if (f_rcd_[i]) fclose(f_rcd_[i]);
    }
    delete sqlite_db;
}

void SaveFunc::ClearRecData()
{
    for (int i = 0; i <= kVoltdvSave; i++) {
        if (f_rcd_[i]) {
            fclose(f_rcd_[i]);
            f_rcd_[i] = NULL;
        }
    }
    system("save/clear_save");
    for (int i = 0; i <= kVoltdvSave; i++) {
        LoadStatus((SaveFileType)i);
    }
}

static int tableExistedCallback(void *custom, int num, char **col1, char **col2)
{
    *(char*)custom = 1;
    return 0;
}

/*!
Description: 每半秒处理例程。目前处理数据:谐波、闪变、频率、不平衡度、电压偏差的保存
*/
void SaveFunc::SaveHandle(void)
{
    unsigned int alarm_word2; //no use
    int savetype[5]; //savetype,0-4=harm,freq, unbalance, voltage deviation, pst

    time_t timei;
    time(&timei);
    savetype[1] = DetectSaveRcd(timei, kFreqSave);
    savetype[2] = DetectSaveRcd(timei, kUnblcSave);
    savetype[3] = DetectSaveRcd(timei, kVoltdvSave);
    savetype[4] = DetectSaveRcd(timei, kPstSave);
    savetype[0] = DetectSaveHarm(timei);

    if (!pst_save_delay_) {
        if (savetype[4]) notice_pthread(kTTSave, SAVEINFO, savetype[4], NULL);
    } else {
        pst_save_delay_--;
    }
    if (!harm_save_delay_) { //开机延时结束
        for (int i = 3; i >= 0; i--) {
            if (savetype[i]) notice_pthread(kTTSave, SAVEINFO, savetype[i], NULL);
        }
    } else {
        harm_save_delay_--;
    }
}

/*#define CHG_TIME_FRAME(space, last_tm, next_tm) \
    last_tm = time;\
    tm * ptm = localtime(&time);\
    int k = (ptm->tm_min*60 + ptm->tm_sec)%space;\
    if (k) last_tm -= k;\
    next_tm = last_tm + space;
*/
/*!
Description:检测是否需要保存谐波数据

    Input: time -- 时间
    Return: save type. refer to pthread_mng.h for detail. If not save return 0.
*/
int SaveFunc::DetectSaveHarm(const time_t time)
{
    static long last_time = 0;
    static long next_time = 0;

    int retval = 0;
    if (time < last_time || time >= next_time) {
        if (time == next_time) { //It's time to save record
            harmfunc->AggregateData(time);  //谐波数据聚合处理
            harmfunc->get_thd_rcd(thd_buf_);    //Read thd to thd_buf_
            hrm_save_time_ = time;
            retval = kHarmRcd;
        } else {    //Current time is out of range, because time setting etc

        }
        ChgTimeFrame(time, syspara->harm_rcd_space, &last_time, &next_time);  //更改数据存储的时间片
        harmfunc->InitAggregation();
    }
    return retval;
}

/*!
Description:检测是否需要保存记录数据

    Input:  time -- 时间
            type -- Save file type,for detail see SaveFileType
    Return: save type. refer to pthread_mng.h for detail. If not save return 0.
*/
int SaveFunc::DetectSaveRcd(const time_t time, SaveFileType type)
{
    int retval = 0;
    int savetype, savespc;
    switch (type) {
        case kPstSave:
            savetype = kPstRcd;
            savespc = 600;
            break;
        case kFreqSave:
            savetype = kFreqRcd;
            savespc = syspara->freq_rcd_space;
            break;
        case kUnblcSave:
            savetype = kUnblcRcd;
            savespc = syspara->unbalance_rcd_space;
            break;
        case kVoltdvSave:
            savetype = kVoltdvRcd;
            savespc = syspara->voltdv_rcd_space;
        default:
            break;
    }
    if (time < last_time_[type] || time >= next_time_[type]) {
        if (time == next_time_[type]) { //It's time to save record
            other_func->Aggregate(type, time, adj_time_[type]);  //数据聚合处理
            adj_time_[type] = false;
            retval = savetype;
        } else {    //Current time is out of range, because time setting etc
            adj_time_[type] = true;
        }
        //CHG_TIME_FRAME(savespc, last_time_[type], next_time_[type])  //更改数据存储的时间片
        ChgTimeFrame(time, savespc, &last_time_[type], &next_time_[type]);  //更改数据存储的时间片
        other_func->InitData(type);
    }
    return retval;
}

/*!
Description:Save hamonic data
*/
void SaveFunc::SaveHarmonic()
{
    unsigned short huam_buf[3][MAX_HARM_NUM + 1];
    unsigned short hiam_buf[3][MAX_HARM_NUM + 1];
    harmfunc->get_harm_rcd(0, huam_buf);
    harmfunc->get_harm_rcd(1, hiam_buf);
    main_form->PushHRu(huam_buf);   //Push HRu(a) for r-a function

    int i, j, n;
    int inthmcnt;
    unsigned char * pinthm = harmfunc->make_inter_struct(inthmcnt);
    //printf("inthmcnt=%d\n", inthmcnt);
    int len = 1320/2+inthmcnt/2+1;
    unsigned short *save_buf = new unsigned short[len];
    memset (save_buf, 0, sizeof(short)*len);
    unsigned short * phuphs = harmfunc->huphs();
    unsigned short * phiphs = harmfunc->hiphs();
    n = 0;
    //---------- 0~25次谐波 -----------------------------------------------------
    for (j = 0; j < 3; j++) {
        for (i = 0; i < 26; i++) {
            save_buf[n] = huam_buf[j][i];
            save_buf[n + 1] = phuphs[j * (MAX_HARM_NUM + 1) + i];
            save_buf[n + 2] = hiam_buf[j][i];
            save_buf[n + 3] = phiphs[j * (MAX_HARM_NUM + 1) + i];
            n += 4;
        }
    }
    memcpy(&save_buf[n], &hrm_save_time_, sizeof(time_t));

    save_buf[n + 3] = other_func->frequency();
    save_buf[n + 4] = harmfunc->harm_quality();
    
    unsigned char ci;
    ci = prmcfg->pqm_type();
    SetOctecInShort(save_buf, 634, ci);      //Set equipment model
    //Set units type
    SetOctecInShort(save_buf, 635, harmfunc->units_type(0) | (harmfunc->units_type(1) << 2) | (harmfunc->units_type(2) << 4));
    SetOctecInShort(save_buf, 639, 0x49);    //Set extend data enable
    memcpy(&save_buf[320], thd_buf_, 6 * sizeof(short)); //save THD to 640~651
    //---------- 扩展谐波 -----------------------------------------------------
    n = 720/2;
    if (harm_valid_ & 0x1) { //扩展谐波有效
        SetOctecInShort(save_buf, 638, 0x66);
        for (j = 0; j < 3; j++) {
            for (i = 0; i < 25; i++) {
                save_buf[n] = huam_buf[j][26 + i];
                save_buf[n + 1] = phuphs[j * (MAX_HARM_NUM + 1) + 26 + i];
                save_buf[n + 2] = hiam_buf[j][26 + i];
                save_buf[n + 3] = phiphs[j * (MAX_HARM_NUM + 1) + 26 + i];
                n += 4;
            }
        }
    }

    //---------- 间谐波 -------------------------------------------------------
    if (harm_valid_ & 0x2 && inthmcnt) { //间谐波有效
        SetOctecInShort(save_buf, 637, 0x66);
        memcpy(&save_buf[660], pinthm, inthmcnt);
    }
    if (prmcfg->harmrec_sven()) {
        sqlite_db->InsertDB((unsigned char*)save_buf, len*2, hrm_save_time_, harmfunc->harm_quality(), prmcfg->harmrec_svmax());
    }
    delete [] save_buf;
}

#define SAVE_OCLOCK(fstream, oclock, d_sz) \
        fseek(fstream, -(d_sz+sizeof(short))*oclock.count-sizeof(OClockRcd), SEEK_CUR); \
        fwrite(&oclock, sizeof(OClockRcd), 1, fstream); \
        fseek(fstream, (d_sz+sizeof(short))*oclock.count, SEEK_CUR);

#define SAVE_RCDHEAD(fstream, head) \
        long pos = ftell(fstream); \
        fseek(fstream, 0, SEEK_SET); \
        fwrite(&head, sizeof(RcdFileHead), 1, fstream); \
        fseek(fstream, pos, SEEK_SET);

/*!
Description:Save record data
*/
void SaveFunc::SaveRecData(SaveFileType type)
{
    tm tmi;
    int ret = other_func->get_save_data(type, save_buf_);
    if (ret<0) {
        printf("Get save data error!\n");
        return;
    }
    //GmTime(&tmi, &save_time_[type]);
    GmTime(&tmi, (time_t*)save_buf_);
    //printf("%s:typ=%d %d:%d:%d\n", __FUNCTION__, type, tmi.tm_hour, tmi.tm_min, tmi.tm_sec);
    long date = (tmi.tm_year+1900) * 10000 + (tmi.tm_mon+1) * 100 + tmi.tm_mday;
    int data_sz = other_func->get_save_size(type, 0);
    if (f_rcd_[type] == NULL) {
        if (!OpenRecFile(type, &tmi)) return;
    } else {
        if (date != rcd_file_head_[type].date) {    //date is changed, new record file will be opened
            SAVE_OCLOCK(f_rcd_[type], oclock_rcd_[type], data_sz)        //Save OClockRcd
            fclose(f_rcd_[type]);
            if (psave_status_[type]->Insert(&date)) { //If buffer be overflow,delete oldest record file
                long oldest;
                psave_status_[type]->get_trash(&oldest);
                char file_name[128];
                sprintf(file_name, "%s%d%s", kSaveDir[type], oldest, kSaveFileExt[type]);
                printf("remove oldest file:%s\n", file_name);
                remove(file_name);
            }
            if (!OpenRecFile(type, &tmi)) return;
        } else if (ret==1) {  //need adjust time
            SAVE_OCLOCK(f_rcd_[type], oclock_rcd_[type], data_sz)       //Save OClockRcd
            printf("tmi.tm_hour=%d\n", tmi.tm_hour);
            LocateSave(f_rcd_[type], &tmi, &rcd_file_head_[type], &oclock_rcd_[type], data_sz, GetUnitVal(type));
            SAVE_RCDHEAD(f_rcd_[type], rcd_file_head_[type])    //Save RcdFileHead
        }
    }
    if (tmi.tm_hour != oclock_rcd_[type].oclock) { //hour is changed, new oclock record will be created
        SAVE_OCLOCK(f_rcd_[type], oclock_rcd_[type], data_sz)
        oclock_rcd_[type].oclock = tmi.tm_hour;
        oclock_rcd_[type].unit = GetUnitVal(type);
        oclock_rcd_[type].count = 0;
        fwrite(&oclock_rcd_[type], sizeof(OClockRcd), 1, f_rcd_[type]);
        rcd_file_head_[type].count++;
        SAVE_RCDHEAD(f_rcd_[type], rcd_file_head_[type])    //Save RcdFileHead
        //fflush(f_rcd_[type]);
    } else if (!tmi.tm_sec) { // && save_delay_!=flush_cnt) { // Flush per minute
        //save_delay_ = flush_cnt;
        SAVE_OCLOCK(f_rcd_[type], oclock_rcd_[type], data_sz)
        //fflush(f_rcd_[type]);
    }

    short secs = tmi.tm_min * 60 + tmi.tm_sec;
    fwrite(&secs, sizeof(short), 1, f_rcd_[type]);
    fwrite(save_buf_+2, data_sz, 1, f_rcd_[type]);
    oclock_rcd_[type].count++;
}

/*!
Description:Open record file

    Input:  type -- Save file type,for detail see SaveFileType
            tim -- time of current record
    Return: true=success, false=failure
*/
bool SaveFunc::OpenRecFile(SaveFileType type, tm *tim)
{
    int i;
    long date = (tim->tm_year+1900) * 10000 + (tim->tm_mon+1) * 100 + tim->tm_mday;
    int data_sz = other_func->get_save_size(type, 0);
    char file_name[128];
    sprintf(file_name, "%s%d%s", kSaveDir[type], date, kSaveFileExt[type]);
    printf("file_name=%s\n", file_name);
    f_rcd_[type] = fopen(file_name, "rb+");
    if (f_rcd_[type]) { //Open exist record file
        fread(&rcd_file_head_[type], sizeof(RcdFileHead), 1, f_rcd_[type]);
        if (rcd_file_head_[type].date != date) {    //The record file is invalid
            fclose(f_rcd_[type]);
            remove(file_name);
            f_rcd_[type] = NULL;
        } else {
            LocateSave(f_rcd_[type], tim, &rcd_file_head_[type], &oclock_rcd_[type], data_sz, GetUnitVal(type));
            SAVE_RCDHEAD(f_rcd_[type], rcd_file_head_[type])    //Save RcdFileHead
            //fflush(f_rcd_[type]);
        }
    }
    if (!f_rcd_[type]) {    //If record file not opened, create new record file
        f_rcd_[type] = fopen(file_name, "wb+");
        if (!f_rcd_[type]) {
            printf("Create %s failure\n", file_name);
            return false;
        }
        rcd_file_head_[type].date = date;
        rcd_file_head_[type].count = 0;
        fwrite(&rcd_file_head_[type], sizeof(RcdFileHead), 1, f_rcd_[type]);
        oclock_rcd_[type].oclock = tim->tm_hour;
        oclock_rcd_[type].unit = GetUnitVal(type);
        oclock_rcd_[type].count = 0;
        fwrite(&oclock_rcd_[type], sizeof(OClockRcd), 1, f_rcd_[type]);
        rcd_file_head_[type].count++;
        SAVE_RCDHEAD(f_rcd_[type], rcd_file_head_[type])    //Save RcdFileHead
        SaveStatus(type, date);
    }
    return true;
}

/*!
Description:Load record status from file

    Input:  type -- Save file type,for detail see SaveFileType
*/
void SaveFunc::LoadStatus(SaveFileType type)
{
    char file_name[128];
    sprintf(file_name, "%sstatus%s", kSaveDir[type], kSaveFileExt[type]);
    if (psave_status_[type]) delete psave_status_[type];
    psave_status_[type] = new LoopBufSort<long>(MaxSaveDay, CompareInt, file_name);
}

/*!
Description:Save record status to file

    Input:  type -- Save file type,for detail see SaveFileType
    Return: true=success, false=failure
*/
void SaveFunc::SaveStatus(SaveFileType type, long date)
{
    char file_name[128];
    if (psave_status_[type]->Insert(&date)) { //If buffer be overflow,delete oldest record file
        long oldest;
        psave_status_[type]->get_trash(&oldest);
        sprintf(file_name, "%s%d%s", kSaveDir[type], oldest, kSaveFileExt[type]);
        printf("remove oldest file:%s\n", file_name);
        remove(file_name);
    }
    sprintf(file_name, "%sstatus%s", kSaveDir[type], kSaveFileExt[type]);
    psave_status_[type]->SaveFile(file_name);
}

inline int SaveFunc::GetUnitVal(SaveFileType type)
{
    int retval = 0;
    if (type == kFreqSave) {
        retval = harmfunc->units_type(2) << 4;
    } else if (type == kUnblcSave) {
        retval = 0 +  (1<< 2);  //voltage:1/100; current:1/1000
    } else if (type == kVoltdvSave) {
        retval = harmfunc->units_type(0);
    } else if (type == kPstSave) {
        retval = 1;
    }
    return retval;
}

/*!
Description:read record from save file

    Input:      stime -- start time of record(exclude the time)
                etime -- end time of record(include the time)
                max_num -- maximum number be read. 0=default number
                type -- data type. refer SaveFileType for detail
    Output:     buf -- record save buffer
    Return:     number of record
*/
int SaveFunc::ReadRecData(char * buf, time_t* stime, time_t* etime, short max_num, SaveFileType type)
{
    char filename[128];
    if (!max_num) max_num = 255;
    int num = 0;
    char *pbuf = buf;
    buf += 3;
    OClockRcd oclk, oclk2;

    time_t timei = *stime + 1;
    tm tmi;
    GmTime(&tmi, &timei);
    //LocalTime(&tmi, &timei);
    long date, date1 = (tmi.tm_year + 1900)*10000 + (tmi.tm_mon + 1)*100 + tmi.tm_mday;
    int ofst = psave_status_[type]->Match(&date1, -1);
    if (ofst < 0) { //not found
        return 0;
    } else {
        RcdFileHead rcd_hd;
        short secs;
        int data_sz = other_func->get_save_size(type, 0);
        psave_status_[type]->seek(ofst);
        while (num < max_num) {
            if (psave_status_[type]->read(&date) < 0) break;
            sprintf(filename, "%s%d%s", kSaveDir[type], date, kSaveFileExt[type]);
            //printf("filename=%s\n", filename);
            FILE *f_strm = fopen(filename, "rb"); //Open exist record file
            if (f_strm) {
                //printf("opened %s\n", filename);
                if (fread(&rcd_hd, sizeof(RcdFileHead), 1, f_strm) == 1) {
                    //printf("rcd_hd.date=%d date=%d date1=%d\n", rcd_hd.date, date, date1);
                    if (rcd_hd.date == date) { //The record file is valid
                        if (date != date1) {
                            tmi.tm_year = date/10000-1900;
                            tmi.tm_mon = (date%10000)/100 - 1;
                            tmi.tm_mday = date%100;
                            tmi.tm_hour = 0;
                            tmi.tm_min = 0;
                            tmi.tm_sec = 0;
                        }
                        
                        if (LocateRead(f_strm, &tmi, &rcd_hd, &oclk, data_sz)) {
                            *pbuf = oclk.unit;
                            //printf("rcd_hd.count=%d, tmi.tm_mday=%d, oclk.unit=%x\n", rcd_hd.count, tmi.tm_mday, oclk.unit);
                            while(rcd_hd.count-- > 0 && num < max_num) {
                                //printf("oclk.count=%d\n", oclk.count);
                                while (oclk.count-- > 0 && num < max_num) {
                                    if (fread(&secs, sizeof(short), 1, f_strm) < 1) break;
                                    tmi.tm_hour = oclk.oclock;
                                    tmi.tm_min = secs / 60;
                                    tmi.tm_sec = secs % 60;
                                    tmi.tm_isdst = 0;
                                    time_t mtime = MakeTime(&tmi, 0);
                                    if (mtime>*etime) {
                                        //printf("tmi=%04d-%02d-%02d %02d:%02d:%02d secs=%d\n", tmi.tm_year+1900, tmi.tm_mon+1, tmi.tm_mday, tmi.tm_hour, tmi.tm_min, tmi.tm_sec, secs);
                                        //printf("mtime=%d > *etime=%d\n", mtime, *etime);
                                        max_num = num;
                                        break;
                                    }
                                    memcpy(buf, &mtime, sizeof(time_t));
                                    buf += sizeof(time_t);
                                    if (fread(buf, data_sz, 1, f_strm) < 1) break;
                                    buf += other_func->get_save_size(type, 1);
                                    num++;
                                }
                                //if (fread(&oclk, sizeof(OClockRcd), 1, f_strm) < 1) break;
                                for(;;) {   //时序乱时的容错处理
                                    if (fread(&oclk2, sizeof(OClockRcd), 1, f_strm) < 1) {
                                        oclk.count = 0;
                                        break;
                                    }
                                    if (oclk2.oclock<=oclk.oclock) {
                                        oclk.count--;
                                        fseek(f_strm, (data_sz+sizeof(short))*oclk2.count, SEEK_CUR);
                                    } else {
                                        memcpy(&oclk, &oclk2, sizeof(OClockRcd));
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                fclose(f_strm);
            }
        }
    }
    return num;
}

#define SCAN_OCLOCK(hour) \
    fseek(f_strm, sizeof(RcdFileHead), SEEK_SET);\
    for (i=0; i<rec_hd->count; i++) {\
        k = fread(o_rec, sizeof(OClockRcd), 1, f_strm);\
        if (k!=1) printf("fread oClockRcd failure, k=%d, sizeof(OClockRcd)=%d\n", k, sizeof(OClockRcd));\
        if (o_rec->oclock>=hour) break;\
        fseek(f_strm, rec_sz*o_rec->count, SEEK_CUR);\
    }

#define SCAN_SECS(hsec) \
    for (i=0;i<o_rec->count;i++) {   \
        fread(&secs, sizeof(short), 1, f_strm); \
        if (secs>=hsec) {   \
            fseek(f_strm, -sizeof(short), SEEK_CUR);    \
            break;  \
        }   \
        fseek(f_strm, size, SEEK_CUR);  \
    }

/*!
Description:Locate record will be saved with time in file

    Input:      f_strm -- FILE stream of file be locate
                tim -- time of record be located
                rec_hd->count -- record file head
                size -- data size in bytes(exclude secs)
                unit
    Output:     rec_hd, o_rec --
*/
void SaveFunc::LocateSave(FILE *f_strm, tm *tim, RcdFileHead * rec_hd,
                          OClockRcd *o_rec, int size, int unit)
{
    int i, k;

    int rec_sz = size + sizeof(short);
    fseek(f_strm, sizeof(RcdFileHead), SEEK_SET);
    SCAN_OCLOCK(tim->tm_hour)
    bool apd = false;   //append
    if (i >= rec_hd->count) apd = true;
    rec_hd->count = i + 1;
    o_rec->unit = unit;

    if (o_rec->oclock == tim->tm_hour) { //Found. replace
        short hsec = tim->tm_min * 60 + tim->tm_sec;
        short secs;
        SCAN_SECS(hsec)
        o_rec->count = i;
    } else {    //Not found. Insert or append
        o_rec->oclock = tim->tm_hour;
        o_rec->count = 0;
    }
    if (!apd) {
        SAVE_OCLOCK(f_strm, (*o_rec), size)       //Save OClockRcd
    } else fwrite(o_rec, sizeof(o_rec), 1, f_strm);
}

/*!
Description:Locate record will be read with time in file

    Input:      f_strm -- FILE stream of file be locate
                tim -- time of record be located
                rec_hd -- record file head
                size -- data size in bytes(exclude secs)
    Output:     rec_hd -- rec_hd.count=remain count of hours
                o_rec -- o_rec.count=remain count of record in one hour
    Return:     false=not found
*/
bool SaveFunc::LocateRead(FILE *f_strm, tm *tim, RcdFileHead * rec_hd, OClockRcd *o_rec, int size)
{
    int rec_sz = size + sizeof(short);
    int i, k;
    SCAN_OCLOCK(tim->tm_hour)
    rec_hd->count -= i;
    //printf("o_rec->oclock=%d tim->tm_hour=%d, tim->tm_mday=%d\n", o_rec->oclock, tim->tm_hour, tim->tm_mday);
    if (o_rec->oclock < tim->tm_hour) { //Not found.
        return false;
    } else if (o_rec->oclock > tim->tm_hour) {
        return true;
    }

    short hsec = tim->tm_min * 60 + tim->tm_sec;
    short secs;
    SCAN_SECS(hsec)
    o_rec->count -= i;
    //printf("secs=%d, hsec=%d\n", secs, hsec);
    if (secs < hsec) { //Not found.
        if (rec_hd->count>1) {
            rec_hd->count--;
            k = fread(o_rec, sizeof(OClockRcd), 1, f_strm);
            if (k!=1) printf("fread oClockRcd failure, k=%d\n", k);
            return true;
        } else {
            return false;
        }
    } else {
        return true;
    }
}

/*!
Desctiption:Save power on&off time
*/
void SaveFunc::SavePower01Time()
{
    time_t buf[kPower01MaxSz];

    FILE *f_strm = fopen(Power01File, "rb+");   //打开开关机时间存储文件
    if (!f_strm) {
        f_strm = fopen(Power01File, "wb");
        if (!f_strm) {
            printf("Create %s failure\n", Power01File);
            return;
        }
        memset(buf, 0, sizeof(time_t)*kPower01MaxSz);
    } else {
        fread(&buf[2], sizeof(time_t), kPower01MaxSz - 2, f_strm);
        fseek(f_strm, 0, SEEK_SET);
    }
    buf[0] = power_01_tm_[1];
    buf[1] = power_01_tm_[0];
    fseek(f_strm, 0, SEEK_SET);
    fwrite(buf, sizeof(time_t), kPower01MaxSz, f_strm);
    fclose(f_strm);
}

/*!
Description:Read power on&off time from file

    Input:  cnt -- Read number
    Output: tmt -- Time be read
    Return: the number of items actually read, 0=no data
*/
int SaveFunc::ReadPower01Time(time_t* tmt, int cnt)
{
    FILE *f_strm = fopen(Power01File, "rb");   //打开开关机时间存储文件
    if (!f_strm) return 0;

    time_t buf[kPower01MaxSz];
    fread(buf, sizeof(time_t), kPower01MaxSz, f_strm);
    int i;
    if (cnt > kPower01MaxSz) cnt = kPower01MaxSz;
    for (i = 0; i < cnt; i++) {
        if (!buf[i]) break;
        *tmt = buf[i];
        tmt++;
    }
    fclose(f_strm);
    return i;
}

void SaveFunc::UpdatePower01Time()
{
    time_t tm_t = time(NULL);
    time_t tmp = 0;
    
    if (ReadPower01Time(&tmp,1)) { //有开关机记录
        if(!mcu_dev->Read0Time(&tmp) return;
    }

    power_01_tm_[0] = tmp;  //power off time
    char ci = 0;
    shmem_func().SetLphd(kLphdPwrOff, &ci, tmp);
    power_01_tm_[1] = tm_t - 5; //power on time
    shmem_func().IniAlmTime(tm_t);
    ci = 1;
    shmem_func().SetLphd(kLphdPwrOn, &ci, tm_t);
    shmem_func().IncDataUp(1);
    notice_pthread(kTTSave, SAVEINFO, kPower01Time, NULL);
}

/*!
Description:read Plt

    Input:      stime -- start time of record(exclude)
                etime -- end time of record(include)
                max_num -- maximum number be read. 0=default number
    Output:     buf -- record save buffer
    Return:     number of record
*/
int SaveFunc::ReadPltData(char * buf, time_t* stime, time_t* etime, short max_num)
{
    char filename[128];
    if (!max_num) max_num = 255;

    char *pbuf = buf;
    buf += 3;
    //Modify time to 0:00 2:00 4:00 ... 22:00
    *stime += 1;
    tm tmi;
    GmTime(&tmi, stime);
    //LocalTime(&tmi, stime);
    if (tmi.tm_hour % 2) {
        tmi.tm_hour++;
        if (tmi.tm_hour >= 24) tmi.tm_hour = 0;
    }
    tmi.tm_min = 0;
    tmi.tm_sec = 0;
    time_t timei = MakeTime(&tmi, 0);

    int recsz = sizeof(time_t) + other_func->get_save_size(kPstSave, 1);
    recsz = recsz * 255;
    char *pst_buffer = new char[4 + recsz];
    float psti[3][16], plti[3];
    int num = 0;
    time_t tmt = timei;
    timei += 7200;  //add 2 hour
    while (num < max_num) {
        char *pst_buf = pst_buffer;
        int pst_num = ReadRecData(&pst_buf[1], &tmt, etime, 0, kPstSave);
        if (!pst_num) break;
        *pbuf = pst_buf[1];
        pst_buf += 4;
        int m, n;
        m = n = 0;
        short q = 0;
        while(m < pst_num) {
            memcpy(&tmt, pst_buf, sizeof(tmt));
            pst_buf += 4;
            short si = *((short*)pst_buf);
            for (int i = 0; i < 3; i++) {
                psti[i][n] = *(short*)pst_buf;
                psti[i][n] /= 1000;
                pst_buf += sizeof(short);
            }
            if (*(short*)pst_buf) q = *(short*)pst_buf;
            pst_buf += 4;
            m++;
            if (n < 15) n++;
            if (tmt >= timei) {
                if (n > 6) {
                    memset(plti, 0, sizeof(plti));
                    memcpy(buf, &timei, sizeof(time_t));
                    buf += sizeof(time_t);
                    for (int i = 0; i < 3; i++) {
                        for (int j = 0; j < n; j++) {
                            plti[i] += psti[i][j] * psti[i][j] * psti[i][j];
                        }
                        short si = cbrt(plti[i] / n) * 1000 + 0.5;
                        memcpy(buf, &si, sizeof(short));
                        buf += sizeof(short);
                    }
                    memcpy(buf, &q, sizeof(short));
                    q = 0;
                    buf += 4;
                    num++;
                }
                timei += 7200;  //add 2 hour
                n = 0;
            }
        }
    }
    delete [] pst_buffer;

    return num;
}

/*!
    Called by:  thread_save
*/
void SaveFunc::ClearDB(int num)
{
    sqlite_db->ResetHrmDB(num);
}

/*!
Save 10 cycle harmonic data
*/
static FILE * file_cyc10 = NULL;
static char filename[64];
void SaveFunc::Savecyc10(void *p)
{
    Hrm10CycBuf *pbuf = (Hrm10CycBuf *)p;
    tm tmi;
    if (!file_cyc10) {
        //printf("pbuf->time[0].tv_sec=%d\n", pbuf->time[0].tv_sec);
        LocalTime(&tmi, &(pbuf->time[0].tv_sec));
        int n = sprintf(filename, "%s", Hrm10Path);
        strftime(&filename[n], 24, "%Y%m%d_%H%M%S.txt", &tmi);
        //printf("filename=%s\n", filename);
        file_cyc10 = fopen(filename, "w+");
        if (!file_cyc10) {
            printf("open file:%s failure!\n", filename);
            return;
        }
        //printf("opened %s\n", filename);
    }
    
    int sec; char tm_str[24];
    for (int i=0; i<15; i++) {
        LocalTime(&tmi, &(pbuf->time[i].tv_sec));
        strftime(tm_str, 20, "%Y%m%d_%H%M%S", &tmi);
        fprintf(file_cyc10, "%s.%03d ", tm_str, pbuf->time[i].tv_usec/1000);
        for (int j=0;j<3;j++) {
            fprintf(file_cyc10, "%7.3fV ", pbuf->rms[i][j*2]);
        }
        fprintf(file_cyc10, "; ");
        for (int j=0;j<3;j++) {
            fprintf(file_cyc10, "%6.3fV, %5.1f°; ", pbuf->data[i][j*2], pbuf->data[i][j*2+1]);
        }
        fprintf(file_cyc10, "  ");
        for (int j=3;j<6;j++) {
            fprintf(file_cyc10,"%6.4fA,%5.1f°; ", pbuf->data[i][j*2], pbuf->data[i][j*2+1]);
        }
        fprintf(file_cyc10, "\n");
    }
        
    if (!pbuf->on_off) {
        fclose(file_cyc10);
        file_cyc10 = NULL;
        //printf("closed %s\n", filename);
    }
}

/*!
Read a group of harmonics to buffer from database

    Input:  stime -- start time
            etime -- end time
    Return: total number of entries in buffer
*/
int SaveFunc::ReadHarms2Buf(long stime, long etime)
{
    //printf("stime=%d, etime=%d\n", stime, etime);
    sqlite_db->ReadDB(stime, etime);
    return sqlite_db->hrm_rec_num();
}