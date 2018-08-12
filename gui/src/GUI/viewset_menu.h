#ifndef VIEWSET_MENU_H
#define VIEWSET_MENU_H

typedef enum {
  //����
    //���ý���
    VW_S0,          //������
    VW_SY0,         //ϵͳ������
    VW_AUDIT,       //���������
    VW_SteadyPara,  //��ָ̬��������ý���
    VW_LINE,        //��·��������
    VW_RecWavePara, //¼����������
    VW_COMM,        //ͨѶ��������
    VW_OTHERSET,    //������������
    VW_CUSTOM,      //�Զ�����ֵ����
    VW_EEW,         //�����豸����Ԥ��
    VW_RESERVE1,    //Ԥ������1
    VW_HIDE,        //�������ý���
    //ϵͳ���ý���
    VW_VERIFY,      //�����徫��У׼����
    VW_SYSPARA,     //ϵͳ�������ý���
    VW_INIT,        //ϵͳ��ʼ������
    VW_SYS_RSV1,    //ϵͳԤ������1
    VW_SYS_RSV2,    //ϵͳԤ������2
    VW_SYS_RSV3,    //ϵͳԤ������3
    VW_SYS_HIDE,    //ϵͳ��������
    VW_SYS_HIDE2,    //ϵͳ��������2
    //��ƽ���
    VW_AuditWarn,   //�澯�¼���ѯ����
    VW_AuditAlarm,  //�����¼���ѯ����

    VW_ShortRMSVVR,  //��̬��ѹ�����������ý��� Short-duration rms voltage variations
    VW_ShortRMSIVR,  //��̬���������������ý��� Short-duration rms current variations
    VW_GpsParamSet,     //GPS�������ý���
    VW_CvtModify,       //CVT ��������
    VW_SetResRatio,     //���õ���ϵ������
    VW_VoltLinearity,     //��ѹ���Զ�����������
    VW_RcdSpaceSaveType,    //ȡֵ���ͼ��洢������ý���
    VW_SeeCharacterDCValue, // �鿴����ֱ������.
    VW_SYSPARA_RSV1,    //ϵͳ�������ý���Ԥ������1
    VW_SYSPARA_RSV2,    //ϵͳ�������ý���Ԥ������2
    VW_SYSINIT_RSV1,    //ϵͳ��ʼ������Ԥ������1
    VW_SetHarmLimit,    //����г����ֵ
    VW_CvtModifyK,      //CVT ����ϵ�����ý���
    VW_HmCurrLimit,     //����г������ֵ��ѯ����
    VW_CapThresh,     //����Ԥ����ֵ���ý���
    VW_raDisHarm,   //��-����ʾг�������ý���
    VW_raParam,    //��-������������ý���
    VW_raThresh,     //��-����ֵ��ѯ����
    VW_TrnsfmrParam,    //Transformer param set view
    VW_SteadyTrigPara,  //��̬�¼������������ý���
    VW_Fluctuation,     //��ѹ�����������ý���
    VW_InterHarm,       //��г���������ý���
    VW_AlmEnable,       //����ʹ�����ý���
    VW_LOGIN,       //��¼����
    VW_GUI_DILIMITER,   //---- ͨ�ý���ֽ�� ----------------------------------


    //����
    kUserName,
    kPassword,
    VoltWarp,       // �����ѹƫ��.
    ShortCap, UserCap, SuppCap,     // ��С��·�������õ�Э�������������豸����.
    CLimitValue,    // ����г����ֵ
    TransLimit,     // ��̬����ֵ.
    TransCurLimit,  //������̬����ֵ.
    PTScale, CTScale,   // PT��ȡ�CT���.
    VoltLvl,        // ��ѹ�ȼ�.
    UnitNum,        // ���õ�Ԫ��.
    BaudRateSet,    // ���ô��ڲ�����.
    TransEnable,    // ��̬���ʹ��.
    TransCurEnable, //������̬���ʹ��.
    DeviceSn,       // �豸���к�.
    SmpVltScale, SmpCurScale,       // ������ѹ��ȡ������������.
    TranstTolTime, TranstRcdTime,   // ��̬�¼��ܼ�¼ʱ�䡢һ���¼�����¼ʱ��
    kFreqEvalTm,    //Ƶ�ʲ������
    kMdfyEvnStm,    //�¼���ʼʱ��У׼ϵ��
    SysIni,         // ϵͳ��ʼ��.
    ModifyTime,     // ����ʱ��.
    ModifyPasswd,   // �޸�����.
    IPAddrSet, SocketServerPort,    // ����IP��ַ���˿ں�
    NetMaskSet,     // ������������.
    DebugEnable,    // �������ʹ��.
    VAResRatio, VBResRatio, VCResRatio, // A��B��C���ѹ����ϵ��.
    CAResRatio, CBResRatio, CCResRatio, // A��B��C���������ϵ��.
    VALineCoef, VBLineCoef, VCLineCoef, // A��B��C���ѹ���Զ�ϵ��.
    VorCDatum,      //����У׼��׼��ѹ.
    PhsAdjEnable,   //����У׼���ʹ��.
    VorC_AdjEnable, //����У׼��ѹ����ʹ��.
    AutoAdj,        //�����Զ�У׼.
    kVoltLinearity, //��ѹ���Զ�����
    PstMdfyEnable,  //��������ʹ��.
    PstEnable,      //����ʹ��.
    kSecurityEn,    //�Ƿ�ǿ��ִ��Զ�̷��ʰ�ȫ���
    SaveWaveEnable, //¼�����ݱ���ʹ��
    Aggregation10min,   //ÿ10�����������¿�ʼ����
    GateWaySet,     //��������.
    CurrClampEnable,    // ����ǯʹ��.
    CurrClampType,  // ���õ���ǯ�ͺ�.
    LCMDelayTime,   // ����LCM��Դ�رյȴ�ʱ��.
    HarmRcdSpace,   // ����г�����ݴ洢���
    ResetPassword,  // ��λ�û�����
    HrmRcdSaveType, // г����¼�洢����
    ResetDefaultPara,   //�ָ���������
    AdjustZeroEnable,       //�����ʵʱ���ι�������ʹ��
    SetLimitType,   //���ó���ֵ����
    ImbalanceLimit, //�������಻ƽ�ⳬ��ֵ
    NegativeILimit, //�������������ֵ
    FrequecyLimit,  //����Ƶ����ֵ
    PstLimit,       //����������ֵ
    SetHarmLimitSub,    //����г����ֵ�ӽ���
    SetMacAddr,     // ����Ӳ����ַ.
    GetCharacterDCValue,    // ��ȡ����ֱ������.
    VADCValue, CADCValue,   // A���ѹ,��������ֱ������ 
    VBDCValue, CBDCValue,   // B���ѹ,��������ֱ������ 
    VCDCValue, CCDCValue,   // C���ѹ,��������ֱ������ 
    SteadyBakEnable,    //��̬���ݱ���ʹ��
    TranstBakEnable,    //��̬���ݱ���ʹ��
    ConnectType,    //���߷�ʽ
    SynTimeType,    //ʱ���Զ�ͬ����ʽ
    AdSampleRate,   //AD������1024/8=0,2048/10=1,4096/10=2
    TransTbLimit,   //��̬ͻ������ֵ
    StartCurEn,     //��������ʹ��
    StartCurLimit,  //����������ֵ
    TransManualRec, //�ֶ�¼��
    TransTbEnable,
    TransEndNum,
//  SetProofTimeType,   //����GPS��ʱ����
    GpsSingleType,      //m �ź�����
    GpsPulseType,       //m ��������
    SetProofTimeIntr,   //����GPS-PULSE��ʱ���
    SetBTimeIntr,       //����GPS-B��ʱ���
    NTPServerIP,        //NTP Server IP
    ImbalanceRcdSpace,  //��ƽ���¼�洢���
    ImbalanceSaveType,  //��ƽ���¼�洢����
    FreqRcdSpace,       //Ƶ�ʼ�¼�洢���
    FreqSaveType,       //Ƶ�ʼ�¼�洢����
    VoltWarpRcdSpace,   //��ѹƫ��洢���
    VoltWarpSaveType,   //��ѹƫ��洢����
    ShowOcTime,         //
    ZeroInputThr,      //Zero input threshold
    kFilterCP,
    kTimeDiff,      //permissible time difference
    kSignalSimuEn,  //signal simulation enable
    
    CvtModifyEn,    //CVT ����ʹ��
    CvtModifyKSub,  //����CVT ����ϵ���ӽ���
    FluctuationDb,  //Voltage fluctuation deadband
    FluctuationEna, //Voltage fluctuation measurement enable
    ResetHysTime,   //Reset Hysteresis time. unit:hours
    kTimeZone,
    kHarmonic10Cyc, //10 cycle value correspond to harmonic order      
    kClockSource,   //Clock source
    kSysTimeError,  //system time error
    kFixFreqSmpl,   //fix frequency sample
    kCapLifeThr,    //����Ԥ��
    kCapW24hThr,    //24h �ۻ�Ԥ��
    kCapVolt1Thr,   //��������ѹԤ��1
    kCapVolt2Thr,   //��������ѹԤ��2
    kCapCurrThr,    //������Ԥ��
    kCapCapThr,     //������Ԥ��
    kCapPeakThr,    //��ֵ����ѹ
    kraDisHmSub,    //��-����ʾг�������ӽ���
    kCapGamaThr,    //�� ��ֵ Th
    kTransformerMs, //�����·���� Ms
    kCapNmnlMc,     //���ݶ���� Mc
    kCapNmnlUc,     //���ݶ��ѹ Uc
    kCapReactancex, //�����翹���翹��
    kCapCharctHr,   //������г��
    kImpedanceBeta, //��Դ�迹�нǦ�
    kRAThreshold,   //��-����ֵ
    kTrnsfmrPec,    //pu of Eddy Current loss
    kTrnsfmrIn,     //I_N of transformer
    kRecWaveFormat, //Record wave data save format
    kRecWaveSmplRt, //Record wave sampling rate
    kComtradeSvPath,    //COMTRADE file save path
    kStdyTrgrEnable,    //������̬¼������ʹ��
    kFreqTrgrEnable,    //Ƶ��¼������ʹ��   
    kHarmTrgrEnable,    //г��¼������ʹ��
    kUnblcTrgrEnable,   //��ƽ��¼������ʹ��
    kVoltDvTrgrEnable,  //��ѹƫ��¼������ʹ��
    kHarmRecSvMax,  //г����¼��󱣴�����
    kHarmRecSvEn,   //г����¼����ʹ��
    kInterHarmEna,  //��г��ʹ��
    kInterHarmDb,   //��г��deadband
    kInterHmGroup,  //��г��Ⱥȡֵ����
    kHeartbeat61850, //61850 server heartbeat monitor
    kAuditLogSize,  //Audit log record file maximum size
    kAuditWarnList, //Audit log warning record list
    kHarmAlmEna,
    kFreqAlmEna,
    kPstAlmEna,
    kUnblcAlmEna,
    kVoltDvAlmEna,
    
} ViewSetCmd;

