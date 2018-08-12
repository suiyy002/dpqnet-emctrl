//---------------------------------------------------------------------------
#include <stdio.h>
#include "debug_info.h"

//---------------------------------------------------------------------------
pthread_mutex_t debug_mutex; //用于调试信息处理的互斥锁变量
CDebugInfo *pdebugInfo;

CDebugInfo &debugInfo()
{
	static CDebugInfo dbginf;
	return dbginf;
}
//---------------------------------------------------------------------------
CDebugInfo::CDebugInfo()
{
    str_buf = new string[MAXSIZE];
    head = 0;
    tail = 0;
    size = 0;
    for(int i = 0; i < MAXSIZE; i++) {
        str_buf[i] = "";
    }
}

//---------------------------------------------------------------------------
CDebugInfo::~CDebugInfo()
{
    delete [] str_buf;
}

void CDebugInfo::Push(const string &str)
{
    str_buf[head] = str;
    head = (++head) % MAXSIZE;
    if(size < MAXSIZE - 1) {
        size++;
    } else {
        tail = (++tail) % MAXSIZE;
    }
    printf("size=%d, head=%d, tail=%d\n", size, head, tail);
}

//---------------------------------------------------------------------------
//从最早压入的数据开始弹出
const char* CDebugInfo::Pop()
{
    int cur;

    if(size > 0) {
        cur = tail;
        tail = (++tail) % MAXSIZE;
        size --;
        return str_buf[cur].c_str();
    }
    return NULL;
}

//---------------------------------------------------------------------------
//根据要弹出的数目，初始化弹出数据的开始位置
void CDebugInfo::InitPopdown(int tol)
{
    if(tol < size) {
        pop_point = tail + tol;
        pop_point %= 32;
    } else {
        pop_point = head;
    }
    //printf("size=%d, head=%d, tail=%d, pop_point=%d\n", size, head, tail, pop_point);
}
//---------------------------------------------------------------------------
//从指定位置的数据开始，往更早的数据弹出
const char* CDebugInfo::Popdown()
{
    if(pop_point <= 0) pop_point = MAXSIZE - 1;
    else pop_point--;
    if(size > 0) {
        size--;
        tail = (++tail) % MAXSIZE;
    }
    //printf("pop_point=%d\n", pop_point);
    return str_buf[pop_point].c_str();
}

//---------------------------------------------------------------------------
const int CDebugInfo::Size()
{
    return size;
}


