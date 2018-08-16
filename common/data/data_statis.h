/*! \file data_statis.h
    \brief pq data statistic.
    Copyright (c) 2018  Xi'an Boyuu Electric, Inc.
*/
#ifndef _DATA_STATIS_H_
#define _DATA_STATIS_H_

template <class T>
class DataStatis
{
public:
	DataStatis(int size, int (*compfunc)(const void *, const void *));
	~DataStatis();

    void IniData();
    void SetData(T val) { SetData(val, val); };
    void SetData(T val, float avg2);
    void GetData(T *val);
    
    //Mutators
    void set_cp95_en(bool val) { cp95_en_ = val; };
protected:

private:
    int (*compare_)(const void *, const void *);
    void SetCp95buf(T *val);

    T data_[4];   //[0-3]:average, maximum, minimum, cp95
    float avg_tmp_; //temporary variable for calculate average
    int count_;     //used for calculate average
    bool cp95_en_;  //enable cp95 calculate. true=enable
    T *cp95_buf_;   //buffer used to calculate cp95
    int max_idx_;
};

template <class T>
DataStatis<T>::DataStatis(int size, int (*compfunc)(const void *, const void *))
{
    compare_ = compfunc;
    cp95_buf_ = new T[size+1];
    cp95_en_ = false;
    max_idx_ = size;
    IniData();
}

template <class T>
DataStatis<T>::~DataStatis()
{
    delete [] cp95_buf_;
}

template <class T>
void DataStatis<T>::IniData()
{
    memset(data_, 0, sizeof(data_));
    memset(&data_[2], 0x7f, sizeof(T));
    count_ = 0;
    avg_tmp_ = 0;
    
    memset(cp95_buf_, 0, sizeof(cp95_buf_));
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
    if (cp95_en_) SetCp95buf(&val);
    count_++;
};

/*!
    Output: val -- T*[4]
*/
template <class T>
void DataStatis<T>::GetData(T *val)
{
    //printf("avg_tmp_=%6.3f, count_=%d\n", avg_tmp_, count_);
    data_[0] = avg_tmp_/count_;
    if (cp95_en_) {
        data_[3] = cp95_buf_[0];
    } else {
        float frand = rand();
        frand /= RAND_MAX;
        data_[3] = data_[0] + (data_[1]-data_[0])*frand;
    }
    memcpy(val, data_, sizeof(T)*4);
}

template <class T>
void DataStatis<T>::SetCp95buf(T *val)
{
    int k = compare_(val, &cp95_buf_[0]);
    if (k<=0) return;
    k = compare_(val, &cp95_buf_[max_idx_]);
    if (k>=0) {
        memcpy(cp95_buf_, &cp95_buf_[1], sizeof(T)*max_idx_);
        cp95_buf_[max_idx_] = *val;
    } else {
        for (int i=1; i<max_idx_; i++) {
            k = compare_(val, &cp95_buf_[i]);
            if (k<=0) {
                memcpy(cp95_buf_, &cp95_buf_[1], sizeof(T)*(i-1));
                cp95_buf_[i-1] = *val;
            }
        }
    }
}

#endif // _DATA_STATIS_H_ 