//�˵������ݽṹ
struct SMenuItem { //�˵���ṹ
    int val_x;      //�˵�������ֵ����ʼ������(��λ:���ָ���)
    int tag;        //bit0(0=��ʾ���1=����ʾ); bit1(0=����ʾ���,1=��ʾ)
    //bit2(0=����ֵ,1=���ֵ); bit3(0=��ʾ���, 1=����ʾ)
    //bit4(0=Enable,1=Disable);
    char name[24];  //�˵�������
    int cmd;        //��Ӧ����������
};

struct SSetMenu { //���ò˵��ṹ
    int count;  //ÿҳ�˵������Ŀ
    int width;  //�����(��λ:���ָ���)
    int totl;   //�˵������������Ϊ0,��ʾֻ��1ҳ
    int tag;    //����ʵ������������,
     SMenuItem * pmenu_item; //�˵���
};

static  SMenuItem ViewLOGINMenuItem[] = {  //VW_LOGIN ��¼����
    {8, 0,   "USER NAME:", kUserName},
    {8, 0x2, "PASSWORD :", kPassword}
};
static  SSetMenu ViewLOGINMenu = { 2, 14, 0, 0, ViewLOGINMenuItem };

//------ 1st layer menu---------------------------------------------------------
static  SMenuItem ViewS0MenuItem[] = {
    {0, 0, "��·��������", VW_LINE},
    {0, 0, "��ָ̬���������", VW_SteadyPara},
    {0, 0, "ͨѶ��������", VW_COMM},
    {0, 0, "�������ܲ���", VW_OTHERSET},
    {0, 0, "¼����������", VW_RecWavePara},
    {0, 0, "EEW ��������", VW_EEW},
    {0, 0x11, "Ԥ������1", VW_RESERVE1},
    {0, 0x11, "Ԥ������1", VW_RESERVE1},
    {0, 0x1, "��������", VW_HIDE}
};
static  SSetMenu ViewS0Menu = { 9, 12, 0, 0, ViewS0MenuItem };

