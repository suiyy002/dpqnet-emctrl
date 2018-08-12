#ifndef _FLICKER_MEAS_H_
#define _FLICKER_MEAS_H_

enum EquipModel4Pst {kPQM_34Pst, kPQNet4Pst, kPQNetxxD4Pst};

class FlickerMeas
{
public:
	FlickerMeas(EquipModel4Pst model=kPQNetxxD4Pst);
	~FlickerMeas();
	
	void WriteSt(short phs, short cnt, float *avgp);
    float GetPstResult(short phs, short mdfy);

 
protected:
private:
    void ModifyPst(short phs);
    float StatisPsti(unsigned long *dp, long num);
    
    long *pst_buf_[3];
    float pst_[3];

};


#endif //_FLICKER_MEAS_H_

