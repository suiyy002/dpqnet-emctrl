#ifndef VIEWSET_MENU_H
#define VIEWSET_MENU_H

typedef enum {
  //界面
    //设置界面
    VW_S0,          //主界面
    VW_SY0,         //系统主界面
    VW_AUDIT,       //审计主界面
    VW_SteadyPara,  //稳态指标参数设置界面
    VW_LINE,        //线路参数设置
    VW_RecWavePara, //录波参数设置
    VW_COMM,        //通讯参数设置
    VW_OTHERSET,    //其它参数设置
    VW_CUSTOM,      //自定义限值设置
    VW_EEW,         //电气设备运行预警
    VW_RESERVE1,    //预留设置1
    VW_HIDE,        //隐藏设置界面
    //系统设置界面
    VW_VERIFY,      //采样板精度校准界面
    VW_SYSPARA,     //系统参数设置界面
    VW_INIT,        //系统初始化界面
    VW_SYS_RSV1,    //系统预留设置1
    VW_SYS_RSV2,    //系统预留设置2
    VW_SYS_RSV3,    //系统预留设置3
    VW_SYS_HIDE,    //系统隐藏设置
    VW_SYS_HIDE2,    //系统隐藏设置2
    //审计界面
    VW_AuditWarn,   //告警事件查询界面
    VW_AuditAlarm,  //警报事件查询界面

    VW_ShortRMSVVR,  //暂态电压触发参数设置界面 Short-duration rms voltage variations
    VW_ShortRMSIVR,  //暂态电流触发参数设置界面 Short-duration rms current variations
    VW_GpsParamSet,     //GPS参数设置界面
    VW_CvtModify,       //CVT 修正界面
    VW_SetResRatio,     //设置电阻系数界面
    VW_VoltLinearity,     //电压线性度修正界面面
    VW_RcdSpaceSaveType,    //取值类型及存储间隔设置界面
    VW_SeeCharacterDCValue, // 查看特征直流分量.
    VW_SYSPARA_RSV1,    //系统参数设置界面预留设置1
    VW_SYSPARA_RSV2,    //系统参数设置界面预留设置2
    VW_SYSINIT_RSV1,    //系统初始化界面预留设置1
    VW_SetHarmLimit,    //设置谐波限值
    VW_CvtModifyK,      //CVT 修正系数设置界面
    VW_HmCurrLimit,     //电流谐波允许值查询界面
    VW_CapThresh,     //电容预警阈值设置界面
    VW_raDisHarm,   //γ-α显示谐波的设置界面
    VW_raParam,    //γ-α计算参数设置界面
    VW_raThresh,     //γ-α阈值查询界面
    VW_TrnsfmrParam,    //Transformer param set view
    VW_SteadyTrigPara,  //稳态事件触发参数设置界面
    VW_Fluctuation,     //电压波动参数设置界面
    VW_InterHarm,       //间谐波参数设置界面
    VW_AlmEnable,       //报警使能设置界面
    VW_LOGIN,       //登录界面
    VW_GUI_DILIMITER,   //---- 通用界面分界符 ----------------------------------


    //命令
    kUserName,
    kPassword,
    VoltWarp,       // 允许电压偏差.
    ShortCap, UserCap, SuppCap,     // 最小短路容量、用电协议容量、供电设备容量.
    CLimitValue,    // 电流谐波限值
    TransLimit,     // 暂态门限值.
    TransCurLimit,  //电流暂态门限值.
    PTScale, CTScale,   // PT变比、CT变比.
    VoltLvl,        // 电压等级.
    UnitNum,        // 设置单元号.
    BaudRateSet,    // 设置串口波特率.
    TransEnable,    // 暂态监测使能.
    TransCurEnable, //电流暂态监测使能.
    DeviceSn,       // 设备序列号.
    SmpVltScale, SmpCurScale,       // 采样电压变比、采样电流变比.
    TranstTolTime, TranstRcdTime,   // 暂态事件总记录时间、一个事件最大记录时间
    kFreqEvalTm,    //频率测量间隔
    kMdfyEvnStm,    //事件开始时间校准系数
    SysIni,         // 系统初始化.
    ModifyTime,     // 设置时间.
    ModifyPasswd,   // 修改密码.
    IPAddrSet, SocketServerPort,    // 设置IP地址、端口号
    NetMaskSet,     // 设置子网掩码.
    DebugEnable,    // 程序调试使能.
    VAResRatio, VBResRatio, VCResRatio, // A、B、C相电压电阻系数.
    CAResRatio, CBResRatio, CCResRatio, // A、B、C相电流电阻系数.
    VALineCoef, VBLineCoef, VCLineCoef, // A、B、C相电压线性度系数.
    VorCDatum,      //精度校准基准电压.
    PhsAdjEnable,   //精度校准相别使能.
    VorC_AdjEnable, //精度校准电压电流使能.
    AutoAdj,        //精度自动校准.
    kVoltLinearity, //电压线性度修正
    PstMdfyEnable,  //闪变修正使能.
    PstEnable,      //闪变使能.
    kSecurityEn,    //是否强制执行远程访问安全检查
    SaveWaveEnable, //录波数据保存使能
    Aggregation10min,   //每10分钟整点重新开始计算
    GateWaySet,     //设置网关.
    CurrClampEnable,    // 电流钳使能.
    CurrClampType,  // 设置电流钳型号.
    LCMDelayTime,   // 设置LCM电源关闭等待时间.
    HarmRcdSpace,   // 设置谐波数据存储间隔
    ResetPassword,  // 复位用户密码
    HrmRcdSaveType, // 谐波记录存储类型
    ResetDefaultPara,   //恢复出厂设置
    AdjustZeroEnable,       //主板侧实时波形过零点调节使能
    SetLimitType,   //设置超限值类型
    ImbalanceLimit, //设置三相不平衡超限值
    NegativeILimit, //设置零序电流限值
    FrequecyLimit,  //设置频率限值
    PstLimit,       //设置闪变限值
    SetHarmLimitSub,    //设置谐波限值子界面
    SetMacAddr,     // 设置硬件地址.
    GetCharacterDCValue,    // 获取特征直流分量.
    VADCValue, CADCValue,   // A相电压,电流特征直流分量 
    VBDCValue, CBDCValue,   // B相电压,电流特征直流分量 
    VCDCValue, CCDCValue,   // C相电压,电流特征直流分量 
    SteadyBakEnable,    //稳态数据备份使能
    TranstBakEnable,    //暂态数据备份使能
    ConnectType,    //接线方式
    SynTimeType,    //时钟自动同步方式
    AdSampleRate,   //AD采样率1024/8=0,2048/10=1,4096/10=2
    TransTbLimit,   //暂态突变门限值
    StartCurEn,     //启动电流使能
    StartCurLimit,  //启动电流限值
    TransManualRec, //手动录波
    TransTbEnable,
    TransEndNum,
//  SetProofTimeType,   //设置GPS对时类型
    GpsSingleType,      //m 信号类型
    GpsPulseType,       //m 脉冲类型
    SetProofTimeIntr,   //设置GPS-PULSE对时间隔
    SetBTimeIntr,       //设置GPS-B对时间隔
    NTPServerIP,        //NTP Server IP
    ImbalanceRcdSpace,  //不平衡记录存储间隔
    ImbalanceSaveType,  //不平衡记录存储类型
    FreqRcdSpace,       //频率记录存储间隔
    FreqSaveType,       //频率记录存储类型
    VoltWarpRcdSpace,   //电压偏差存储间隔
    VoltWarpSaveType,   //电压偏差存储类型
    ShowOcTime,         //
    ZeroInputThr,      //Zero input threshold
    kFilterCP,
    kTimeDiff,      //permissible time difference
    kSignalSimuEn,  //signal simulation enable
    
    CvtModifyEn,    //CVT 修正使能
    CvtModifyKSub,  //设置CVT 修正系数子界面
    FluctuationDb,  //Voltage fluctuation deadband
    FluctuationEna, //Voltage fluctuation measurement enable
    ResetHysTime,   //Reset Hysteresis time. unit:hours
    kTimeZone,
    kHarmonic10Cyc, //10 cycle value correspond to harmonic order      
    kClockSource,   //Clock source
    kSysTimeError,  //system time error
    kFixFreqSmpl,   //fix frequency sample
    kCapLifeThr,    //寿命预警
    kCapW24hThr,    //24h 累积预警
    kCapVolt1Thr,   //持续过电压预警1
    kCapVolt2Thr,   //持续过电压预警2
    kCapCurrThr,    //过电流预警
    kCapCapThr,     //过容限预警
    kCapPeakThr,    //峰值过电压
    kraDisHmSub,    //γ-α显示谐波设置子界面
    kCapGamaThr,    //γ 阈值 Th
    kTransformerMs, //主变短路容量 Ms
    kCapNmnlMc,     //电容额定容量 Mc
    kCapNmnlUc,     //电容额定电压 Uc
    kCapReactancex, //串联电抗器电抗率
    kCapCharctHr,   //主特征谐波
    kImpedanceBeta, //电源阻抗夹角β
    kRAThreshold,   //γ-α阈值
    kTrnsfmrPec,    //pu of Eddy Current loss
    kTrnsfmrIn,     //I_N of transformer
    kRecWaveFormat, //Record wave data save format
    kRecWaveSmplRt, //Record wave sampling rate
    kComtradeSvPath,    //COMTRADE file save path
    kStdyTrgrEnable,    //设置稳态录波触发使能
    kFreqTrgrEnable,    //频率录波触发使能   
    kHarmTrgrEnable,    //谐波录波触发使能
    kUnblcTrgrEnable,   //不平衡录波触发使能
    kVoltDvTrgrEnable,  //电压偏差录波触发使能
    kHarmRecSvMax,  //谐波记录最大保存数量
    kHarmRecSvEn,   //谐波记录保存使能
    kInterHarmEna,  //间谐波使能
    kInterHarmDb,   //间谐波deadband
    kInterHmGroup,  //间谐波群取值类型
    kHeartbeat61850, //61850 server heartbeat monitor
    kAuditLogSize,  //Audit log record file maximum size
    kAuditWarnList, //Audit log warning record list
    kHarmAlmEna,
    kFreqAlmEna,
    kPstAlmEna,
    kUnblcAlmEna,
    kVoltDvAlmEna,
    
} ViewSetCmd;