static  SMenuItem ViewSy0MenuItem[] = {
    {0, 0, "�����徫��У׼", VW_VERIFY},
    {0, 0, "ϵͳ��������", VW_SYSPARA},
    {0, 0, "ϵͳ��ʼ��", VW_INIT},
    {0, 0, "�������ܲ���", VW_OTHERSET},
    {0, 0x11, "ϵͳԤ������1", VW_SYS_RSV1},
    {0, 0x11, "ϵͳԤ������2", VW_SYS_RSV2},
    {0, 0x11, "ϵͳԤ������3", VW_SYS_RSV3},
    {0, 0x1, "ϵͳ��������", VW_SYS_HIDE},
    {0, 0x1, "ϵͳ��������2", VW_SYS_HIDE2}
};
static  SSetMenu ViewSy0Menu = { 9, 12, 0, 0, ViewSy0MenuItem };

static  SMenuItem ViewAuditMenuItem[] = {
    {0, 0, "�澯�¼���ѯ", VW_AuditWarn},
    {0, 0, "�����¼���ѯ", VW_AuditAlarm},
    {10, 0x2, "��־�ļ���С(k):", kAuditLogSize},
    {0, 0x11, "ϵͳԤ������1", VW_SYS_RSV1},
    {0, 0x11, "ϵͳԤ������2", VW_SYS_RSV2},
    {0, 0x11, "ϵͳԤ������3", VW_SYS_RSV3},
};
static  SSetMenu ViewAuditMenu = { 6, 12, 0, 0, ViewAuditMenuItem };

//------ 2nd layer menu---------------------------------------------------------
static  SMenuItem ViewLineMenuItem[] = {       // VW_LINE ��·�������ý���
    {7, 0x6, "PT ���:", PTScale},
    {7, 0x6, "CT ���:", CTScale},
    {7, 0, "��ѹ�ȼ�:", VoltLvl},
    {9, 0x2, "��С��·����:", ShortCap},
    {9, 0x2, "�õ�Э������:", UserCap},
    {9, 0x2, "�����豸����:", SuppCap},
    {9, 0, "PT ���߷�ʽ:", ConnectType},
    {8, 0, "����ǯ�ͺ�:", CurrClampType}
};
static  SSetMenu ViewLineMenu = { 8, 16, 0, 0, ViewLineMenuItem };

