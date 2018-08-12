/*! \file app_prtcl_guis.h
    \brief Applicate layer type GUI server protocol class.
    Copyright (c) 2017  Xi'an Boyuu Electric, Inc.
*/
#ifndef _APP_PRTCL_GUI_H_
#define _APP_PRTCL_GUI_H_

enum kGuiCommand { //GUI communication command
    //Client -> Server
    //kCmdNone,
    kCmdPrmPhd = 1,
    kCmdPrmLd,
    kCmdPrmChnl,
    kCmdGetEvtState,
    kCmdSendCtrlSig,
    kCmdGetEquipPrm,
    kCmdSetEquipPrm,
    kCmdGetHrmLimit,
    kCmdGetUsrInfo,
    kCmdSetUsrInfo,
    kCmdGetEquipState,
    kGuiCmdC2SEnd,
    //Server -> Client
    kCmdFrequency=0x101,
    kCmdWaveform,
    kCmdHarmonic,
    kCmdOthers,
    kCmdPower,
    kCmdWarnChlMatch,
};

class AppPrtclGui
{
public:
	AppPrtclGui(){};
	~AppPrtclGui(){};
	
protected:

    void IniHead(struct CommDataHead *head);

private:
};


#endif // _APP_PRTCL_GUI_H_ 