//菜单项数据结构
struct SMenuItem { //菜单项结构
    int val_x;      //菜单项设置值的起始横坐标(单位:汉字个数)
    int tag;        //bit0(0=显示该项，1=不显示); bit1(0=不显示光标,1=显示)
    //bit2(0=单个值,1=多个值); bit3(0=显示序号, 1=不显示)
    //bit4(0=Enable,1=Disable);
    char name[24];  //菜单项名称
    int cmd;        //对应处理命令编号
};

struct SSetMenu { //设置菜单结构
    int count;  //每页菜单项的数目
    int width;  //最大宽度(单位:汉字个数)
    int totl;   //菜单项的总数，如为0,表示只有1页
    int tag;    //根据实际情况灵活运用,
     SMenuItem * pmenu_item; //菜单项
};

static  SMenuItem ViewLOGINMenuItem[] = {  //VW_LOGIN 登录界面
    {8, 0,   "USER NAME:", kUserName},
    {8, 0x2, "PASSWORD :", kPassword}
};
static  SSetMenu ViewLOGINMenu = { 2, 14, 0, 0, ViewLOGINMenuItem };

//------ 1st layer menu---------------------------------------------------------
static  SMenuItem ViewS0MenuItem[] = {
    {0, 0, "线路参数设置", VW_LINE},
    {0, 0, "稳态指标参数设置", VW_SteadyPara},
    {0, 0, "通讯参数设置", VW_COMM},
    {0, 0, "其他功能参数", VW_OTHERSET},
    {0, 0, "录波参数设置", VW_RecWavePara},
    {0, 0, "EEW 参数设置", VW_EEW},
    {0, 0x11, "预留设置1", VW_RESERVE1},
    {0, 0x11, "预留设置1", VW_RESERVE1},
    {0, 0x1, "隐藏设置", VW_HIDE}
};
static  SSetMenu ViewS0Menu = { 9, 12, 0, 0, ViewS0MenuItem };