static  SMenuItem ViewSteadyMenuItem[] = {      // VW_SteadyPara ��ָ̬��������ý���
    {11, 0x6, "�����ѹƫ��(%):", VoltWarp},
    {10, 0x6, "��ƽ����ֵ(%):", ImbalanceLimit},
    {9, 0x2, "���������ֵ:", NegativeILimit},
    {0, 0, "����г������ֵ��ѯ", VW_HmCurrLimit},
    {8, 0, "����ֵ����:", SetLimitType},
    {0, 0, "�Զ�������ֵ����", VW_CUSTOM},
    {0, 0, "��ѹ����", VW_Fluctuation},
    {0, 0, "��г��", VW_InterHarm},
    {0, 0, "����ʹ��", VW_AlmEnable},
};
static  SSetMenu ViewSteadyMenu = { 9, 18, 0, 0, ViewSteadyMenuItem };

static  SMenuItem ViewCommMenuItem[] = {       // VW_COMM ͨѶ�������ý���
    {6, 0x2, "��ַ��:", UnitNum},
    {6, 0x2, "������:", BaudRateSet},
    {8, 0x2, "IP ADDR:", IPAddrSet},
    {8, 0x2, "NET MASK:", NetMaskSet},
    {8, 0x2, "GATE WAY:", GateWaySet},
    {8, 0x6, "MAC ADDR:", SetMacAddr},
    {6, 0x2, "�˿ں�:", SocketServerPort},
};
static  SSetMenu ViewCommMenu = { 7, 18, 0, 0, ViewCommMenuItem };

static  SMenuItem ViewOtherMenuItem[] = {      // VW_OTHERSET �����������ý���
    {7, 0x6, "����ʱ��:", ModifyTime},
    {8, 0x6, "�޸�����", ModifyPasswd},
    {9, 0x2, "LCD�ر���ʱ:", LCMDelayTime},
    {0, 0, "��ʱ��������", VW_GpsParamSet},
    {0, 0, "���ػ�ʱ���ѯ", ShowOcTime},
    {0, 0, "CVT����", VW_CvtModify},
    //{0, 0, "�������ݵ�U��", VW_BakToUSB},
};
static  SSetMenu ViewOtherMenu = { 6, 18, 0, 0, ViewOtherMenuItem };

static  SMenuItem ViewRecWaveMenuItem[] = {      // VW_RecWavePara ¼���������ý���
    {0, 0, "��̬��ѹ������������", VW_ShortRMSVVR},
    {0, 0, "��̬����������������", VW_ShortRMSIVR},
    {0, 0, "��̬�¼�������������", VW_SteadyTrigPara},
    {9, 0, "�ֶ�¼��ʹ��:", TransManualRec},
    {9, 0x2, "�¼����ܲ���:", TransEndNum},
    {11, 0x2, "��̬���¼ʱ��:", TranstRcdTime},
    {9, 0, "¼�������ʽ:", kRecWaveFormat},
    {11, 0, "¼��������(Hz):", kRecWaveSmplRt},
    {11, 0, "COMTRADE����·��:", kComtradeSvPath}
};
static  SSetMenu ViewRecWaveMenu = { 9, 22, 0, 0, ViewRecWaveMenuItem };

static  SMenuItem ViewEEWMenuItem[] = {     // VW_EEW �����豸����Ԥ���������ý���
    {0, 0, "����Ԥ����ֵ����", VW_CapThresh},
    {9, 0, "��-����ʾг��:", VW_raDisHarm},
    {0, 0, "��-�������������", VW_raParam},
    {0, 0, "��-����ֵ��ѯ", VW_raThresh},
    {0, 0, "��ѹ����������", VW_TrnsfmrParam},
};
static  SSetMenu ViewEEWMenu = { 5, 19, 0, 0, ViewEEWMenuItem };

static  SMenuItem ViewHideMenuItem[] = {       // VW_HIDE ���ز������ý���
    {7, 0, "AD������:", AdSampleRate},
    {11, 0x2, "��������ʱ��(h):", ResetHysTime},
    {12, 0x2, "Harmonics_10cyc:", kHarmonic10Cyc},
    {6, 0, "ʱ��Դ:", kClockSource},
    {11, 0x2, "ϵͳʱ�����(ms):", kSysTimeError},
    {9, 0x0, "�̶�Ƶ�ʲ���:", kFixFreqSmpl},
    {9, 0x0, "61850�������:", kHeartbeat61850}
};
static  SSetMenu ViewHideMenu = { 7, 18, 0, 0, ViewHideMenuItem };

