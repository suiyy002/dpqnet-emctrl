#include "generic.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

using namespace std;

/*!
Generate GUID

    Input:  guid -- uint16_t[8]
    Return: guid in string type
*/
char *GenGuid( uint16_t *guid )
{
    int i;
    static char guid_str[33];
    for (i=0; i<8; i++) {
        guid[i] = rand()%0xffff;
        sprintf(&guid_str[i*4], "%04x", guid[i]);
    }
    return guid_str;
}

/*!
Compare two integer

    Input:  arg1,arg2 -- data be compared
    Return: -1=(arg1<arg2), 0=(arg1==arg2), 1=(arg1>arg2)
*/
inline int CompareInt(const void *arg1, const void *arg2)
{
    int a = *(int*)arg1;
    int b = *(int*)arg2;
    if (a>b) return 1;
    else if (a<b) return -1;
    else return 0;
}

/*!
Compare two float

    Input:  arg1,arg2 -- data be compared
    Return: -1=(arg1<arg2), 0=(arg1==arg2), 1=(arg1>arg2)
*/
int CompareFloat(const void *arg1, const void *arg2)
{
    float a = *(float*)arg1;
    float b = *(float*)arg2;
    if (a>b) return 1;
    else if (a<b) return -1;
    else return 0;
}

/*!
if the path not exist, create it

    Return: 0=success, 1=failure
*/
int AssertPath(const char * path)
{
    FILE *fp = fopen(path, "rb");
    if (fp==NULL) {
        char stri[128];
        sprintf(stri, "mkdir -p %s", path);
        system(stri);
        fp = fopen(path, "rb");
        if (fp==NULL) {
            printf("create path %s failure!\n", path);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

/*!
Calculate the number of decimal places that fdata should retain��
example1��fdata=123456.15675... totl=5 decim=3 ���=0
example2��fdata=12345.15675... totl=5 decim=3 ���=0
example3��fdata=1234.15675... totl=5 decim=3 ���=1
example4��fdata=123.15675... totl=5 decim=3 ���=2
example5��fdata=12.15675... totl=5 decim=3 ���=3
example6��fdata=1.15675... totl=5 decim=3 ���=3
Note��Positive and negative are the same

    Input:  fdata --
            totl -- total number of significant digits
            decim -- Maximum number of decimal places
*/
int FloatFmt(float fdata, int totl, int decim)
{
	long lj, li = 1;
	int i;
	
	for(i=0;i<totl;i++) li *= 10;
	i = 0;
	li -= 1;
	lj = (long)fdata;
	if(lj==0)lj = 1;
	lj = li/lj;
	lj /= 10;
	while(lj){
		lj /= 10;
		i++;
	}
	i = i>decim?decim:i;
	return i;
}

/*!
Adjust save space to valid value

    Input:  space -- interval time. unit:s
            type -- 0=3s, 1=freq
*/
const static int ValidSaveSpaces3s[ ] = {3, 6, 12, 15, 30, 45, 60, 90, 
            120, 180, 240, 300, 360, 600, 720, 900};
const static int ValidSaveSpacesFrq[ ] = {1, 3, 10, 60, 180, 600};
unsigned int AdjSaveSpc(unsigned int space, int type)
{
	int k; 
	const int *pk;
    if (type) {
        k = sizeof(ValidSaveSpacesFrq)/sizeof(int);
        pk = ValidSaveSpacesFrq;
    } else {
        k = sizeof(ValidSaveSpaces3s)/sizeof(int);
        pk = ValidSaveSpaces3s;
    }
    for (int i=0; i<k; i++) {
        if (space<=pk[i]) return pk[i];
	}
	return pk[k-1];
}