static  SMenuItem ViewSy0MenuItem[] = {
    {0, 0, "采样板精度校准", VW_VERIFY},
    {0, 0, "系统参数设置", VW_SYSPARA},
    {0, 0, "系统初始化", VW_INIT},
    {0, 0, "其他功能参数", VW_OTHERSET},
    {0, 0x11, "系统预留设置1", VW_SYS_RSV1},
    {0, 0x11, "系统预留设置2", VW_SYS_RSV2},
    {0, 0x11, "系统预留设置3", VW_SYS_RSV3},
    {0, 0x1, "系统隐藏设置", VW_SYS_HIDE},
    {0, 0x1, "系统隐藏设置2", VW_SYS_HIDE2}
};
static  SSetMenu ViewSy0Menu = { 9, 12, 0, 0, ViewSy0MenuItem };

static  SMenuItem ViewAuditMenuItem[] = {
    {0, 0, "告警事件查询", VW_AuditWarn},
    {0, 0, "警报事件查询", VW_AuditAlarm},
    {10, 0x2, "日志文件大小(k):", kAuditLogSize},
    {0, 0x11, "系统预留设置1", VW_SYS_RSV1},
    {0, 0x11, "系统预留设置2", VW_SYS_RSV2},
    {0, 0x11, "系统预留设置3", VW_SYS_RSV3},
};
static  SSetMenu ViewAuditMenu = { 6, 12, 0, 0, ViewAuditMenuItem };