static  SMenuItem ViewVerifyMenuItem[] = {     // VW_VERIFY �����徫��У׼����
    {8, 0x2, "������ѹ��:", SmpVltScale},
    {8, 0x2, "����������:", SmpCurScale},
    {0, 0, "��������ϵ������", VW_SetResRatio},
    {9, 0x6, "��׼��ѹ/����:", VorCDatum},
    {9, 0x6, "���У׼ʹ��:", PhsAdjEnable},
    {11, 0x6, "��ѹ/����У׼ʹ��:", VorC_AdjEnable},
    {9, 0, "�����Զ�У׼", AutoAdj},
    {10, 0, "��ѹ���Զ�����", VW_VoltLinearity},
};
static  SSetMenu ViewVerifyMenu = { 8, 16, 0, 0, ViewVerifyMenuItem };

static  SMenuItem ViewSysparaMenuItem[] = {    // VW_SYSPARA ϵͳ�������ý���
    {0, 0, "�洢���ȡֵ��������", VW_RcdSpaceSaveType},
    {0, 0, "����ֱ��������ѯ", VW_SeeCharacterDCValue},
    {11, 0, "��ȡ����ֱ������", GetCharacterDCValue},
    {9, 0x2, "��̬��¼�޶�:", TranstTolTime},
    {11, 0, "Ƶ�ʲ������(s):", kFreqEvalTm},
    {13, 0, "time modify(ms):", kMdfyEvnStm},
    {0, 0x11, "Ԥ������3:", VW_SYSPARA_RSV2},
};
static  SSetMenu ViewSysparaMenu = { 7, 18, 0, 0, ViewSysparaMenuItem };

static  SMenuItem ViewSysInitMenuItem[] = {    // VW_INIT ϵͳ��ʼ������
    {7, 0x2, "�豸���:", DeviceSn},
    {8, 0, "�ָ���������", ResetDefaultPara},
    {9, 0, "��������ʹ��:", PstMdfyEnable},
    {8, 0, "����ǯʹ��:", CurrClampEnable},
    {7, 0, "��λ�û�����", ResetPassword},
    {6, 0, "ϵͳ��ʼ��", SysIni},
    {12, 0x6, "�������ź��ж���ֵ:", ZeroInputThr},
    {7, 0, "����ʹ��:", DebugEnable},
    {0, 0x11, "Ԥ������1:", VW_SYSINIT_RSV1},
};
static  SSetMenu ViewSysInitMenu = { 9, 18, 0, 0, ViewSysInitMenuItem };

static  SMenuItem ViewSysHideMenuItem[] = {    // VW_SYS_HIDE ϵͳ���ز������ý���
    {12, 0, "ʵʱ���ι�������:", AdjustZeroEnable},
    {7, 0, "����ʹ��:", PstEnable},
    {11, 0, "¼�����ݱ���ʹ��:", SaveWaveEnable},
    {13, 0x2, "г����¼��������(��):", kHarmRecSvMax},
    {11, 0, "г����¼����ʹ��:", kHarmRecSvEn},
    {12, 0, "10min����ʱ�����:", Aggregation10min},
    {9, 0x2, "��̬�¼��˲�:", kFilterCP},
};
static  SSetMenu ViewSysHideMenu = { 7, 18, 0, 0, ViewSysHideMenuItem };

static  SMenuItem ViewSysHide2MenuItem[] = {    // VW_SYS_HIDE2 ϵͳ���ز������ý���2
    {13, 0, "Զ�̷���ǿ�ư�ȫ���:", kSecurityEn},
    {10, 0, "����ʱ���(s):", kTimeDiff},
    {10, 0, "�źŷ���ʹ��:", kSignalSimuEn},
};
static  SSetMenu ViewSysHide2Menu = { 3, 18, 0, 0, ViewSysHide2MenuItem };

//------ <=3rd layer menu---------------------------------------------------------
static  SMenuItem ViewCustomMenuItem[] = {     // VW_CUSTOM �Զ�������ֵ���ý���
    {11, 0x6, "Ƶ��ƫ����ֵ(Hz):", FrequecyLimit},
    {7, 0x6, "������ֵ:", PstLimit},
    {0, 0, "����г����ֵ", VW_SetHarmLimit},
};
static  SSetMenu ViewCustomMenu = { 3, 18, 0, 0, ViewCustomMenuItem };

static  SMenuItem ViewFluctuationMenuItem[] = {     // VW_Fluctuation ��ѹ�����������ý���
    {11, 0x6, "��ѹ��������ʹ��:", FluctuationEna},
    {14, 0x2, "��ѹ����Deadband(%):", FluctuationDb},
};
static  SSetMenu ViewFluctuationMenu = { 2, 18, 0, 0, ViewFluctuationMenuItem };

static  SMenuItem ViewInterHarmMenuItem[] = {     // VW_InterHarm ��г���������ý���
    {8, 0x0, "��г��ʹ��:", kInterHarmEna},
    {13, 0x2, "��г��Deadband(%):", kInterHarmDb},
    {7, 0x0, "��г��Ⱥ:", kInterHmGroup},
};
static  SSetMenu ViewInterHarmMenu = { 3, 18, 0, 0, ViewInterHarmMenuItem };

