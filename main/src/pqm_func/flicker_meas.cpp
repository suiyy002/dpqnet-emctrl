/*! \file flicker_meas.cpp
    \brief flicker measurement.
*/

#include "flicker_meas.h"

using namespace std;

static const int kPstPara[3][3] = { //[][0-2]:StSTEP, SampleRate, AvgNum
    {640, 400, 20},     //PQM-3
    {2560, 2560, 80},   //PQNetxxx
    {1600, 1600, 40},   //PQNetxxxD
};

/*!
    Input:  model -- equipment model
*/
FlickerMeas::FlickerMeas(EquipModel4Pst model)
{
    const int *i_p = kPstPara[model];
    avgnum_perstep_ = i_p[0]/i_p[2];
    totl_stepnum_ = i_p[1]*600/i_p[0];
    sz_of_buf_ = avgnum_perstep_*totl_stepnum_;
    
    for (int i; i<3; i++) {
        pst_buf_[i] = new long[sz_of_buf_];
    }
    
}

FlickerMeas::~FlickerMeas()
{
    for (int i; i<3; i++) {
        delete [] pst_buf_[i];
    }
}

/*!
写入瞬时闪变视感度

    Input:  phs -- 相别,0=A,1=B,2=C;
            cnt -- 第几步
            avgp -- 每一步平均后的瞬时闪变值
*/
void FlickerMeas::WriteSt(short phs, short cnt, float *avgp)
{
	short k,j;

	if(cnt>=totl_stepnum_) return;
	for (j=0;j<avgnum_perstep_;j++) {
		pst_buf_[phs][cnt*avgnum_perstep_+j] = avgp[j]*10000;
	}
}

/*!
Statistic Pst

    Input:  dp -- 瞬时闪变视感度平均值
            num -- number of average value
*/
float FlickerMeas::StatisPsti(unsigned long *dp, long num)
{
	qsort(dp, num, sizeof(long), CompareInt);
	//QuickSorti(dp,0,num-1);

	float P01,P1,P3,P10,P50;
	long l1 = num;
	P01 = dp[l1*999/1000];
	P1 = dp[l1*99/100];
	P3 =  dp[l1*97/100];
	P10 = dp[l1*9/10];
	P50 = dp[l1/2];

	float K01,K1,K3,K10,K50;
	K01 = 0.0314; K1 = 0.0525;
	K3 = 0.0657; K10 = 0.28;
	K50 = 0.08;
	return sqrt(K01*P01+K1*P1+K3*P3+K10*P10+K50*P50);
}

//Description:  修正Pst的精度
//              phs, 相别
void FlickerMeas::ModifyPst(short phs)
{
	if(pst_[phs]>0.93){
		pst_[phs] += 0.02;
	}
	if(pst_[phs]>1.9){
		pst_[phs] += 0.02;
	}
	if(pst_[phs]>2.9){
		pst_[phs] += 0.02;
	}
	if(pst_[phs]>4.9){
		pst_[phs] += 0.02;
	}
}

//Description:	获取Pst的结果
//Input:	phs,相别;
//          mdfy:是否修正,1=修正
//Return:	Pst计算结果
float FlickerMeas::GetPstResult(short phs, short mdfy)
{
	pst_[phs] = StatisPsti(pst_buf_[phs], sz_of_buf_)/100;
	if(mdfy) ModifyPst(phs);
	return pst_[phs];
}