//------ 2nd layer menu---------------------------------------------------------
static  SMenuItem ViewLineMenuItem[] = {       // VW_LINE 线路参数设置界面
    {7, 0x6, "PT 变比:", PTScale},
    {7, 0x6, "CT 变比:", CTScale},
    {7, 0, "电压等级:", VoltLvl},
    {9, 0x2, "最小短路容量:", ShortCap},
    {9, 0x2, "用电协议容量:", UserCap},
    {9, 0x2, "供电设备容量:", SuppCap},
    {9, 0, "PT 接线方式:", ConnectType},
    {8, 0, "电流钳型号:", CurrClampType}
};
static  SSetMenu ViewLineMenu = { 8, 16, 0, 0, ViewLineMenuItem };

static  SMenuItem ViewSteadyMenuItem[] = {      // VW_SteadyPara 稳态指标参数设置界面
    {11, 0x6, "允许电压偏差(%):", VoltWarp},
    {10, 0x6, "不平衡限值(%):", ImbalanceLimit},
    {9, 0x2, "负序电流限值:", NegativeILimit},
    {0, 0, "电流谐波允许值查询", VW_HmCurrLimit},
    {8, 0, "门限值类型:", SetLimitType},
    {0, 0, "自定义门限值设置", VW_CUSTOM},
    {0, 0, "电压波动", VW_Fluctuation},
    {0, 0, "间谐波", VW_InterHarm},
    {0, 0, "报警使能", VW_AlmEnable},
};
static  SSetMenu ViewSteadyMenu = { 9, 18, 0, 0, ViewSteadyMenuItem };

static  SMenuItem ViewCommMenuItem[] = {       // VW_COMM 通讯参数设置界面
    {6, 0x2, "地址号:", UnitNum},
    {6, 0x2, "波特率:", BaudRateSet},
    {8, 0x2, "IP ADDR:", IPAddrSet},
    {8, 0x2, "NET MASK:", NetMaskSet},
    {8, 0x2, "GATE WAY:", GateWaySet},
    {8, 0x6, "MAC ADDR:", SetMacAddr},
    {6, 0x2, "端口号:", SocketServerPort},
};
static  SSetMenu ViewCommMenu = { 7, 18, 0, 0, ViewCommMenuItem };

static  SMenuItem ViewOtherMenuItem[] = {      // VW_OTHERSET 其他参数设置界面
    {7, 0x6, "设置时间:", ModifyTime},
    {8, 0x6, "修改密码", ModifyPasswd},
    {9, 0x2, "LCD关闭延时:", LCMDelayTime},
    {0, 0, "对时参数设置", VW_GpsParamSet},
    {0, 0, "开关机时间查询", ShowOcTime},
    {0, 0, "CVT修正", VW_CvtModify},
    //{0, 0, "备份数据到U盘", VW_BakToUSB},
};
static  SSetMenu ViewOtherMenu = { 6, 18, 0, 0, ViewOtherMenuItem };

static  SMenuItem ViewRecWaveMenuItem[] = {      // VW_RecWavePara 录波参数设置界面
    {0, 0, "暂态电压触发参数设置", VW_ShortRMSVVR},
    {0, 0, "暂态电流触发参数设置", VW_ShortRMSIVR},
    {0, 0, "稳态事件触发参数设置", VW_SteadyTrigPara},
    {9, 0, "手动录波使能:", TransManualRec},
    {9, 0x2, "事件后周波数:", TransEndNum},
    {11, 0x2, "暂态最长记录时间:", TranstRcdTime},
    {9, 0, "录波保存格式:", kRecWaveFormat},
    {11, 0, "录波采样率(Hz):", kRecWaveSmplRt},
    {11, 0, "COMTRADE保存路径:", kComtradeSvPath}
};
static  SSetMenu ViewRecWaveMenu = { 9, 22, 0, 0, ViewRecWaveMenuItem };