static  SMenuItem ViewAlmEnableMenuItem[] = {     // VW_AlmEnable ����ʹ�����ý���
    {8, 0x0, "г������ʹ��:", kHarmAlmEna},
    {8, 0x2, "Ƶ�ʱ���ʹ��:", kFreqAlmEna},
    {8, 0x0, "���䱨��ʹ��:", kPstAlmEna},
    {10, 0x0, "��ƽ�ⱨ��ʹ��:", kUnblcAlmEna},
    {10, 0x0, "��ѹƫ���ʹ��:", kVoltDvAlmEna},
};
static  SSetMenu ViewAlmEnableMenu = { 5, 18, 0, 0, ViewAlmEnableMenuItem };

static  SMenuItem ViewCLimitMenuItem[] = {       // VW_HmCurrLimit ��������ֵ��ѯ����
    //{0,0, "г������     ����ֵ(A)", CLimitValue},
    {0, 0, "", CLimitValue},
    {0, 0, "", CLimitValue},
    {0, 0, "", CLimitValue},
    {0, 0, "", CLimitValue},
    {0, 0, "", CLimitValue},
    {0, 0, "", CLimitValue},
    {0, 0, "", CLimitValue},
    {0, 0, "", CLimitValue},
};
static  SSetMenu ViewCLimitMenu = { 8, 14, 49, 0, ViewCLimitMenuItem };

static  SMenuItem ViewShortVVRMenuItem[] = {     // VW_ShortRMSVVR ��̬��ѹ�����������ý���
    {9, 0x6, "������ʹ��:", TransEnable},
    {9, 0x6, "ͻ�䴥��ʹ��:", TransTbEnable},
    {9, 0x6, "��������ֵ(%):", TransLimit},
    {9, 0x6, "ͻ������ֵ(%):", TransTbLimit},
};
static  SSetMenu ViewShortVVRMenu = { 4, 18, 0, 0, ViewShortVVRMenuItem };

static  SMenuItem ViewShortIVRMenuItem[] = {     // VW_ShortRMSIVR ��̬���������������ý���
    {9, 0x6, "��������ʹ��:", StartCurEn},
    {11, 0x2, "������������(%):", StartCurLimit},
    {9, 0x6, "��̬����ʹ��:", TransCurEnable},
    {11, 0x6, "��̬��������(A):", TransCurLimit}
};
static  SSetMenu ViewShortIVRMenu = { 4, 19, 0, 0, ViewShortIVRMenuItem };

static  SMenuItem ViewStdyTrgMenuItem[] = {     // VW_SteadyTrigPara ��̬�¼������������ý���
    {9, 0x0, "�ܴ���ʹ��:", kStdyTrgrEnable},
    {9, 0x0, "Ƶ�ʴ���ʹ��:", kFreqTrgrEnable},
    {9, 0x6, "г������ʹ��:", kHarmTrgrEnable},
    {10, 0x6, "��ƽ�ⴥ��ʹ��:", kUnblcTrgrEnable},
    {11, 0x0, "��ѹƫ���ʹ��:", kVoltDvTrgrEnable}
};
static  SSetMenu ViewStdyTrgMenu = { 5, 16, 0, 0, ViewStdyTrgMenuItem };

static  SMenuItem ViewGPSMenuItem[] = {        // VW_GpsParamSet GPS�������ý���
#ifdef PQNet2xx_3xx
    {9, 0, "GPS�ź�����:", GpsSingleType},
    {9, 0, "GPS��������:", GpsPulseType},
    {13, 0x6, "�����ʱ���(��/��):", SetProofTimeIntr},
    {11, 0x2, "B���ʱ���(��):", SetBTimeIntr},
#elif defined PQAny3xx
    {9, 0x10, "GPS�ź�����:", GpsSingleType},
    {9, 0x10, "GPS��������:", GpsPulseType},
    {13, 0x10, "�����ʱ���(��/��):", SetProofTimeIntr},
    {11, 0x10, "B���ʱ���(��):", SetBTimeIntr},
#else //Equipment type
#endif
    {11, 0x2, "NTP Server IP:", NTPServerIP},
    //{12, 0x2, "NTP Server Port:", NTPServerPort},
    {8, 0x4, "TimeZone:", kTimeZone}
};
static  SSetMenu ViewGPSMenu = { 6, 18, 0, 0, ViewGPSMenuItem };

static  SMenuItem ViewCVTMenuItem[] = {        // VW_CvtModify CVT��������
    {9, 0, "CVT����ʹ��:", CvtModifyEn},
    {0, 0, "����CVT����ϵ��", VW_CvtModifyK}
};
static  SSetMenu ViewCVTMenu = { 2, 14, 0, 0, ViewCVTMenuItem };
static  SMenuItem ViewCVTKMenuItem[] = {       // VW_CvtModifyK CVT����ϵ�����ý���
    {6, 0x2, "", CvtModifyKSub},
    {6, 0x2, "", CvtModifyKSub},
    {6, 0x2, "", CvtModifyKSub},
    {6, 0x2, "", CvtModifyKSub},
    {6, 0x2, "", CvtModifyKSub},
    {6, 0x2, "", CvtModifyKSub},
    {6, 0x2, "", CvtModifyKSub},
    {6, 0x2, "", CvtModifyKSub},
};
static  SSetMenu ViewCVTKMenu = { 8, 14, 49, 0, ViewCVTKMenuItem };

