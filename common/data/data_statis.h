/*! \file data_statis.h
    \brief pq data statistic.
    Copyright (c) 2018  Xi'an Boyuu Electric, Inc.
*/
#ifndef _DATA_STATIS_H_
#define _DATA_STATIS_H_

#include "loopbuf_sort.h"

template <class T>
class DataStatis
{
public:
	DataStatis(int size, int (*compfunc)(const void *, const void *));
	~DataStatis();

    void IniData();
    void SetData(T val, float avg2);
    void GetData(T *val);
    
    //Mutators
    void set_cp95_en(bool val) { cp95_en_ = val; };
protected:

private:
    T data_[4];   //[0-3]:average, maximum, minimum, cp95
    float avg_tmp_; //temporary variable for calculate average
    int count_;     //used for calculate average
    bool cp95_en_;  //enable cp95 calculate. true=enable
    LoopBufSort<T> *cal_cp95_;   //calculate cp95
};

template <class T>
DataStatis<T>::DataStatis(int size, int (*compfunc)(const void *, const void *))
{
    cal_cp95_ = new LoopBufSort<T>(size, compfunc, NULL);
    cp95_en_ = false;
    IniData();
}

template <class T>
DataStatis<T>::~DataStatis()
{
    delete cal_cp95_;
}

template <class T>
void DataStatis<T>::IniData()
{
    count_ = 0;
    memset(data_, 0, sizeof(data_));
    avg_tmp_ = 0;
    
    memset(&data_[2], 0x7f, sizeof(T));    
    cal_cp95_->Clear();
}

/*!
    Input:  val -- 3s value; 
            avg2 -- average of sum of squares of 10-cycle value over 3s
*/
template <class T> 
void DataStatis<T>::SetData(T val, float avg2)
{
    avg_tmp_ += avg2;
    if (val>data_[1]) data_[1] = val;
    if (val<data_[2]) data_[2] = val;
    if (cp95_en_) cal_cp95_->InsertMax(&val);
    count_++;
};

/*!
    Output: val -- T*[4]
*/
template <class T>
void DataStatis<T>::GetData(T *val)
{
    printf("avg_tmp_=%6.3f, count_=%d\n", avg_tmp_, count_);
    data_[0] = avg_tmp_/count_;
    cal_cp95_->Seek(0); 
    if (cp95_en_) {
        cal_cp95_->Read(&data_[3]);
    } else {
        float frand = rand();
        printf("???\n");
        frand /= RAND_MAX;
        data_[3] = data_[0] + (data_[1]-data_[0])*frand;
    }
    memcpy(val, data_, sizeof(T)*4);
}

#endif // _DATA_STATIS_H_ 