static  SMenuItem ViewEEWMenuItem[] = {     // VW_EEW 电气设备运行预警参数设置界面
    {0, 0, "电容预警阈值设置", VW_CapThresh},
    {9, 0, "γ-α显示谐波:", VW_raDisHarm},
    {0, 0, "γ-α计算参数设置", VW_raParam},
    {0, 0, "γ-α阈值查询", VW_raThresh},
    {0, 0, "变压器参数设置", VW_TrnsfmrParam},
};
static  SSetMenu ViewEEWMenu = { 5, 19, 0, 0, ViewEEWMenuItem };

static  SMenuItem ViewHideMenuItem[] = {       // VW_HIDE 隐藏参数设置界面
    {7, 0, "AD采样率:", AdSampleRate},
    {11, 0x2, "重启迟滞时间(h):", ResetHysTime},
    {12, 0x2, "Harmonics_10cyc:", kHarmonic10Cyc},
    {6, 0, "时钟源:", kClockSource},
    {11, 0x2, "系统时钟误差(ms):", kSysTimeError},
    {9, 0x0, "固定频率采样:", kFixFreqSmpl},
    {9, 0x0, "61850心跳监测:", kHeartbeat61850}
};
static  SSetMenu ViewHideMenu = { 7, 18, 0, 0, ViewHideMenuItem };

static  SMenuItem ViewVerifyMenuItem[] = {     // VW_VERIFY 采样板精度校准界面
    {8, 0x2, "采样电压比:", SmpVltScale},
    {8, 0x2, "采样电流比:", SmpCurScale},
    {0, 0, "采样电阻系数设置", VW_SetResRatio},
    {9, 0x6, "基准电压/电流:", VorCDatum},
    {9, 0x6, "相别校准使能:", PhsAdjEnable},
    {11, 0x6, "电压/电流校准使能:", VorC_AdjEnable},
    {9, 0, "精度自动校准", AutoAdj},
    {10, 0, "电压线性度修正", VW_VoltLinearity},
};
static  SSetMenu ViewVerifyMenu = { 8, 16, 0, 0, ViewVerifyMenuItem };

static  SMenuItem ViewSysparaMenuItem[] = {    // VW_SYSPARA 系统参数设置界面
    {0, 0, "存储间隔取值类型设置", VW_RcdSpaceSaveType},
    {0, 0, "特征直流分量查询", VW_SeeCharacterDCValue},
    {11, 0, "获取特征直流分量", GetCharacterDCValue},
    {9, 0x2, "暂态记录限额:", TranstTolTime},
    {11, 0, "频率测量间隔(s):", kFreqEvalTm},
    {13, 0, "time modify(ms):", kMdfyEvnStm},
    {0, 0x11, "预留设置3:", VW_SYSPARA_RSV2},
};
static  SSetMenu ViewSysparaMenu = { 7, 18, 0, 0, ViewSysparaMenuItem };

static  SMenuItem ViewSysInitMenuItem[] = {    // VW_INIT 系统初始化界面
    {7, 0x2, "设备编号:", DeviceSn},
    {8, 0, "恢复出厂设置", ResetDefaultPara},
    {9, 0, "闪变修正使能:", PstMdfyEnable},
    {8, 0, "电流钳使能:", CurrClampEnable},
    {7, 0, "复位用户密码", ResetPassword},
    {6, 0, "系统初始化", SysIni},
    {12, 0x6, "零输入信号判断阈值:", ZeroInputThr},
    {7, 0, "调试使能:", DebugEnable},
    {0, 0x11, "预留设置1:", VW_SYSINIT_RSV1},
};
static  SSetMenu ViewSysInitMenu = { 9, 18, 0, 0, ViewSysInitMenuItem };

static  SMenuItem ViewSysHideMenuItem[] = {    // VW_SYS_HIDE 系统隐藏参数设置界面
    {12, 0, "实时波形过零点调节:", AdjustZeroEnable},
    {7, 0, "闪变使能:", PstEnable},
    {11, 0, "录波数据保存使能:", SaveWaveEnable},
    {13, 0x2, "谐波记录保存数量(条):", kHarmRecSvMax},
    {11, 0, "谐波记录保存使能:", kHarmRecSvEn},
    {12, 0, "10min数据时标调整:", Aggregation10min},
    {9, 0x2, "暂态事件滤波:", kFilterCP},
};
static  SSetMenu ViewSysHideMenu = { 7, 18, 0, 0, ViewSysHideMenuItem };