static  SMenuItem ViewResRatioMenuItem[] = {   // VW_SetResRatio ��������ϵ�����ý���
    {8, 0x2, "A���ѹϵ��:", VAResRatio},
    {8, 0x2, "B���ѹϵ��:", VBResRatio},
    {8, 0x2, "C���ѹϵ��:", VCResRatio},
    {8, 0x2, "A�����ϵ��:", CAResRatio},
    {8, 0x2, "B�����ϵ��:", CBResRatio},
    {8, 0x2, "C�����ϵ��:", CCResRatio}
};
static  SSetMenu ViewResRatioMenu = { 6, 14, 0, 0, ViewResRatioMenuItem };

static  SMenuItem ViewLinearityMenuItem[] = {   // VW_VoltLinearity ��ѹ���Զ���������
    {10, 0x2, "A���ѹ����ϵ��:", VALineCoef},
    {10, 0x2, "B���ѹ����ϵ��:", VBLineCoef},
    {10, 0x2, "C���ѹ����ϵ��:", VCLineCoef},
};
static  SSetMenu ViewLinearityMenu = { 3, 18, 0, 0, ViewLinearityMenuItem };

static  SMenuItem ViewRcdSpaceMenuItem[] = {   // VW_RcdSpaceSaveType �洢�����ȡֵ�������ý���
    {9, 0x2, "г���洢���:", HarmRcdSpace},
    {9, 0x4, "г����¼ȡֵ:", HrmRcdSaveType},
    {9, 0x4, "Ƶ�ʴ洢���:", FreqRcdSpace},
    {9, 0x4, "Ƶ�ʼ�¼ȡֵ:", FreqSaveType},
    {11, 0x2, "��ѹƫ��洢���:", VoltWarpRcdSpace},
    {11, 0x4, "��ѹƫ���¼ȡֵ:", VoltWarpSaveType},
    {11, 0x2, "��ƽ��ȴ洢���:", ImbalanceRcdSpace},
    {11, 0x4, "��ƽ��ȼ�¼ȡֵ:", ImbalanceSaveType}
};
static  SSetMenu ViewRcdSpaceMenu = { 8, 16, 0, 0, ViewRcdSpaceMenuItem };

static  SMenuItem ViewDCValueMenuItem[] = {    // VW_SeeCharacterDCValue ����ֱ��������ѯ����
    {11, 0, "A���ѹֱ������:", VADCValue},
    {11, 0, "B���ѹֱ������:", VBDCValue},
    {11, 0, "C���ѹֱ������:", VCDCValue},
    {11, 0, "A�����ֱ������:", CADCValue},
    {11, 0, "B�����ֱ������:", CBDCValue},
    {11, 0, "C�����ֱ������:", CCDCValue}
};
static  SSetMenu ViewDCValueMenu = { 6, 14, 0, 0, ViewDCValueMenuItem };

static  SMenuItem ViewSetHlmtItem[] = {    // VW_SetHarmLimit ����г����ֵ����
    {5, 0x6, "", SetHarmLimitSub},
    {5, 0x6, "", SetHarmLimitSub},
    {5, 0x6, "", SetHarmLimitSub},
    {5, 0x6, "", SetHarmLimitSub},
    {5, 0x6, "", SetHarmLimitSub},
    {5, 0x6, "", SetHarmLimitSub},
    {5, 0x6, "", SetHarmLimitSub},
    {5, 0x6, "", SetHarmLimitSub},
};
static  SSetMenu ViewSetHlmtMenu = { 8, 16, 50, 0, ViewSetHlmtItem };

static  SMenuItem ViewCapThrMenuItem[] = {     // VW_CapThresh ����Ԥ����ֵ����
    {9, 0x6, "����Ԥ����ֵ:", kCapLifeThr},
    {10, 0x2, "24h�ۻ�Ԥ����ֵ:", kCapW24hThr},
    {10, 0x6, "��������ѹ1��ֵ:", kCapVolt1Thr},
    {10, 0x6, "��������ѹ2��ֵ:", kCapVolt2Thr},
    {10, 0x2, "������Ԥ����ֵ:", kCapCurrThr},
    {10, 0x2, "������Ԥ����ֵ:", kCapCapThr},
    {10, 0x2, "��ֵ����ѹ��ֵ", kCapPeakThr},
};
static  SSetMenu ViewCapThrMenu = { 7, 19, 0, 0, ViewCapThrMenuItem };

static  SMenuItem ViewraDisHmMenuItem[] = {       // VW_raDisHarm ��-����ʾг�����ý���
    {5, 0, "", kraDisHmSub},
    {5, 0, "", kraDisHmSub},
    {5, 0, "", kraDisHmSub},
    {5, 0, "", kraDisHmSub},
    {5, 0, "", kraDisHmSub},
    {5, 0, "", kraDisHmSub},
    {5, 0, "", kraDisHmSub},
    {5, 0, "", kraDisHmSub},
};
static  SSetMenu ViewraDisHmMenu = { 8, 10, 24, 0, ViewraDisHmMenuItem };

