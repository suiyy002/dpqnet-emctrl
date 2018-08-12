//---------------------------------------------------------------------------

#ifndef debug_infoH
#define debug_infoH
//---------------------------------------------------------------------------
#include <string>
using namespace std;
//---------------------------------------------------------------------------
class CDebugInfo
{
public:
    CDebugInfo();
    ~CDebugInfo();
    void Push(const string &str);
    const char* Pop();
    const int Size();
    void InitPopdown(int tol);
    const char* Popdown();
    
protected:
private:

    string *str_buf;
    int head;
    int tail;
    string strtmpi;
    int size;   //已使用的空间大小
    int pop_point; //Popdown()函数当前要弹出数据的指针
static const int MAXSIZE=32;
};
extern CDebugInfo *pdebugInfo;

CDebugInfo &debugInfo();

extern pthread_mutex_t debug_mutex;
#endif