static  SMenuItem ViewSysHide2MenuItem[] = {    // VW_SYS_HIDE2 系统隐藏参数设置界面2
    {13, 0, "远程访问强制安全检查:", kSecurityEn},
    {10, 0, "允许时间差(s):", kTimeDiff},
    {10, 0, "信号仿真使能:", kSignalSimuEn},
};
static  SSetMenu ViewSysHide2Menu = { 3, 18, 0, 0, ViewSysHide2MenuItem };

//------ <=3rd layer menu---------------------------------------------------------
static  SMenuItem ViewCustomMenuItem[] = {     // VW_CUSTOM 自定义门限值设置界面
    {11, 0x6, "频率偏差限值(Hz):", FrequecyLimit},
    {7, 0x6, "闪变限值:", PstLimit},
    {0, 0, "设置谐波限值", VW_SetHarmLimit},
};
static  SSetMenu ViewCustomMenu = { 3, 18, 0, 0, ViewCustomMenuItem };

static  SMenuItem ViewFluctuationMenuItem[] = {     // VW_Fluctuation 电压波动参数设置界面
    {11, 0x6, "电压波动测量使能:", FluctuationEna},
    {14, 0x2, "电压波动Deadband(%):", FluctuationDb},
};
static  SSetMenu ViewFluctuationMenu = { 2, 18, 0, 0, ViewFluctuationMenuItem };

static  SMenuItem ViewInterHarmMenuItem[] = {     // VW_InterHarm 间谐波参数设置界面
    {8, 0x0, "间谐波使能:", kInterHarmEna},
    {13, 0x2, "间谐波Deadband(%):", kInterHarmDb},
    {7, 0x0, "间谐波群:", kInterHmGroup},
};
static  SSetMenu ViewInterHarmMenu = { 3, 18, 0, 0, ViewInterHarmMenuItem };

static  SMenuItem ViewAlmEnableMenuItem[] = {     // VW_AlmEnable 报警使能设置界面
    {8, 0x0, "谐波报警使能:", kHarmAlmEna},
    {8, 0x2, "频率报警使能:", kFreqAlmEna},
    {8, 0x0, "闪变报警使能:", kPstAlmEna},
    {10, 0x0, "不平衡报警使能:", kUnblcAlmEna},
    {10, 0x0, "电压偏差报警使能:", kVoltDvAlmEna},
};
static  SSetMenu ViewAlmEnableMenu = { 5, 18, 0, 0, ViewAlmEnableMenuItem };