static  SMenuItem ViewraParamMenuItem[] = {     // VW_raParam ��-������������ý���
    {8, 0x2, "����ֵ Th:", kCapGamaThr},
    {10, 0x2, "�����·����Ms:", kTransformerMs},
    {10, 0x2, "���ݶ����Mc:", kCapNmnlMc},
    {10, 0x2, "���ݶ��ѹUc:", kCapNmnlUc},
    {11, 0x2, "�����翹���翹��:", kCapReactancex},
    {8, 0x10, "������г��:", kCapCharctHr},
    {10, 0x2, "��Դ�迹�нǦ�:", kImpedanceBeta},
};
static  SSetMenu ViewraParamMenu = { 7, 19, 0, 0, ViewraParamMenuItem };

static  SMenuItem ViewraThrMenuItem[] = {       // VW_raThresh ��-����ֵ��ѯ����
    {0, 0x10, "", kRAThreshold},
    {0, 0x10, "", kRAThreshold},
    {0, 0x10, "", kRAThreshold},
    {0, 0x10, "", kRAThreshold},
    {0, 0x10, "", kRAThreshold},
    {0, 0x10, "", kRAThreshold},
    {0, 0x10, "", kRAThreshold},
    {0, 0x10, "", kRAThreshold},
};
static  SSetMenu ViewraThrMenu = { 8, 18, 24, 0, ViewraThrMenuItem };

static  SMenuItem ViewTrnsfmrMenuItem[] = {     // VW_TrnsfmrParam ��ѹ����������
    {10, 0x2, "������ı���ֵ:", kTrnsfmrPec},
    {10, 0x2, "��ѹ�������:", kTrnsfmrIn},
};
static  SSetMenu ViewTrnsfmrMenu = { 2, 19, 0, 0, ViewTrnsfmrMenuItem };

static  const int ULevNum = 9;
static  const char ULevSet[ULevNum][8] = {"380V", "6kV", "10kV", "35kV", "66kV",
                                         "110kV", "220kV", "330kV", "500kV"
                                        };
static  const char TimeZoneSelect[25][4] = {"12", "11", "10", "9", "8", "7",
                              "6", "5", "4", "3", "2", "1", "0", "-1", "-2",
                              "-3", "-4", "-5", "-6", "-7", "-8", "-9", "-10", "-11", "-12" };
static  const char GPSTZTypeSelect[2][6] = {"utc", "local"};
static  const char AdSmplRtSlct[3][10] = {"1024/8", "2048/10", "4096/10"};
static  const char WvSmplRtSlct[2][6] = {"10240", "20480"}; //¼��������
static  const char FreqSaveSpc[6][10] = {"1", "3", "10", "60", "180", "600"};
static  const char FreqEvalTmSlct[2][4] = {"1", "10"};    //Ƶ�ʲ������. unit:s
static  const char ClockSourceSlct[2][7] = {"RTC", "System"};
static  const char ExtremaVoltSlct[2][4] = {"CPU", "DSP"};
static  const char kCmtrdSvPathSlct[2][24] = {"COMTRADE", "<LDName>/COMTRADE"}; 

static  const char FFTWidthSet[3][10] = {" 4���ܲ�", " 8���ܲ�", "10���ܲ�"};
static  const char HrmRcdTypeSet[2][10] = {"���ֵ", "ƽ��ֵ"};
static  const char HrmMaxTypeSet[2][10] = {"���ֵ", "��2��ֵ"};
static  const char LimitTypeSet[2][10] = {"����", "�Զ���"};
static  const char ProtocolTypeSet[2][10] = {"�Զ���A", "�Զ���B"};
static  const char kIntrHarmGrpt[2][20] = {"Group", "Centred Subgroup"};

static  const char kGpsSingleType[4][11] = {"B��+����", "B��", "����", "��"};
static  const char Gps_Pulse_Type[2][10] = {"������", "������"};

static  const char YesNo[2][4] = {"��", "��"};
static  const char NoYes[2][4] = {"��", "��"};
static  const char CheckBox[2][4] = {"�� ", "�� "};
static  const unsigned int month_day[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static  const char Bak2USBResult[][32] = {"������ϣ�", "û���ҵ�USB�豸��", "�ҽ�USB�豸ʧ�ܣ�",
                                         "����Ŀ¼usb/boyuuxʧ�ܣ�", "��Ч���������ͣ�", "������̬����ʧ�ܣ�",
                                         "������̬����ʧ�ܣ�", "ж��USB�豸ʧ�ܣ�", "��ѡ��Ҫ���ݵ��������ͣ�"
                                        };
static  const char CnctTpSlct[2][4] = {"Y0", "VV"}; //���߷�ʽѡ��
static  const char RcdWvSavFmt[2][12] = {"Boyuu", "ComTraDE"}; //Record wave data save format

static  const char MdfyPassVW[][12] = {   //�����������
    "    ������:",
    "    ������:",
    "ȷ��������:"
};

#endif // VIEWSET_MENU_H 


