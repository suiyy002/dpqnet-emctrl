#ifndef VersionH
#define VersionH

const int VER_1st = 3;
const int VER_2nd = 7;
const int VER_3rd = 4;

const int VER_sn = 0;

enum PRD_TYPE   //产品型号
{
	kPQNet101=101, kPQNet111,
	kPQNet200=120, kPQNet210,
	kPQNet300=130, kPQNet310,
	kPQNet410=140,
	kPQAny316=180,
	kPQAny416=190,
	kEEWNet200=200,  //变压器
	kEEWNet300=210, kEEWNet310,   //电容，电容有录波触发
};

#define EQUIP_TYPE (data_buffer().equip_para()->pqm_type())

#ifdef PQNet2xx_3xx
	const static int CUR_PHS_MODIFY = 0; //电流相位修正系数	

#elif defined PQAny3xx
	const static int CUR_PHS_MODIFY = 3; //电流相位修正系数
#else //Equipment type  
#endif

#ifdef X86
	#define FEED_DOG_TIM 40  //喂狗间隔
	#define SRL_PORT_SNUM 1 //Serial port start number

#elif defined ARM
	#define FEED_DOG_TIM 3	 //喂狗间隔
	#define SRL_PORT_SNUM 3 //Serial port start number

#else //Other architecture

#endif

#endif /* VersionH */