static  SMenuItem ViewCLimitMenuItem[] = {       // VW_HmCurrLimit 电流允许值查询界面
    //{0,0, "谐波次数     允许值(A)", CLimitValue},
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

static  SMenuItem ViewShortVVRMenuItem[] = {     // VW_ShortRMSVVR 暂态电压触发参数设置界面
    {9, 0x6, "常规监测使能:", TransEnable},
    {9, 0x6, "突变触发使能:", TransTbEnable},
    {9, 0x6, "常规门限值(%):", TransLimit},
    {9, 0x6, "突变门限值(%):", TransTbLimit},
};
static  SSetMenu ViewShortVVRMenu = { 4, 18, 0, 0, ViewShortVVRMenuItem };

static  SMenuItem ViewShortIVRMenuItem[] = {     // VW_ShortRMSIVR 暂态电流触发参数设置界面
    {9, 0x6, "启动电流使能:", StartCurEn},
    {11, 0x2, "启动电流门限(%):", StartCurLimit},
    {9, 0x6, "暂态电流使能:", TransCurEnable},
    {11, 0x6, "暂态电流门限(A):", TransCurLimit}
};
static  SSetMenu ViewShortIVRMenu = { 4, 19, 0, 0, ViewShortIVRMenuItem };

static  SMenuItem ViewStdyTrgMenuItem[] = {     // VW_SteadyTrigPara 稳态事件触发参数设置界面
    {9, 0x0, "总触发使能:", kStdyTrgrEnable},
    {9, 0x0, "频率触发使能:", kFreqTrgrEnable},
    {9, 0x6, "谐波触发使能:", kHarmTrgrEnable},
    {10, 0x6, "不平衡触发使能:", kUnblcTrgrEnable},
    {11, 0x0, "电压偏差触发使能:", kVoltDvTrgrEnable}
};
static  SSetMenu ViewStdyTrgMenu = { 5, 16, 0, 0, ViewStdyTrgMenuItem };

static  SMenuItem ViewGPSMenuItem[] = {        // VW_GpsParamSet GPS参数设置界面
#ifdef PQNet2xx_3xx
    {9, 0, "GPS信号类型:", GpsSingleType},
    {9, 0, "GPS脉冲类型:", GpsPulseType},
    {13, 0x6, "脉冲对时间隔(分/秒):", SetProofTimeIntr},
    {11, 0x2, "B码对时间隔(秒):", SetBTimeIntr},
#elif defined PQAny3xx
    {9, 0x10, "GPS信号类型:", GpsSingleType},
    {9, 0x10, "GPS脉冲类型:", GpsPulseType},
    {13, 0x10, "脉冲对时间隔(分/秒):", SetProofTimeIntr},
    {11, 0x10, "B码对时间隔(秒):", SetBTimeIntr},
#else //Equipment type
#endif
    {11, 0x2, "NTP Server IP:", NTPServerIP},
    //{12, 0x2, "NTP Server Port:", NTPServerPort},
    {8, 0x4, "TimeZone:", kTimeZone}
};
static  SSetMenu ViewGPSMenu = { 6, 18, 0, 0, ViewGPSMenuItem };

static  SMenuItem ViewCVTMenuItem[] = {        // VW_CvtModify CVT修正界面
    {9, 0, "CVT修正使能:", CvtModifyEn},
    {0, 0, "设置CVT修正系数", VW_CvtModifyK}
};
static  SSetMenu ViewCVTMenu = { 2, 14, 0, 0, ViewCVTMenuItem };
static  SMenuItem ViewCVTKMenuItem[] = {       // VW_CvtModifyK CVT修正系数设置界面
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

static  SMenuItem ViewResRatioMenuItem[] = {   // VW_SetResRatio 采样电阻系数设置界面
    {8, 0x2, "A相电压系数:", VAResRatio},
    {8, 0x2, "B相电压系数:", VBResRatio},
    {8, 0x2, "C相电压系数:", VCResRatio},
    {8, 0x2, "A相电流系数:", CAResRatio},
    {8, 0x2, "B相电流系数:", CBResRatio},
    {8, 0x2, "C相电流系数:", CCResRatio}
};
static  SSetMenu ViewResRatioMenu = { 6, 14, 0, 0, ViewResRatioMenuItem };

static  SMenuItem ViewLinearityMenuItem[] = {   // VW_VoltLinearity 电压线性度修正界面
    {10, 0x2, "A相电压线性系数:", VALineCoef},
    {10, 0x2, "B相电压线性系数:", VBLineCoef},
    {10, 0x2, "C相电压线性系数:", VCLineCoef},
};
static  SSetMenu ViewLinearityMenu = { 3, 18, 0, 0, ViewLinearityMenuItem };

static  SMenuItem ViewRcdSpaceMenuItem[] = {   // VW_RcdSpaceSaveType 存储间隔及取值类型设置界面
    {9, 0x2, "谐波存储间隔:", HarmRcdSpace},
    {9, 0x4, "谐波记录取值:", HrmRcdSaveType},
    {9, 0x4, "频率存储间隔:", FreqRcdSpace},
    {9, 0x4, "频率记录取值:", FreqSaveType},
    {11, 0x2, "电压偏差存储间隔:", VoltWarpRcdSpace},
    {11, 0x4, "电压偏差记录取值:", VoltWarpSaveType},
    {11, 0x2, "不平衡度存储间隔:", ImbalanceRcdSpace},
    {11, 0x4, "不平衡度记录取值:", ImbalanceSaveType}
};
static  SSetMenu ViewRcdSpaceMenu = { 8, 16, 0, 0, ViewRcdSpaceMenuItem };

static  SMenuItem ViewDCValueMenuItem[] = {    // VW_SeeCharacterDCValue 特征直流分量查询界面
    {11, 0, "A相电压直流分量:", VADCValue},
    {11, 0, "B相电压直流分量:", VBDCValue},
    {11, 0, "C相电压直流分量:", VCDCValue},
    {11, 0, "A相电流直流分量:", CADCValue},
    {11, 0, "B相电流直流分量:", CBDCValue},
    {11, 0, "C相电流直流分量:", CCDCValue}
};
static  SSetMenu ViewDCValueMenu = { 6, 14, 0, 0, ViewDCValueMenuItem };

static  SMenuItem ViewSetHlmtItem[] = {    // VW_SetHarmLimit 设置谐波限值界面
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

static  SMenuItem ViewCapThrMenuItem[] = {     // VW_CapThresh 电容预警限值设置
    {9, 0x6, "寿命预警阈值:", kCapLifeThr},
    {10, 0x2, "24h累积预警阈值:", kCapW24hThr},
    {10, 0x6, "持续过电压1阈值:", kCapVolt1Thr},
    {10, 0x6, "持续过电压2阈值:", kCapVolt2Thr},
    {10, 0x2, "过电流预警阈值:", kCapCurrThr},
    {10, 0x2, "过容限预警阈值:", kCapCapThr},
    {10, 0x2, "峰值过电压阈值", kCapPeakThr},
};
static  SSetMenu ViewCapThrMenu = { 7, 19, 0, 0, ViewCapThrMenuItem };

static  SMenuItem ViewraDisHmMenuItem[] = {       // VW_raDisHarm γ-α显示谐波设置界面
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

static  SMenuItem ViewraParamMenuItem[] = {     // VW_raParam γ-α计算参数设置界面
    {8, 0x2, "γ阈值 Th:", kCapGamaThr},
    {10, 0x2, "主变短路容量Ms:", kTransformerMs},
    {10, 0x2, "电容额定容量Mc:", kCapNmnlMc},
    {10, 0x2, "电容额定电压Uc:", kCapNmnlUc},
    {11, 0x2, "串联电抗器电抗率:", kCapReactancex},
    {8, 0x10, "主特征谐波:", kCapCharctHr},
    {10, 0x2, "电源阻抗夹角β:", kImpedanceBeta},
};
static  SSetMenu ViewraParamMenu = { 7, 19, 0, 0, ViewraParamMenuItem };

static  SMenuItem ViewraThrMenuItem[] = {       // VW_raThresh γ-α阈值查询界面
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

static  SMenuItem ViewTrnsfmrMenuItem[] = {     // VW_TrnsfmrParam 变压器参数设置
    {10, 0x2, "涡流损耗标幺值:", kTrnsfmrPec},
    {10, 0x2, "变压器额定电流:", kTrnsfmrIn},
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
static  const char WvSmplRtSlct[2][6] = {"10240", "20480"}; //录波采样率
static  const char FreqSaveSpc[6][10] = {"1", "3", "10", "60", "180", "600"};
static  const char FreqEvalTmSlct[2][4] = {"1", "10"};    //频率测量间隔. unit:s
static  const char ClockSourceSlct[2][7] = {"RTC", "System"};
static  const char ExtremaVoltSlct[2][4] = {"CPU", "DSP"};
static  const char kCmtrdSvPathSlct[2][24] = {"COMTRADE", "<LDName>/COMTRADE"}; 

static  const char FFTWidthSet[3][10] = {" 4个周波", " 8个周波", "10个周波"};
static  const char HrmRcdTypeSet[2][10] = {"最大值", "平均值"};
static  const char HrmMaxTypeSet[2][10] = {"最大值", "第2大值"};
static  const char LimitTypeSet[2][10] = {"国标", "自定义"};
static  const char ProtocolTypeSet[2][10] = {"自定义A", "自定义B"};
static  const char kIntrHarmGrpt[2][20] = {"Group", "Centred Subgroup"};

static  const char kGpsSingleType[4][11] = {"B码+脉冲", "B码", "脉冲", "关"};
static  const char Gps_Pulse_Type[2][10] = {"分脉冲", "秒脉冲"};

static  const char YesNo[2][4] = {"关", "开"};
static  const char NoYes[2][4] = {"开", "关"};
static  const char CheckBox[2][4] = {"□ ", "√ "};
static  const unsigned int month_day[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static  const char Bak2USBResult[][32] = {"备份完毕！", "没有找到USB设备！", "挂接USB设备失败！",
                                         "创建目录usb/boyuux失败！", "无效的数据类型！", "拷贝稳态数据失败！",
                                         "拷贝暂态数据失败！", "卸载USB设备失败！", "请选择要备份的数据类型！"
                                        };
static  const char CnctTpSlct[2][4] = {"Y0", "VV"}; //接线方式选项
static  const char RcdWvSavFmt[2][12] = {"Boyuu", "ComTraDE"}; //Record wave data save format

static  const char MdfyPassVW[][12] = {   //设置密码界面
    "    旧密码:",
    "    新密码:",
    "确认新密码:"
};

#endif // VIEWSET_MENU_H 


