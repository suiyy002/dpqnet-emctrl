//---------------------------------------------------------------------------
#ifndef _LOOP_BUF_SV_H_
#define _LOOP_BUF_SV_H_
//---------------------------------------------------------------------------
#include "loop_buffer.h"
#include <string>
using namespace std;
//---------------------------------------------------------------------------
template <class T>
class LoopBufSV:public LoopBuffer<T>
{
    using LoopBuffer<T>::buf_size_;
    using LoopBuffer<T>::trash_;
    using LoopBuffer<T>::buffer_;
public:
    LoopBufSV(int size);
    ~LoopBufSV();
    
    T *Pop();
    T *PushP();
    T *Read(int idx=0);
    
protected:
private:

};

template <class T>
LoopBufSV<T>::LoopBufSV():LoopBuffer<T>(size)
{
}

template <class T>
LoopBufSV<T>::~LoopBufSV()
{
}

/*!
only return the data address point, and change the head point. 
the data be pushed by calling function later.
*/
template <class T>
T *LoopBufSV<T>::PushP()
{
    T *ptemp = buffer_ + head_;
    
    ++head_ &= buf_size_;
    if (tail_==head_) {
        ++tail_ &= buf_size_;
    }
    return ptemp;
}

/*!
    Return:   NULL=buffer be empty
*/
template <class T>
T * LoopBufSV<T>::Pop()
{
    if (tail_==head_) return NULL;
    T *ptemp = buffer_ + tail_;
    ++tail_ &= buf_size_;
    return ptemp;
}

/*!
    Input:  idx -- 0=1st data in buffer
    Return: NULL=buffer be empty
*/
template <class T>
T * LoopBufSV<T>::Read(int idx)
{
    if (idx>=data_num()) return NULL;
    int pos = tail_ + idex;
    pos &= buf_size_;
    T *ptemp = buffer_ + pos;
    return ptemp;
}

#endif  //_LOOP_BUF_SV_H_

